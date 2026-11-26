#include "YapBaseEditorModule.h"

#include "AssetToolsModule.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IAssetTools.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "YapBase/YapBaseLog.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapCharacterComponent.h"
#include "YapBase/YapSpeechConfig.h"
#include "DetailCustomization_YapCharacterAsset.h"
#include "DetailCustomization_YapSpeechConfig.h"
#include "PropertyCustomization_YapCharacterIdentity.h"
#include "PropertyCustomization_YapPortraitList.h"
#include "YapCharacterThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

namespace
{
static void AddRuleIfMissing(UAssetManagerSettings& Settings, const FPrimaryAssetType& Type, UClass* BaseClass, TArray<FString>& OutAdded)
{
	const bool bHasRule = Settings.PrimaryAssetTypesToScan.ContainsByPredicate([&Type](const FPrimaryAssetTypeInfo& Info)
	{
		return Info.PrimaryAssetType == Type.GetName();
	});

	if (bHasRule)
	{
		return;
	}

	FDirectoryPath Game;
	Game.Path = TEXT("/Game");

	FPrimaryAssetTypeInfo Info(Type.GetName(), BaseClass, false, false, { Game }, {});
	Info.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;

	Settings.PrimaryAssetTypesToScan.Add(Info);
	OutAdded.Add(Type.ToString());
}
}

FAssetCategoryPath FYapBaseAssetCategoryPaths::Yap(LOCTEXT("YapCategory", "Yap"));

EAssetTypeCategories::Type FYapBaseEditorModule::AssetCategory = EAssetTypeCategories::Misc;

void FYapBaseEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Yap")), LOCTEXT("YapCategory", "Yap"));

	RegisterCustomizations();

	PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(this, &FYapBaseEditorModule::EnsureAssetManagerRules);
}

void FYapBaseEditorModule::ShutdownModule()
{
	FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);

	UnregisterCustomizations();
}

void FYapBaseEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	auto AddClass = [this, &PropertyModule](const UClass* Class, FOnGetDetailCustomizationInstance Make)
	{
		PropertyModule.RegisterCustomClassLayout(Class->GetFName(), Make);
		CustomizedClasses.Add(Class->GetFName());
	};

	auto AddStruct = [this, &PropertyModule](const UScriptStruct* Struct, FOnGetPropertyTypeCustomizationInstance Make)
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(Struct->GetFName(), Make);
		CustomizedStructs.Add(Struct->GetFName());
	};

	AddClass(UYapCharacterAsset::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapCharacterAsset::MakeInstance));
	AddClass(UYapSpeechConfig::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapSpeechConfig::MakeInstance));
	AddStruct(FYapPortraitList::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_YapPortraitList::MakeInstance));
	AddStruct(FYapCharacterIdentity::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_YapCharacterIdentity::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	UThumbnailManager::Get().RegisterCustomRenderer(UYapCharacterAsset::StaticClass(), UYapCharacterThumbnailRenderer::StaticClass());
}

void FYapBaseEditorModule::UnregisterCustomizations()
{
	if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		for (const FName& Name : CustomizedClasses)
		{
			PropertyModule->UnregisterCustomClassLayout(Name);
		}
		for (const FName& Name : CustomizedStructs)
		{
			PropertyModule->UnregisterCustomPropertyTypeLayout(Name);
		}
		PropertyModule->NotifyCustomizationModuleChanged();
	}
	CustomizedClasses.Empty();
	CustomizedStructs.Empty();

	if (UObjectInitialized() && !IsEngineExitRequested())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UYapCharacterAsset::StaticClass());
	}
}

void FYapBaseEditorModule::EnsureAssetManagerRules()
{
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();
	if (!Settings)
	{
		return;
	}

	TArray<FString> Added;
	AddRuleIfMissing(*Settings, UYapSpeechConfig::GetPrimaryAssetType(), UYapSpeechConfig::StaticClass(), Added);
	AddRuleIfMissing(*Settings, UYapCharacterAsset::GetPrimaryAssetType(), UYapCharacterAsset::StaticClass(), Added);

	if (Added.Num() == 0)
	{
		return;
	}

	Settings->TryUpdateDefaultConfigFile();

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		AssetManager->ReinitializeFromConfig();
	}

	const FString Types = FString::Join(Added, TEXT(", "));
	UE_LOG(LogYapBase, Display, TEXT("Added Asset Manager rules for Yap's primary asset types (%s), in DefaultGame.ini."), *Types);

	FNotificationInfo Info(FText::Format(LOCTEXT("AddedRules", "Yap added Asset Manager rules for {0} (Project Settings > Asset Manager), so those assets can be found by name and are packaged."), FText::FromString(Types)));
	Info.ExpireDuration = 8.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
}

IMPLEMENT_MODULE(FYapBaseEditorModule, YapBaseEditor)

#undef LOCTEXT_NAMESPACE

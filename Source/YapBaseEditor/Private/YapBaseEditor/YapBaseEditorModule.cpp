#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "YapBase/YapBaseLog.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapSpeechConfig.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

/**
 * The YapBase editor module. At startup it makes sure the Asset Manager knows Yap's primary asset types (speech
 * configs and characters), so scripts can name them and they're packaged without being listed anywhere: if a rule is
 * missing from Project Settings > Asset Manager, it's added (saved to DefaultGame.ini) and a notification says so.
 */
class FYapBaseEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(this, &FYapBaseEditorModule::EnsureAssetManagerRules);
	}

	virtual void ShutdownModule() override
	{
		FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
	}

private:
	void EnsureAssetManagerRules()
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

	FDelegateHandle PostEngineInitHandle;
};

IMPLEMENT_MODULE(FYapBaseEditorModule, YapBaseEditor)

#undef LOCTEXT_NAMESPACE

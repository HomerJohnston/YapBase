#include "AssetFactory_YapBroker.h"

#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "YapBase/YapBroker.h"
#include "YapBaseEditorModule.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

UAssetFactory_YapBroker::UAssetFactory_YapBroker()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
}

UObject* UAssetFactory_YapBroker::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(UYapBroker::StaticClass(), InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

uint32 UAssetFactory_YapBroker::GetMenuCategories() const
{
	return FYapBaseEditorModule::GetAssetCategory();
}

FText UAssetFactory_YapBroker::GetDisplayName() const
{
	return LOCTEXT("YapBrokerFactoryName", "Yap Broker");
}

FText UAssetFactory_YapBroker::GetToolTip() const
{
	return LOCTEXT("YapBrokerFactoryToolTip", "A Blueprint subclass of the Yap broker, where the game answers Yap's questions (maturity setting, playback speed, text time, audio). Set it as Project Settings > Yap > Broker Class.");
}

#undef LOCTEXT_NAMESPACE

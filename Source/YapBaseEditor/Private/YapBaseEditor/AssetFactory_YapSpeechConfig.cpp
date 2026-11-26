#include "AssetFactory_YapSpeechConfig.h"

#include "YapBase/YapSpeechConfig.h"
#include "YapBaseEditorModule.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

UAssetFactory_YapSpeechConfig::UAssetFactory_YapSpeechConfig()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYapSpeechConfig::StaticClass();
}

UObject* UAssetFactory_YapSpeechConfig::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UYapSpeechConfig>(InParent, Class, Name, Flags | RF_Transactional);
}

uint32 UAssetFactory_YapSpeechConfig::GetMenuCategories() const
{
	return FYapBaseEditorModule::GetAssetCategory();
}

FText UAssetFactory_YapSpeechConfig::GetDisplayName() const
{
	return LOCTEXT("YapSpeechConfigFactoryName", "Yap Speech Config");
}

#undef LOCTEXT_NAMESPACE

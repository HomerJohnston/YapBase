#include "AssetFactory_YapCharacter.h"

#include "YapBase/YapCharacterAsset.h"
#include "YapBaseEditorModule.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

UAssetFactory_YapCharacter::UAssetFactory_YapCharacter()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYapCharacterAsset::StaticClass();
}

UObject* UAssetFactory_YapCharacter::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UYapCharacterAsset>(InParent, Class, Name, Flags | RF_Transactional);
}

uint32 UAssetFactory_YapCharacter::GetMenuCategories() const
{
	return FYapBaseEditorModule::GetAssetCategory();
}

FText UAssetFactory_YapCharacter::GetDisplayName() const
{
	return LOCTEXT("YapCharacterFactoryName", "Yap Character");
}

#undef LOCTEXT_NAMESPACE

#include "AssetDefinition_YapCharacterAsset.h"

#include "YapBase/YapCharacterAsset.h"
#include "YapBaseEditorColor.h"
#include "YapBaseEditorModule.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

FText UAssetDefinition_YapCharacterAsset::GetAssetDisplayName() const
{
	return LOCTEXT("YapCharacterAssetName", "Yap Character");
}

FLinearColor UAssetDefinition_YapCharacterAsset::GetAssetColor() const
{
	return YapBaseColor::CharacterAsset;
}

TSoftClassPtr<UObject> UAssetDefinition_YapCharacterAsset::GetAssetClass() const
{
	return UYapCharacterAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapCharacterAsset::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { FYapBaseAssetCategoryPaths::Yap };
	return Categories;
}

FAssetSupportResponse UAssetDefinition_YapCharacterAsset::CanLocalize(const FAssetData& InAsset) const
{
	return FAssetSupportResponse::Supported();
}

#undef LOCTEXT_NAMESPACE

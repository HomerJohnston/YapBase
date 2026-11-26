#include "AssetDefinition_YapSpeechConfig.h"

#include "YapBase/YapSpeechConfig.h"
#include "YapBaseEditorColor.h"
#include "YapBaseEditorModule.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

FText UAssetDefinition_YapSpeechConfig::GetAssetDisplayName() const
{
	return LOCTEXT("YapSpeechConfigAssetName", "Yap Speech Config");
}

FLinearColor UAssetDefinition_YapSpeechConfig::GetAssetColor() const
{
	return YapBaseColor::SpeechConfig;
}

TSoftClassPtr<UObject> UAssetDefinition_YapSpeechConfig::GetAssetClass() const
{
	return UYapSpeechConfig::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YapSpeechConfig::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { FYapBaseAssetCategoryPaths::Yap };
	return Categories;
}

#undef LOCTEXT_NAMESPACE

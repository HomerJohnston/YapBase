#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_YapCharacterAsset.generated.h"

/** Yap characters in the content browser: name, colour, the "Yap" category. */
UCLASS()
class UAssetDefinition_YapCharacterAsset : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual FAssetSupportResponse CanLocalize(const FAssetData& InAsset) const override;
};

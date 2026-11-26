#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_YapSpeechConfig.generated.h"

/** Yap speech configs in the content browser: name, colour, the "Yap" category. */
UCLASS()
class UAssetDefinition_YapSpeechConfig : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};

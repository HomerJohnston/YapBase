#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetFactory_YapSpeechConfig.generated.h"

/**
 * Makes speech configs from the content browser's Add menu (Yap > Yap Speech Config). Scripts name a config by its
 * Config Name (the asset's name if that's empty); the one named by Project Settings > Yap > Default Speech Config
 * ("Default") is used when a script names none.
 */
UCLASS()
class UAssetFactory_YapSpeechConfig : public UFactory
{
	GENERATED_BODY()

public:
	UAssetFactory_YapSpeechConfig();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};

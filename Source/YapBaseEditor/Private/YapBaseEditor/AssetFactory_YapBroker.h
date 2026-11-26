#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetFactory_YapBroker.generated.h"

/**
 * Makes a Blueprint subclass of UYapBroker from the content browser's Add menu (Yap > Yap Broker). Set it as Project
 * Settings > Yap > Broker Class to have it used.
 *
 * (Yap's broker factory made a broker object; YapBase's settings take a broker class, so this makes a Blueprint.)
 */
UCLASS()
class UAssetFactory_YapBroker : public UFactory
{
	GENERATED_BODY()

public:
	UAssetFactory_YapBroker();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;
};

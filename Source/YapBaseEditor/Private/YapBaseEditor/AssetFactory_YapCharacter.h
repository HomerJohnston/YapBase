#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetFactory_YapCharacter.generated.h"

/** Makes Yap characters from the content browser's Add menu (Yap > Yap Character). */
UCLASS()
class UAssetFactory_YapCharacter : public UFactory
{
	GENERATED_BODY()

public:
	UAssetFactory_YapCharacter();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};

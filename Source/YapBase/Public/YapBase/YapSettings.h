#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "YapSettings.generated.h"

class UYapBroker;

/** Project settings: Plugins > Yap. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Yap"))
class YAPBASE_API UYapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Your game's broker (UYapBroker subclass). Empty: Yap's default one. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapBroker> BrokerClass;

	/** The speech config used where none is named. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	FName DefaultSpeechConfig = TEXT("Default");

	/** A child-safe text's key is its line's ID with this added. */
	UPROPERTY(Config, EditAnywhere, Category = "Lines")
	FString ChildSafeKeySuffix = TEXT("_ChildSafe");

	/**
	 * The form of new line IDs, for editor tools that make them: * a random capital letter, ? a random letter or digit,
	 * # a digit of the next number. The default makes IDs like UXA001.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Lines")
	FString LineIDFormat = TEXT("***###");

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};

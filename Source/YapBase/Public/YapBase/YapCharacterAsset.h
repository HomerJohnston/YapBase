#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "YapCharacterAsset.generated.h"

class UTexture2D;

/**
 * A character that can speak. Scripts name it by its Character ID (the asset's name if that's empty); Display Name is
 * what players see. It's a primary asset, so it's found and packaged without being listed anywhere.
 *
 * Subclass it (in C++ or Blueprint) to add your game's own character data.
 */
UCLASS(BlueprintType, Blueprintable)
class YAPBASE_API UYapCharacterAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	static FPrimaryAssetType GetPrimaryAssetType() { return FPrimaryAssetType(FName(TEXT("YapCharacter"))); }

	/** What scripts call this character. Empty: the asset's name. Must be unique among characters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName CharacterID;

	/** The name players see. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FLinearColor Color = FLinearColor::White;

	/** The portrait for a mood that has none of its own, and for no mood. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portraits")
	TObjectPtr<UTexture2D> Portrait = nullptr;

	/** A portrait for each mood that has its own. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portraits")
	TMap<FGameplayTag, TObjectPtr<UTexture2D>> MoodPortraits;

public:
	/** CharacterID, or the asset's name if that's empty. */
	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	FName GetCharacterID() const;

	/** The mood's portrait, or the default portrait. */
	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	UTexture2D* GetPortrait(FGameplayTag Mood) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

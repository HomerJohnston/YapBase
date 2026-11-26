#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "YapCharacterAsset.generated.h"

class UTexture2D;

/** A character's portraits for the moods under one mood root (keyed by the mood tag's name). */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapPortraitList
{
	GENERATED_BODY()

	/** Texture for each mood. Moods are stored as FNames instead of gameplay tags, so the list can be filled in for you without stopping tags from being deleted. */
	UPROPERTY(EditAnywhere, Category = "Default", EditFixedSize, meta = (ReadOnlyKeys, ForceInlineRow))
	TMap<FName, TObjectPtr<UTexture2D>> Map;
};

/**
 * A character that can speak. Scripts name it by its Character ID (the asset's name if that's empty); Name is what
 * players see. It's a primary asset, so it's found and packaged without being listed anywhere.
 *
 * Its portraits are kept per mood: one list for each mood root that a speech config uses, filled in by the asset's
 * details panel (Portraits: "Click to refresh").
 *
 * Subclass it (in C++ or Blueprint) to add your game's own character data.
 */
UCLASS(BlueprintType, Blueprintable)
class YAPBASE_API UYapCharacterAsset : public UObject
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FDetailCustomization_YapCharacterAsset;
#endif

public:
	UYapCharacterAsset();

	static FPrimaryAssetType GetPrimaryAssetType() { return FPrimaryAssetType(FName(TEXT("YapCharacter"))); }

protected:
	/** What scripts call this character. Empty: the asset's name. Must be unique among characters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName CharacterID;

	/** Human-readable name of this character or entity: what players see. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Name", Category = "Character")
	FText EntityName;

	/** Color for display. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Color", Category = "Character")
	FLinearColor EntityColor;

	/** Default portrait texture. Used when a line has no mood (or a mood with no portrait), and for the asset's thumbnail. */
	UPROPERTY(EditAnywhere, Category = "Portraits", DisplayName = "Default Portrait")
	TObjectPtr<UTexture2D> Portrait;

	/**
	 * Portrait textures, raw map access: mood root's name -> (mood tag's name -> texture). The details panel above shows
	 * the same thing; you don't normally edit this. If you do, close and reopen the asset.
	 */
	UPROPERTY(EditAnywhere, Category = "Portraits", AdvancedDisplay, DisplayName = "Portraits Map (Raw Access)", meta = (ForceInlineRow))
	TMap<FName, FYapPortraitList> PortraitsMap;

public:
	/** CharacterID, or the asset's name if that's empty. */
	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	FName GetCharacterID() const;

	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	FText GetCharacterName() const { return EntityName; }

	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	FLinearColor GetCharacterColor() const { return EntityColor; }

	/** The mood's portrait, or the default portrait (no mood, or none set for it). */
	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	UTexture2D* GetCharacterPortrait(FGameplayTag MoodTag) const;

	void SetCharacterID(FName InCharacterID) { CharacterID = InCharacterID; }
	void SetName(const FText& InName) { EntityName = InName; }
	void SetColor(const FLinearColor& InColor) { EntityColor = InColor; }
	void SetPortrait(UTexture2D* InTexture) { Portrait = InTexture; }

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	bool HasAnyMoodTags() const;
	/** True if the portrait lists don't match the mood roots (and their mood tags) of the project's speech configs. */
	bool GetPortraitsOutOfDate() const;
#endif
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "YapBase/YapTypes.h"
#include "YapSpeechConfig.generated.h"

/**
 * A kind of speech: "in person", "radio chatter", "tutorial hints". Its settings are the defaults for every line
 * spoken with it, and the game can tell the kinds apart (each event says which config it's from; a handler can be
 * registered for one config only).
 *
 * Scripts name a config by its Config Name (the asset's name if that's empty). It's a primary asset, so it's found
 * and packaged without being listed anywhere.
 */
UCLASS(BlueprintType)
class YAPBASE_API UYapSpeechConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	static FPrimaryAssetType GetPrimaryAssetType() { return FPrimaryAssetType(FName(TEXT("YapSpeechConfig"))); }

	/** What scripts call this config. Empty: the asset's name. Must be unique among speech configs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName ConfigName;

	// ---- Conversations (free speech is always interruptible and always moves on by itself)

	/** In a conversation, can the player's advance cut a line short while it's playing? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
	bool bInterruptible = true;

	/** In a conversation, does a line move on by itself when it's done, or wait for the player to advance? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
	bool bAutoAdvance = true;

	// ---- Playback

	/** If set, a character can say several things at once. Otherwise a new line from a speaker cancels the one they're saying. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	bool bPermitOverlappingSpeech = false;

	/** If set, child-safe text is never used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	bool bDisableChildSafe = false;

	/** If set, lines never have audio. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	bool bDisableAudio = false;

	// ---- Time

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	EYapTimeMode DefaultTimeMode = EYapTimeMode::Audio;

	/** Seconds after a line before what follows it. Negative: what follows starts before it's finished. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (UIMin = -2.0, UIMax = 5.0))
	float DefaultPadding = 0.25f;

	/** The shortest time a line timed from its text plays for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = 0.0))
	float MinimumTextTime = 1.0f;

	/** The shortest time a line timed from its audio plays for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = 0.0))
	float MinimumAudioTime = 0.5f;

	/** The shortest time any timed line plays for. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = 0.0))
	float MinimumSpeakingTime = 0.0f;

	/** For text time: how fast lines are read. The broker's playback speed scales it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = 1.0, UIMin = 60.0, UIMax = 240.0))
	float TextWordsPerMinute = 120.0f;

	// ---- Prompts

	/** If set, a prompt with only one entry chooses it without asking the player. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompts")
	bool bAutoSelectLastPrompt = false;

	/** If set, a chosen entry's line moves on by itself, even in a conversation that waits for the player. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompts")
	bool bPromptAdvancesImmediately = false;

	// ---- Moods

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moods")
	bool bDisableMoodTags = false;

	/** Moods are named by the tags under this one: "Happy" is <root>.Happy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moods", meta = (EditCondition = "!bDisableMoodTags"))
	FGameplayTag MoodTagsRoot;

	/** The mood of a line that doesn't give one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moods", meta = (EditCondition = "!bDisableMoodTags"))
	FGameplayTag DefaultMoodTag;

public:
	/** ConfigName, or the asset's name if that's empty. */
	UFUNCTION(BlueprintPure, Category = "Yap")
	FName GetConfigName() const;

	/** The default time mode, never Default. */
	EYapTimeMode GetDefaultTimeMode() const;

	/** The tag for a mood name (None: the default mood). False, with OutError, if there's no such tag under the root. */
	bool ResolveMood(FName Mood, FGameplayTag& OutTag, FString& OutError) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

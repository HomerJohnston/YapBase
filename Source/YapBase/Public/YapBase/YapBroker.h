#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YapBase/YapTypes.h"
#include "YapBroker.generated.h"

class UYapSpeechConfig;

/**
 * Connects Yap to your game. Make a subclass (C++ or Blueprint), override what you need, and set it in the project
 * settings (Plugins > Yap > Broker Class). One is made for each game world.
 *
 * In C++, override the _Implementation functions. Don't call the parent's version of what you override.
 */
UCLASS(Blueprintable)
class YAPBASE_API UYapBroker : public UObject
{
	GENERATED_BODY()

public:
	/** The player's maturity setting: whether to use child-safe text and audio. Default: mature. */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	EYapMaturitySetting GetMaturitySetting() const;

	/** Scales text time (a player's reading speed setting). 1: normal. Doesn't affect audio time. */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	float GetPlaybackSpeed() const;

	/** How many words a text has, for text time. */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	int32 CalculateWordCount(const FText& Text) const;

	/** How long a text takes to read. Default: the config's words per minute, scaled by GetPlaybackSpeed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	float CalculateTextTime(int32 WordCount, int32 CharacterCount, const UYapSpeechConfig* Config) const;

	/** How long an audio asset plays. Default: a USoundBase's duration. Override for Wwise, FMOD and the like. */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	float GetAudioDuration(const UObject* Audio) const;

	/**
	 * The audio for a line, found by its audio ID: the line's ID, with the child-safe suffix for child-safe audio.
	 * Default: none.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Yap")
	UObject* FindLineAudio(FName AudioID, EYapMaturitySetting Maturity, const UYapSpeechConfig* Config) const;

	virtual UWorld* GetWorld() const override;

protected:
	virtual EYapMaturitySetting GetMaturitySetting_Implementation() const;
	virtual float GetPlaybackSpeed_Implementation() const;
	virtual int32 CalculateWordCount_Implementation(const FText& Text) const;
	virtual float CalculateTextTime_Implementation(int32 WordCount, int32 CharacterCount, const UYapSpeechConfig* Config) const;
	virtual float GetAudioDuration_Implementation(const UObject* Audio) const;
	virtual UObject* FindLineAudio_Implementation(FName AudioID, EYapMaturitySetting Maturity, const UYapSpeechConfig* Config) const;
};

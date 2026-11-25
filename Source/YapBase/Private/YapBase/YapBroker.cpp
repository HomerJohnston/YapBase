#include "YapBase/YapBroker.h"

#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "YapBase/YapSpeechConfig.h"

UWorld* UYapBroker::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	const UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

EYapMaturitySetting UYapBroker::GetMaturitySetting_Implementation() const
{
	return EYapMaturitySetting::Mature;
}

float UYapBroker::GetPlaybackSpeed_Implementation() const
{
	return 1.0f;
}

int32 UYapBroker::CalculateWordCount_Implementation(const FText& Text) const
{
	const FString String = Text.ToString();
	int32 Words = 0;
	bool bInWord = false;

	for (const TCHAR Character : String)
	{
		const bool bSpace = FChar::IsWhitespace(Character);
		if (!bSpace && !bInWord)
		{
			++Words;
		}
		bInWord = !bSpace;
	}

	return Words;
}

float UYapBroker::CalculateTextTime_Implementation(int32 WordCount, int32 CharacterCount, const UYapSpeechConfig* Config) const
{
	const float WordsPerMinute = Config ? FMath::Max(Config->TextWordsPerMinute, 1.0f) : 120.0f;
	const float Speed = FMath::Max(GetPlaybackSpeed(), KINDA_SMALL_NUMBER);
	return (60.0f * WordCount / WordsPerMinute) / Speed;
}

float UYapBroker::GetAudioDuration_Implementation(const UObject* Audio) const
{
	if (const USoundBase* Sound = Cast<USoundBase>(Audio))
	{
		return Sound->GetDuration();
	}

	return 0.0f;
}

UObject* UYapBroker::FindLineAudio_Implementation(FName AudioID, EYapMaturitySetting Maturity, const UYapSpeechConfig* Config) const
{
	return nullptr;
}

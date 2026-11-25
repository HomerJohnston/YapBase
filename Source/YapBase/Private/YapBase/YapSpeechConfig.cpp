#include "YapBase/YapSpeechConfig.h"

FName UYapSpeechConfig::GetConfigName() const
{
	return ConfigName.IsNone() ? GetFName() : ConfigName;
}

EYapTimeMode UYapSpeechConfig::GetDefaultTimeMode() const
{
	return DefaultTimeMode == EYapTimeMode::Default ? EYapTimeMode::Audio : DefaultTimeMode;
}

bool UYapSpeechConfig::ResolveMood(FName Mood, FGameplayTag& OutTag, FString& OutError) const
{
	OutTag = FGameplayTag();

	if (bDisableMoodTags)
	{
		return true;
	}

	if (Mood.IsNone())
	{
		OutTag = DefaultMoodTag;
		return true;
	}

	if (!MoodTagsRoot.IsValid())
	{
		OutError = FString::Printf(TEXT("The speech config '%s' has no mood tags root, so the mood '%s' can't be found."), *GetConfigName().ToString(), *Mood.ToString());
		return false;
	}

	const FString TagName = MoodTagsRoot.ToString() + TEXT(".") + Mood.ToString();
	OutTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);

	if (!OutTag.IsValid())
	{
		OutError = FString::Printf(TEXT("'%s' isn't a mood of the speech config '%s': there's no gameplay tag '%s'."), *Mood.ToString(), *GetConfigName().ToString(), *TagName);
		return false;
	}

	return true;
}

FPrimaryAssetId UYapSpeechConfig::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(GetPrimaryAssetType(), GetConfigName());
}

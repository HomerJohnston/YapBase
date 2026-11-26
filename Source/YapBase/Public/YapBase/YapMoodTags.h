#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UYapSpeechConfig;

namespace Yap
{
	/** Every speech config asset in the project (loading them). */
	YAPBASE_API TArray<UYapSpeechConfig*> GetSpeechConfigs();

	/** The mood roots the project's speech configs use (those with mood tags turned on), sorted. */
	YAPBASE_API FGameplayTagContainer GetMoodTagRoots();

	/** The speech config that uses this mood root, or null. */
	YAPBASE_API UYapSpeechConfig* GetConfigUsingMoodRoot(const FGameplayTag& Root);

	/** Every mood tag (the children of every mood root). */
	YAPBASE_API FGameplayTagContainer GetAllMoodTags();

	YAPBASE_API FGameplayTagContainer GetAllMoodTagsUnder(const FGameplayTagContainer& Roots);
	YAPBASE_API FGameplayTagContainer GetAllMoodTagsUnder(const FGameplayTag& Root);
}

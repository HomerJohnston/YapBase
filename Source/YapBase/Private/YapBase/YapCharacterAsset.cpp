#include "YapBase/YapCharacterAsset.h"

#include "Engine/Texture2D.h"

FName UYapCharacterAsset::GetCharacterID() const
{
	return CharacterID.IsNone() ? GetFName() : CharacterID;
}

UTexture2D* UYapCharacterAsset::GetPortrait(FGameplayTag Mood) const
{
	if (Mood.IsValid())
	{
		if (const TObjectPtr<UTexture2D>* MoodPortrait = MoodPortraits.Find(Mood))
		{
			if (*MoodPortrait)
			{
				return *MoodPortrait;
			}
		}
	}

	return Portrait;
}

FPrimaryAssetId UYapCharacterAsset::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(GetPrimaryAssetType(), GetCharacterID());
}

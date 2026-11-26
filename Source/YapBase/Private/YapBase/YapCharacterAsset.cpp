#include "YapBase/YapCharacterAsset.h"

#include "Engine/Texture2D.h"
#include "GameplayTagsManager.h"
#include "YapBase/YapMoodTags.h"

UYapCharacterAsset::UYapCharacterAsset()
	: EntityColor(0.5f, 0.5f, 0.5f, 1.0f)
{
}

FName UYapCharacterAsset::GetCharacterID() const
{
	return CharacterID.IsNone() ? GetFName() : CharacterID;
}

UTexture2D* UYapCharacterAsset::GetCharacterPortrait(FGameplayTag MoodTag) const
{
	if (MoodTag.IsValid())
	{
		if (const FYapPortraitList* PortraitList = PortraitsMap.Find(MoodTag.RequestDirectParent().GetTagName()))
		{
			if (const TObjectPtr<UTexture2D>* Texture = PortraitList->Map.Find(MoodTag.GetTagName()))
			{
				if (*Texture)
				{
					return *Texture;
				}
			}
		}
	}

	return Portrait;
}

FPrimaryAssetId UYapCharacterAsset::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || HasAnyFlags(RF_Transient))
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(GetPrimaryAssetType(), GetCharacterID());
}

#if WITH_EDITOR
bool UYapCharacterAsset::HasAnyMoodTags() const
{
	for (const TPair<FName, FYapPortraitList>& Pair : PortraitsMap)
	{
		if (Pair.Key.IsValid() && !Pair.Key.IsNone())
		{
			return true;
		}
	}
	return false;
}

bool UYapCharacterAsset::GetPortraitsOutOfDate() const
{
	const FGameplayTagContainer MoodTagRoots = Yap::GetMoodTagRoots();

	if (PortraitsMap.Num() != MoodTagRoots.Num())
	{
		return true;
	}

	for (const FGameplayTag& Root : MoodTagRoots)
	{
		const FYapPortraitList* List = PortraitsMap.Find(Root.GetTagName());
		if (!List)
		{
			return true;
		}

		const FGameplayTagContainer MoodTags = Yap::GetAllMoodTagsUnder(Root);
		if (List->Map.Num() != MoodTags.Num())
		{
			return true;
		}

		for (const FGameplayTag& Mood : MoodTags)
		{
			if (!List->Map.Contains(Mood.GetTagName()))
			{
				return true;
			}
		}
	}

	return false;
}
#endif

#include "YapBase/YapMoodTags.h"

#include "AssetRegistry/IAssetRegistry.h"
#include "GameplayTagsManager.h"
#include "YapBase/YapSpeechConfig.h"

TArray<UYapSpeechConfig*> Yap::GetSpeechConfigs()
{
	TArray<UYapSpeechConfig*> Configs;

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return Configs;
	}

	TArray<FAssetData> Assets;
	AssetRegistry->GetAssetsByClass(UYapSpeechConfig::StaticClass()->GetClassPathName(), Assets, true);
	for (const FAssetData& Asset : Assets)
	{
		if (UYapSpeechConfig* Config = Cast<UYapSpeechConfig>(Asset.GetAsset()))
		{
			Configs.Add(Config);
		}
	}

	return Configs;
}

FGameplayTagContainer Yap::GetMoodTagRoots()
{
	TArray<FGameplayTag> Roots;
	for (const UYapSpeechConfig* Config : GetSpeechConfigs())
	{
		if (!Config->bDisableMoodTags && Config->MoodTagsRoot.IsValid())
		{
			Roots.AddUnique(Config->MoodTagsRoot);
		}
	}
	Roots.Sort();

	FGameplayTagContainer Container;
	for (const FGameplayTag& Root : Roots)
	{
		Container.AddTag(Root);
	}
	return Container;
}

UYapSpeechConfig* Yap::GetConfigUsingMoodRoot(const FGameplayTag& Root)
{
	for (UYapSpeechConfig* Config : GetSpeechConfigs())
	{
		if (Config->MoodTagsRoot == Root)
		{
			return Config;
		}
	}
	return nullptr;
}

FGameplayTagContainer Yap::GetAllMoodTags()
{
	return GetAllMoodTagsUnder(GetMoodTagRoots());
}

FGameplayTagContainer Yap::GetAllMoodTagsUnder(const FGameplayTagContainer& Roots)
{
	FGameplayTagContainer AllMoodTags;
	for (const FGameplayTag& Root : Roots)
	{
		AllMoodTags.AppendTags(UGameplayTagsManager::Get().RequestGameplayTagChildren(Root));
	}
	return AllMoodTags;
}

FGameplayTagContainer Yap::GetAllMoodTagsUnder(const FGameplayTag& Root)
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Root);
}

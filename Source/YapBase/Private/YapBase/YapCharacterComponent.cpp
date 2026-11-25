#include "YapBase/YapCharacterComponent.h"

#include "YapBase/YapBaseLog.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapCharacterManager.h"

UYapCharacterComponent::UYapCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UYapCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Character)
	{
		UE_LOG(LogYapBase, Warning, TEXT("%s's Yap character component has no character."), *GetNameSafe(GetOwner()));
		return;
	}

	if (UYapCharacterManager* Manager = UYapCharacterManager::Get(this))
	{
		Manager->RegisterCharacter(Character);
		Manager->RegisterActor(Character->GetCharacterID(), GetOwner());
	}
}

void UYapCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character)
	{
		if (UYapCharacterManager* Manager = UYapCharacterManager::Get(this))
		{
			Manager->UnregisterActor(Character->GetCharacterID(), GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

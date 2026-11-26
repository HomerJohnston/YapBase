#include "YapBase/YapCharacterComponent.h"

#include "GameFramework/Actor.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapCharacterManager.h"

FName FYapCharacterIdentity::Get(const UObject* Owner)
{
	if (Identity_Character)
	{
		return Identity_Character->GetCharacterID();
	}

	if (!Identity_Name.IsNone())
	{
		return Identity_Name;
	}

	if (Identity_Auto.IsNone())
	{
		const FString OwnerName = Owner ? Owner->GetName() : TEXT("Character");
		Identity_Auto = FName(OwnerName + TEXT("_") + FGuid::NewGuid().ToString(EGuidFormats::Digits));
	}
	return Identity_Auto;
}

UYapCharacterComponent::UYapCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

FName UYapCharacterComponent::GetCharacterID()
{
	return Identity.Get(GetOwner());
}

void UYapCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bPreventAutoRegisterComponent && !bComponentRegistered)
	{
		if (UYapCharacterManager* Manager = UYapCharacterManager::Get(this))
		{
			Manager->RegisterActor(GetCharacterID(), GetOwner());
			bComponentRegistered = true;
		}
	}

	if (!bPreventAutoRegisterCharacterDefinition)
	{
		RegisterCharacterDefinition();
	}
}

void UYapCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bComponentRegistered)
	{
		if (UYapCharacterManager* Manager = UYapCharacterManager::Get(this))
		{
			Manager->UnregisterActor(GetCharacterID(), GetOwner());
		}
		bComponentRegistered = false;
	}

	UnregisterCharacterDefinition();

	Super::EndPlay(EndPlayReason);
}

void UYapCharacterComponent::RegisterCharacterDefinition()
{
	if (bDefinitionRegistered)
	{
		return;
	}

	UYapCharacterManager* Manager = UYapCharacterManager::Get(this);
	if (!Manager)
	{
		return;
	}

	if (Identity.UsesProjectCharacter())
	{
		Manager->RegisterCharacter(Identity.GetCharacter());
	}
	else
	{
		Manager->RegisterCharacterDefinition(GetCharacterID(), CharacterDefinition);
	}
	bDefinitionRegistered = true;
}

void UYapCharacterComponent::UnregisterCharacterDefinition()
{
	if (!bDefinitionRegistered)
	{
		return;
	}

	if (UYapCharacterManager* Manager = UYapCharacterManager::Get(this))
	{
		Manager->UnregisterCharacter(GetCharacterID());
	}
	bDefinitionRegistered = false;
}

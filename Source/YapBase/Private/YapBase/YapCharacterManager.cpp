#include "YapBase/YapCharacterManager.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "YapBase/YapAssets.h"
#include "YapBase/YapBaseLog.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapCharacterRuntimeDefinition.h"

UYapCharacterManager* UYapCharacterManager::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UYapCharacterManager>() : nullptr;
}

UYapCharacterAsset* UYapCharacterManager::FindCharacter(FName CharacterID)
{
	FString Error;
	return LookUpCharacter(CharacterID, Error);
}

UYapCharacterAsset* UYapCharacterManager::LookUpCharacter(FName CharacterID, FString& OutError)
{
	if (CharacterID.IsNone())
	{
		OutError = TEXT("No character was given.");
		return nullptr;
	}

	if (const TObjectPtr<UYapCharacterAsset>* Character = Registered.Find(CharacterID))
	{
		return *Character;
	}

	if (const TObjectPtr<UYapCharacterAsset>* Character = Found.Find(CharacterID))
	{
		return *Character;
	}

	UYapCharacterAsset* Character = Cast<UYapCharacterAsset>(Yap::FindNamedAsset(UYapCharacterAsset::GetPrimaryAssetType(), UYapCharacterAsset::StaticClass(), CharacterID));
	if (!Character)
	{
		OutError = FString::Printf(TEXT("There's no character '%s': no Yap character asset has that ID, and none was registered with it."), *CharacterID.ToString());
		return nullptr;
	}

	Found.Add(CharacterID, Character);
	return Character;
}

void UYapCharacterManager::RegisterCharacter(UYapCharacterAsset* Character, FName IDOverride)
{
	if (!Character)
	{
		UE_LOG(LogYapBase, Warning, TEXT("RegisterCharacter: no character was given."));
		return;
	}

	const FName ID = IDOverride.IsNone() ? Character->GetCharacterID() : IDOverride;
	Registered.Add(ID, Character);
}

UYapCharacterAsset* UYapCharacterManager::RegisterCharacterDefinition(FName CharacterID, const FYapCharacterRuntimeDefinition& Definition)
{
	if (CharacterID.IsNone())
	{
		UE_LOG(LogYapBase, Warning, TEXT("RegisterCharacterDefinition: no character ID was given."));
		return nullptr;
	}

	UYapCharacterAsset* Character = NewObject<UYapCharacterAsset>(this, NAME_None, RF_Transient);
	Character->SetCharacterID(CharacterID);
	Definition.InitializeCharacter(Character);
	Registered.Add(CharacterID, Character);
	return Character;
}

void UYapCharacterManager::UnregisterCharacter(FName CharacterID)
{
	Registered.Remove(CharacterID);
}

void UYapCharacterManager::RegisterActor(FName CharacterID, AActor* Actor)
{
	if (CharacterID.IsNone() || !Actor)
	{
		UE_LOG(LogYapBase, Warning, TEXT("RegisterActor: a character ID and an actor are both needed."));
		return;
	}

	if (const TWeakObjectPtr<AActor>* Existing = Actors.Find(CharacterID))
	{
		if (Existing->IsValid() && Existing->Get() != Actor)
		{
			UE_LOG(LogYapBase, Warning, TEXT("The character '%s' is now %s in the world, instead of %s."), *CharacterID.ToString(), *Actor->GetName(), *Existing->Get()->GetName());
		}
	}

	Actors.Add(CharacterID, Actor);
}

void UYapCharacterManager::UnregisterActor(FName CharacterID, AActor* Actor)
{
	if (const TWeakObjectPtr<AActor>* Existing = Actors.Find(CharacterID))
	{
		if (!Existing->IsValid() || Existing->Get() == Actor)
		{
			Actors.Remove(CharacterID);
		}
	}
}

AActor* UYapCharacterManager::FindActor(FName CharacterID) const
{
	const TWeakObjectPtr<AActor>* Actor = Actors.Find(CharacterID);
	return Actor ? Actor->Get() : nullptr;
}

bool UYapCharacterManager::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

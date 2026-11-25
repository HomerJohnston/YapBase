#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "YapCharacterManager.generated.h"

class AActor;
class UYapCharacterAsset;

/**
 * Where characters are looked up for speech: by ID, to their definition (a character asset) and to the actor that
 * stands for them in the world, if there is one.
 *
 * Definitions are found among the project's character assets the first time they're asked for, or registered while
 * playing (RegisterCharacter, which also lets an ID stand for another asset). Actors are registered by their
 * UYapCharacterComponent.
 */
UCLASS()
class YAPBASE_API UYapCharacterManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UYapCharacterManager* Get(const UObject* WorldContext);

	/** The character with this ID: one registered, or else the character asset with this ID. Null if there's none. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	UYapCharacterAsset* FindCharacter(FName CharacterID);

	/** FindCharacter, with an error saying why when there's none. */
	UYapCharacterAsset* LookUpCharacter(FName CharacterID, FString& OutError);

	/** Makes the ID stand for this character (its own ID if IDOverride is None), replacing what it stood for. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void RegisterCharacter(UYapCharacterAsset* Character, FName IDOverride = NAME_None);

	/** Forgets a registered character. The ID then finds the character asset with that ID again, if there is one. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void UnregisterCharacter(FName CharacterID);

	/** Makes Actor the one that stands for the character in the world. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void RegisterActor(FName CharacterID, AActor* Actor);

	/** Forgets the character's actor, if it's this one. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void UnregisterActor(FName CharacterID, AActor* Actor);

	/** The actor that stands for the character, or null. */
	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	AActor* FindActor(FName CharacterID) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** Registered characters. */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UYapCharacterAsset>> Registered;

	/** Character assets found by ID. */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UYapCharacterAsset>> Found;

	TMap<FName, TWeakObjectPtr<AActor>> Actors;
};

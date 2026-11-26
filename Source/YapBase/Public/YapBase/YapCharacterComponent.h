#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YapBase/YapCharacterRuntimeDefinition.h"
#include "YapCharacterComponent.generated.h"

class UYapCharacterAsset;

/** Which character an actor is: a project character (its asset), or a custom ID. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapCharacterIdentity
{
	GENERATED_BODY()

	friend class UYapCharacterComponent;
	friend class FPropertyCustomization_YapCharacterIdentity;

private:
	/** A project character: its asset, whose Character ID this actor goes by. */
	UPROPERTY(EditAnywhere, Category = "Default", DisplayName = "Character")
	TObjectPtr<UYapCharacterAsset> Identity_Character = nullptr;

	/** Without a Character: a custom ID (for an NPC scripts name), used with the component's Character Definition. */
	UPROPERTY(EditAnywhere, Category = "Default", DisplayName = "Custom ID")
	FName Identity_Name;

	/** Without either: an ID made up while playing (scripts can't name it). */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Default", DisplayName = "Automatic ID")
	FName Identity_Auto;

public:
	/** The character's ID: the Character's, else the custom ID, else one made up for the owner (kept from then on). */
	FName Get(const UObject* Owner);

	bool UsesProjectCharacter() const { return Identity_Character != nullptr; }
	UYapCharacterAsset* GetCharacter() const { return Identity_Character; }
};

/**
 * Lets Yap find the actor that stands for a character in the world: the game is given it with the character's speech
 * (FYapSpeechData::SpeakerActor).
 *
 * For a project character, set Identity's Character: the character is registered too, so it's found by its ID even if
 * the Asset Manager doesn't know its asset. For another actor (an NPC), give a custom ID and a Character Definition: a
 * character is made from it while playing, under that ID.
 */
UCLASS(ClassGroup = "Yap", meta = (BlueprintSpawnableComponent), HideCategories = ("Sockets", "ComponentTick", "Cooking", "Events", "Asset UserData", "Navigation"))
class YAPBASE_API UYapCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYapCharacterComponent();

protected:
	/** Which character this actor is. */
	UPROPERTY(EditAnywhere, Category = "Yap Character")
	FYapCharacterIdentity Identity;

	/** Without a project character (ignored with one): what's used for this actor's name, portrait and color when it speaks (NPCs). */
	UPROPERTY(EditAnywhere, Category = "Yap Character")
	FYapCharacterRuntimeDefinition CharacterDefinition;

	/** If set, the actor isn't registered as the character's actor at BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Yap Character", AdvancedDisplay)
	bool bPreventAutoRegisterComponent = false;

	/** If set, the character (its asset, or the Character Definition) isn't registered at BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Yap Character", AdvancedDisplay)
	bool bPreventAutoRegisterCharacterDefinition = false;

	UPROPERTY(Transient)
	bool bComponentRegistered = false;

	UPROPERTY(Transient)
	bool bDefinitionRegistered = false;

public:
	/** Registers the character: its asset, or a character made from the Character Definition. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void RegisterCharacterDefinition();

	UFUNCTION(BlueprintCallable, Category = "Yap|Character")
	void UnregisterCharacterDefinition();

	UFUNCTION(BlueprintPure, Category = "Yap|Character")
	FName GetCharacterID();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

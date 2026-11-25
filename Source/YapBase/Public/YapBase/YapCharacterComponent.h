#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YapCharacterComponent.generated.h"

class UYapCharacterAsset;

/**
 * Makes its actor the one that stands for a character in the world, so the game is given it with the character's
 * speech (FYapSpeechData::SpeakerActor). It also registers the character, so scripts can name it even if its asset
 * isn't found by the Asset Manager.
 */
UCLASS(ClassGroup = "Yap", meta = (BlueprintSpawnableComponent))
class YAPBASE_API UYapCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYapCharacterComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Yap")
	TObjectPtr<UYapCharacterAsset> Character = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

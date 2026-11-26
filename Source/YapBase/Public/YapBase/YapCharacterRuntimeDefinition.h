#pragma once

#include "CoreMinimal.h"
#include "YapCharacterRuntimeDefinition.generated.h"

class UTexture2D;
class UYapCharacterAsset;

/** A character made while playing, for an actor that isn't a project character (an NPC): see UYapCharacterComponent. */
USTRUCT(BlueprintType)
struct YAPBASE_API FYapCharacterRuntimeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TObjectPtr<UTexture2D> DefaultPortrait = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FLinearColor Color = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	void InitializeCharacter(UYapCharacterAsset* Character) const;
};

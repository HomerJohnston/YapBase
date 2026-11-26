#include "YapBase/YapCharacterRuntimeDefinition.h"

#include "Engine/Texture2D.h"
#include "YapBase/YapCharacterAsset.h"

void FYapCharacterRuntimeDefinition::InitializeCharacter(UYapCharacterAsset* Character) const
{
	Character->SetName(Name);
	Character->SetPortrait(DefaultPortrait);
	Character->SetColor(Color);
}

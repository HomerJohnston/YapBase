#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"

namespace Yap
{
	/**
	 * Finds the asset of Class whose primary asset ID is (Type, Name), and loads it (synchronously, if it isn't loaded).
	 * Asks the Asset Manager first (which needs a rule for Type, so the assets are cooked: YapBaseEditor adds it), then
	 * the Asset Registry. Null if there's none.
	 */
	YAPBASE_API UObject* FindNamedAsset(const FPrimaryAssetType& Type, const UClass* Class, FName Name);
}

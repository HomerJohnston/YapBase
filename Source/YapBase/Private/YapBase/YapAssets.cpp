#include "YapBase/YapAssets.h"

#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/AssetManager.h"

UObject* Yap::FindNamedAsset(const FPrimaryAssetType& Type, const UClass* Class, FName Name)
{
	if (Name.IsNone() || !Class)
	{
		return nullptr;
	}

	const FPrimaryAssetId Id(Type, Name);
	FSoftObjectPath Path;

	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		Path = AssetManager->GetPrimaryAssetPath(Id);
	}

	if (!Path.IsValid())
	{
		if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
		{
			TArray<FAssetData> Assets;
			AssetRegistry->GetAssetsByClass(Class->GetClassPathName(), Assets, true);
			for (const FAssetData& Asset : Assets)
			{
				if (Asset.GetPrimaryAssetId() == Id)
				{
					Path = Asset.GetSoftObjectPath();
					break;
				}
			}
		}
	}

	if (!Path.IsValid())
	{
		return nullptr;
	}

	UObject* Object = Path.TryLoad();
	return (Object && Object->IsA(Class)) ? Object : nullptr;
}

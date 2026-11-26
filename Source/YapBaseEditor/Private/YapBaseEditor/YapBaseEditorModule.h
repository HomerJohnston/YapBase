#pragma once

#include "AssetDefinition.h"
#include "AssetTypeCategories.h"
#include "Modules/ModuleInterface.h"

/** The content browser category Yap's assets are in. */
struct FYapBaseAssetCategoryPaths : EAssetCategoryPaths
{
	static FAssetCategoryPath Yap;
};

/**
 * The YapBase editor module.
 *
 * - Registers the "Yap" asset category (characters, speech configs, brokers), the character asset's details panel,
 *   property customizations and thumbnail, and the speech config's details panel.
 * - At startup, makes sure the Asset Manager knows Yap's primary asset types (speech configs and characters), so
 *   scripts can name them and they're packaged without being listed anywhere: if a rule is missing from Project
 *   Settings > Asset Manager, it's added (saved to DefaultGame.ini) and a notification says so.
 */
class FYapBaseEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** The "Yap" category in the content browser's Add menu. */
	static EAssetTypeCategories::Type GetAssetCategory() { return AssetCategory; }

private:
	void RegisterCustomizations();
	void UnregisterCustomizations();
	void EnsureAssetManagerRules();

	static EAssetTypeCategories::Type AssetCategory;

	FDelegateHandle PostEngineInitHandle;
	TArray<FName> CustomizedClasses;
	TArray<FName> CustomizedStructs;
};

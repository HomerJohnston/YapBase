using UnrealBuildTool;

/** YapBase editor support: asset types, settings and detail customizations that don't depend on any scripting system. */
public class YapBaseEditor : ModuleRules
{
	public YapBaseEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"AssetTools",
				"AssetDefinition",
				"PropertyEditor",
				"GameplayTags",
				"GameplayTagsEditor",
				"YapBase",
			}
		);
	}
}

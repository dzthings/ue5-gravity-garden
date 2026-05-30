using UnrealBuildTool;

public class GravityEntityEditor : ModuleRules
{
	public GravityEntityEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "UnrealEd", "GravityEntityRuntime"
		});
	}
}

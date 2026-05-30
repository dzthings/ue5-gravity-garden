using UnrealBuildTool;

public class GravityEntityRuntime : ModuleRules
{
	public GravityEntityRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine",
			"ProceduralMeshComponent", // runtime mesh generation for part shapes and link tubes
		});
	}
}

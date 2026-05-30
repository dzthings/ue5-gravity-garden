using UnrealBuildTool;

public class GravityEntityRuntime : ModuleRules
{
	public GravityEntityRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine",
			"GeometryCore",       // FDynamicMesh3 and core math types
			"GeometryFramework",  // UDynamicMesh, UDynamicMeshComponent
			"GeometryScript",     // UGeometryScriptLibrary_* primitive generators
		});
	}
}

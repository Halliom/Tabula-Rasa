namespace UnrealBuildTool.Rules
{
	public class VoxelLib : ModuleRules
	{
        public VoxelLib(TargetInfo Target)
        {
	    PublicIncludePaths.Add("VoxelLib/Private");
            PrivateIncludePaths.Add("VoxelLib/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "InputCore",
                    "RHI",
					"RenderCore",
					"ShaderCore",
				}
			);
		}
	}
}

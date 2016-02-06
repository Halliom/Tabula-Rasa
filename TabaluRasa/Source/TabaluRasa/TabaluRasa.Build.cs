// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TabaluRasa : ModuleRules
{
	public TabaluRasa(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "VoxelLib" });

		PrivateDependencyModuleNames.AddRange(new string[] { "VoxelLib" });
 		PrivateIncludePathModuleNames.AddRange(new string[] { "VoxelLib" });
 
		PrivateIncludePaths.AddRange(new string[] { "VoxelLib/Public", "VoxelLib/Classes", "VoxelLib/Private" });
		PublicIncludePaths.AddRange(new string[] { "VoxelLib/Public", "VoxelLib/Classes", "VoxelLib/Private" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
		// if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
		// {
		//		if (UEBuildConfiguration.bCompileSteamOSS == true)
		//		{
		//			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		//		}
		// }
	}
}

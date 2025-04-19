// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS_Shooter : ModuleRules
{
	public TPS_Shooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule", "UMG", "PhysicsCore", "Slate" });

		
    }
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RobotActionProject : ModuleRules
{
	public RobotActionProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RobotActionProject",
			"RobotActionProject/Variant_Platforming",
			"RobotActionProject/Variant_Platforming/Animation",
			"RobotActionProject/Variant_Combat",
			"RobotActionProject/Variant_Combat/AI",
			"RobotActionProject/Variant_Combat/Animation",
			"RobotActionProject/Variant_Combat/Gameplay",
			"RobotActionProject/Variant_Combat/Interfaces",
			"RobotActionProject/Variant_Combat/UI",
			"RobotActionProject/Variant_SideScrolling",
			"RobotActionProject/Variant_SideScrolling/AI",
			"RobotActionProject/Variant_SideScrolling/Gameplay",
			"RobotActionProject/Variant_SideScrolling/Interfaces",
			"RobotActionProject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

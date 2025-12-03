// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheGauntlet2 : ModuleRules
{
	public TheGauntlet2(ReadOnlyTargetRules Target) : base(Target)
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
			"CommonUI"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TheGauntlet2",
			"TheGauntlet2/Variant_Platforming",
			"TheGauntlet2/Variant_Platforming/Animation",
			"TheGauntlet2/Variant_Combat",
			"TheGauntlet2/Variant_Combat/AI",
			"TheGauntlet2/Variant_Combat/Animation",
			"TheGauntlet2/Variant_Combat/Gameplay",
			"TheGauntlet2/Variant_Combat/Interfaces",
			"TheGauntlet2/Variant_Combat/UI",
			"TheGauntlet2/Variant_SideScrolling",
			"TheGauntlet2/Variant_SideScrolling/AI",
			"TheGauntlet2/Variant_SideScrolling/Gameplay",
			"TheGauntlet2/Variant_SideScrolling/Interfaces",
			"TheGauntlet2/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

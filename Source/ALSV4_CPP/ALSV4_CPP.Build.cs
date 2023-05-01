// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

using UnrealBuildTool;

public class ALSV4_CPP : ModuleRules
{
	public ALSV4_CPP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
		PublicDependencyModuleNames.AddRange(new[]
			{
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"NavigationSystem",
				"AIModule",
				"ModularGameplay",
				"ModularGameplayActors",
				"GameplayTasks",
				"GameFeatures",
				"PhysicsCore",
				"Niagara", 
				"CommonLoadingScreen",
				"EnhancedInput"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"InputCore",
				"AudioMixer",
				"AudioModulation",
				"EnhancedInput",
				"NetCore",
				"CommonInput",
				"CommonGame",
				"CommonUser",
				"CommonUI",
				"GameplayMessageRuntime",
				"Slate",
				"SlateCore"
			}
		);
	}
}
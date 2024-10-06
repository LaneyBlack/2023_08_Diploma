// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheFallenSamurai : ModuleRules
{
	public TheFallenSamurai(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "DidItHit", "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AnimGraphRuntime", "UMG" });
		
		PrivateDependencyModuleNames.AddRange(new string[]{"GameplayAbilities", "GameplayTags", "GameplayTasks"});
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
// using UnrealBuildTool.Rules;

public class TheFallenSamurai : ModuleRules
{
	public TheFallenSamurai(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "DidItHit", "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AnimGraphRuntime", "MeshDescription", "ProceduralMeshComponent", "UMG" });
		
		PrivateDependencyModuleNames.AddRange(new string[]{"GameplayAbilities", "GameplayTags", "GameplayTasks", "MeshDescription", "StaticMeshDescription" });
    }
}

// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class MovellaXsensDotLibrary : ModuleRules
{
	public MovellaXsensDotLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/Public");

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{
            // Add public include path
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "MovellaXsensDotLibrary"));

            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "lib", "movelladot_pc_sdk64.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "lib", "xstypes64.lib"));
        }
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateSteamTarget : TemplateTarget
{
	public TemplateSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
		bOverrideBuildEnvironment = true;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateServerSteamTarget : TemplateServerTarget
{
	public TemplateServerSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
		bOverrideBuildEnvironment = true;
	}
}

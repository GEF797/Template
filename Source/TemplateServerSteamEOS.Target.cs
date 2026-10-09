// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateServerSteamEOSTarget : TemplateServerTarget
{
	public TemplateServerSteamEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "SteamEOS";
		bOverrideBuildEnvironment = true;
	}
}

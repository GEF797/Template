// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// SteamEOS runs through Steam while EOS supplies cross-platform online services.
public class TemplateSteamEOSTarget : TemplateTarget
{
	public TemplateSteamEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "SteamEOS";
		bOverrideBuildEnvironment = true;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateServerEOSTarget : TemplateServerTarget
{
	public TemplateServerEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "EOS";
		bOverrideBuildEnvironment = true;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateEOSTarget : TemplateTarget
{
	public TemplateEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "EOS";
		bOverrideBuildEnvironment = true;
	}
}

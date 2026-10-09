// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateClientTarget : TargetRules
{
	public TemplateClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("Template");
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TemplateEditorTarget : TargetRules
{
	public TemplateEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("Template");
	}
}

// Copyright Sami Kangasmaa, 2022

using UnrealBuildTool;
using System.IO;

public class QuestFrameworkEditor : ModuleRules
{
	public QuestFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"QuestFramework"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
                "CoreUObject",
				"ApplicationCore",
				"Projects",
				"UnrealEd",
				"GraphEditor",
				"RenderCore",
                "InputCore",
                "SlateCore",
                "Slate",
                "EditorStyle",
                "MessageLog",
                "EditorWidgets",
				"WorkspaceMenuStructure",
				"DetailCustomizations",
                "PropertyEditor",
				"BlueprintGraph",
                "Kismet",
                "KismetCompiler",
                "KismetWidgets",
                "ContentBrowser",
				"ToolMenus",
				"ContentBrowserData",
				"EditorFramework"
			});

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

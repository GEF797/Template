// Copyright Sami Kangasmaa, 2022


#include "QuestEditor/QuestEditorStyles.h"
#include "Interfaces/IPluginManager.h"

FSlateColor FQuestEditorStyles::InputPinColor = FSlateColor(FColor(17, 21, 26, 255));
FSlateColor FQuestEditorStyles::InputPinColor_Hover = FSlateColor(FColor(255, 255, 255, 255));

FSlateColor FQuestEditorStyles::OutputPinColor = FSlateColor(FColor(17, 21, 26, 255));
FSlateColor FQuestEditorStyles::OutputPinColor_Hover = FSlateColor(FColor(255, 255, 255, 255));
FSlateColor FQuestEditorStyles::OutputPinColor_Completed = FSlateColor(FColor(0, 64, 18, 255));
FSlateColor FQuestEditorStyles::OutputPinColor_Failed = FSlateColor(FColor(64, 0, 0, 255));

FSlateColor FQuestEditorStyles::ConnectionColor = FSlateColor(FColor(128, 128, 128, 255));
FSlateColor FQuestEditorStyles::ConnectionColor_Conditions = FSlateColor(FColor(230, 161, 14, 255));
FSlateColor FQuestEditorStyles::ConnectionColor_Hover = FSlateColor(FColor(255, 255, 255, 255));

FSlateColor FQuestEditorStyles::CommonNodeBorderColor = FSlateColor(FColor(9,11,13, 255));

FLinearColor FQuestEditorStyles::NodeBodyColor_Root = FColor(77, 77, 77, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Stage = FColor(14, 82, 230, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Selector = FColor(138, 114, 19, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_End_Complete = FColor(19, 138, 19, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_End_Failed = FColor(138, 19, 19, 255);

FLinearColor FQuestEditorStyles::NodeBodyColor_Objective = FColor(0, 22, 99, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Objective_Optional = FColor(64, 64, 64, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Objective_RequireComplete = FColor(17, 0, 150, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Objective_ForceComplete = FColor(14, 74, 0, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Objective_ForceFail = FColor(74, 1, 0, 255);
FLinearColor FQuestEditorStyles::NodeBodyColor_Objective_ForceCompleteFail = FColor(74, 37, 0, 255);

FLinearColor FQuestEditorStyles::NodeBodyColor_Service = FColor(13, 102, 107, 255);

FLinearColor FQuestEditorStyles::VariableLabelColor_Int = FColor(0, 22, 99, 255);
FLinearColor FQuestEditorStyles::VariableLabelColor_Float = FColor(14, 74, 0, 255);
FLinearColor FQuestEditorStyles::VariableLabelColor_Bool = FColor(74, 1, 0, 255);

// --------------------------------------------------------------------------------------------- //

TSharedPtr<FSlateStyleSet> FQuestEditorStyles::SlateStyleSet = nullptr;

void FQuestEditorStyles::InitializeSlateStyles()
{
	if (SlateStyleSet.IsValid())
	{
		return;
	}

	SlateStyleSet = MakeShared<FSlateStyleSet>(TEXT("QuestFrameworkStyle"));
	FString PluginContentPath = TEXT("");

	TSharedPtr<IPlugin> QuestFrameworkPlugin = IPluginManager::Get().FindPlugin(TEXT("QuestFramework"));
	if (!QuestFrameworkPlugin.IsValid())
	{
		return;
	}

	SlateStyleSet->SetContentRoot(QuestFrameworkPlugin->GetContentDir());
	PluginContentPath = QuestFrameworkPlugin->GetContentDir();

	// Set thumbnails
	const FVector2D ThumbnailSize = FVector2D(64, 64);

	SlateStyleSet->Set(TEXT("ClassThumbnail.Quest"), new FSlateImageBrush(PluginContentPath / "Thumbnails/Quest_Thumbnail.png", ThumbnailSize));
	SlateStyleSet->Set(TEXT("ClassThumbnail.QuestCondition"), new FSlateImageBrush(PluginContentPath / "Thumbnails/QuestCondition_Thumbnail.png", ThumbnailSize));
	SlateStyleSet->Set(TEXT("ClassThumbnail.QuestEvent"), new FSlateImageBrush(PluginContentPath / "Thumbnails/QuestEvent_Thumbnail.png", ThumbnailSize));
	SlateStyleSet->Set(TEXT("ClassThumbnail.QuestService"), new FSlateImageBrush(PluginContentPath / "Thumbnails/QuestService_Thumbnail.png", ThumbnailSize));
	SlateStyleSet->Set(TEXT("ClassThumbnail.QuestObjectiveObserver"), new FSlateImageBrush(PluginContentPath / "Thumbnails/QuestObjectiveObserver_Thumbnail.png", ThumbnailSize));
	SlateStyleSet->Set(TEXT("ClassThumbnail.QuestTextArgument"), new FSlateImageBrush(PluginContentPath / "Thumbnails/QuestTextArgument_Thumbnail.png", ThumbnailSize));

	FSlateStyleRegistry::RegisterSlateStyle(*SlateStyleSet.Get());
}

void FQuestEditorStyles::UninitializeSlateStyles()
{
	if (SlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*SlateStyleSet.Get());
		SlateStyleSet.Reset();
	}
}
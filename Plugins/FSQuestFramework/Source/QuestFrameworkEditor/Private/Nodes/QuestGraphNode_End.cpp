// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestNode_End.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode_End"

FLinearColor UQuestGraphNode_End::GetNodeStyleColor() const
{
	return GetResultType() == EQuestResult::Completed ? FQuestEditorStyles::NodeBodyColor_End_Complete : FQuestEditorStyles::NodeBodyColor_End_Failed;
}

FText UQuestGraphNode_End::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText TitleText = LOCTEXT("NodeTitle", "End Quest: {Result}");
	
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Result"), GetResultType() == EQuestResult::Completed ? FText::FromString(TEXT("Completed")) : FText::FromString(TEXT("Failed")));

	return FText::Format(TitleText, Arguments);
}

TSubclassOf<UQuestNode> UQuestGraphNode_End::GetQuestNodeClass() const
{
	return UQuestNode_End::StaticClass();
}

EQuestResult UQuestGraphNode_End::GetResultType() const
{
	if (UQuestNode_End* EndNode = Cast<UQuestNode_End>(QuestNode))
	{
		return EndNode->GetEndResultType();
	}
	return EQuestResult::Failed;
}

#undef LOCTEXT_NAMESPACE
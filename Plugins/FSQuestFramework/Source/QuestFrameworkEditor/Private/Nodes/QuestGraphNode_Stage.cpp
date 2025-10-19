// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNode_Stage.h"
#include "Nodes/QuestNode_Stage.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode_Stage"

FText UQuestGraphNode_Stage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText TitleText = LOCTEXT("NodeTitle", "{Name}");

	FString LogicalName = FString(TEXT("Unknown"));
	if (UQuestNode_Stage* Node = GetQuestNodeUnchecked<UQuestNode_Stage>())
	{
		LogicalName = FString(Node->GetLogicalName().ToString());
	}

	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Name"), FText::FromString(LogicalName));

	return FText::Format(TitleText, Arguments);
}

TSubclassOf<UQuestNode> UQuestGraphNode_Stage::GetQuestNodeClass() const
{
	return UQuestNode_Stage::StaticClass();
}

void UQuestGraphNode_Stage::CreateOutputPins()
{
	static const FName PinCategory(TEXT("OutputCategory"));

	static const FName PinNameA(TEXT("Output Pin Completed"));
	FCreatePinParams PinParamsA;
	PinParamsA.Index = QUEST_GRAPH_PIN_OUTPUT1;

	static const FName PinNameB(TEXT("Output Pin Failed"));
	FCreatePinParams PinParamsB;
	PinParamsB.Index = QUEST_GRAPH_PIN_OUTPUT2;

	CreatePin(EGPD_Output, PinCategory, PinNameA, PinParamsA);
	CreatePin(EGPD_Output, PinCategory, PinNameB, PinParamsB);
}

#undef LOCTEXT_NAMESPACE
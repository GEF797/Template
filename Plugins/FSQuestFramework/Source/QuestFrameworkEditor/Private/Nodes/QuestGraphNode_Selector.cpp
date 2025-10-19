// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestNode_Selector.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode_Selector"

FText UQuestGraphNode_Selector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Selector"));
}

TSubclassOf<UQuestNode> UQuestGraphNode_Selector::GetQuestNodeClass() const
{
	return UQuestNode_Selector::StaticClass();
}

#undef LOCTEXT_NAMESPACE
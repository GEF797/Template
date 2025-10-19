// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestNode_Root.h"

FText UQuestGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Root"));
}

TSubclassOf<UQuestNode> UQuestGraphNode_Root::GetQuestNodeClass() const
{
	return UQuestNode_Root::StaticClass();
}
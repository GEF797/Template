// Copyright Sami Kangasmaa, 2022


#include "Factory/QuestGraphFactory.h"
#include "K2Node.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphSchema.h"

#include "Nodes/Slate/SQuestGraphPin.h"
#include "Nodes/Slate/SQuestGraphNodeBase.h"
#include "Nodes/Slate/SQuestGraphNodeCommonBase.h"
#include "Nodes/Slate/SQuestGraphNode_Connection.h"
#include "Nodes/Slate/SQuestGraphNode_Stage.h"

TSharedPtr<SGraphPin> FQuestGraphPinFactory::CreatePin(class UEdGraphPin* Pin) const
{
	if (Pin->GetSchema()->IsA<UQuestGraphSchema>())
	{
		return SNew(SQuestGraphPin, Pin);
	}
	return nullptr;
}

TSharedPtr<SGraphNode> FQuestGraphNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	if (UQuestGraphNode_Stage* StageNode = Cast<UQuestGraphNode_Stage>(InNode))
	{
		return SNew(SQuestGraphNode_Stage, StageNode);
	}

	if (UQuestGraphNodeCommonBase* CommonBaseNode = Cast<UQuestGraphNodeCommonBase>(InNode))
	{
		return SNew(SQuestGraphNodeCommonBase, CommonBaseNode);
	}

	if (UQuestGraphNode_Connection* Connection = Cast<UQuestGraphNode_Connection>(InNode))
	{
		return SNew(SQuestGraphNode_Connection, Connection);
	}
	return nullptr;
}



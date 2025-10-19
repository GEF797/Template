// Copyright Sami Kangasmaa, 2022


#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphSchema.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "GraphEditAction.h"

TArray<UQuestGraphNode_Root*> UQuestGraph::GetGraphNodes_Root() const
{
	TArray<UQuestGraphNode_Root*> NodeArray;
	GetNodesOfClass<UQuestGraphNode_Root>(NodeArray);
	return NodeArray;
}

TArray<UQuestGraphNode_Selector*> UQuestGraph::GetGraphNodes_Selector() const
{
	TArray<UQuestGraphNode_Selector*> NodeArray;
	GetNodesOfClass<UQuestGraphNode_Selector>(NodeArray);
	return NodeArray;
}

TArray<UQuestGraphNode_Connection*> UQuestGraph::GetGraphNodes_Connection() const
{
	TArray<UQuestGraphNode_Connection*> NodeArray;
	GetNodesOfClass<UQuestGraphNode_Connection>(NodeArray);
	return NodeArray;
}

TArray<UQuestGraphNode_End*> UQuestGraph::GetGraphNodes_End() const
{
	TArray<UQuestGraphNode_End*> NodeArray;
	GetNodesOfClass<UQuestGraphNode_End>(NodeArray);
	return NodeArray;
}

TArray<UQuestGraphNode_Stage*> UQuestGraph::GetGraphNodes_Stage() const
{
	TArray<UQuestGraphNode_Stage*> NodeArray;
	GetNodesOfClass<UQuestGraphNode_Stage>(NodeArray);
	return NodeArray;
}

bool UQuestGraph::RemoveQuestGraphNode(UEdGraphNode* Node)
{
	Modify();
	const int32 NumRemoved = Nodes.Remove(Node);

	GetSchema()->BreakNodeLinks(*Node);

	FEdGraphEditAction RemovalAction;
	RemovalAction.Graph = this;
	RemovalAction.Action = GRAPHACTION_RemoveNode;
	RemovalAction.Nodes.Add(Node);
	NotifyGraphChanged(RemovalAction);

	return NumRemoved > 0;
}

void UQuestGraph::RemoveAllNodes()
{
	Modify();
	Nodes.Empty();
}

bool UQuestGraph::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bModified = Super::Modify(bAlwaysMarkDirty);
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNodeBase* QuestNode = Cast<UQuestGraphNodeBase>(Node))
		{
			bModified = bModified && QuestNode->Modify(bAlwaysMarkDirty);
		}
	}

	return bModified;
}

UQuest* UQuestGraph::GetQuest() const
{
	return CastChecked<UQuest>(GetOuter());
}

const UQuestGraphSchema* UQuestGraph::GetQuestGraphSchema() const
{
	return GetDefault<UQuestGraphSchema>(Schema);
}
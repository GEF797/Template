// Copyright Sami Kangasmaa, 2022


#include "Graph/Actions/GraphSchemaAction_NewQuestNode.h"

#include "ScopedTransaction.h"

#include "Quest.h"
#include "Graph/QuestGraph.h"
#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "Nodes/QuestNode_Stage.h"

#define LOCTEXT_NAMESPACE "GraphSchemaAction_NewQuestNode"

FGraphSchemaAction_NewQuestNode::FGraphSchemaAction_NewQuestNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping, TSubclassOf<UQuestGraphNodeBase> InGraphNodeClass)
	: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
{
	check(InGraphNodeClass != nullptr);
	GraphNodeClass = InGraphNodeClass;
}

UEdGraphNode* FGraphSchemaAction_NewQuestNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	const FScopedTransaction Transaction(LOCTEXT("QuestActionNewNode", "Quest Editor: New Node"));

	UQuest* Quest = CastChecked<UQuestGraph>(ParentGraph)->GetQuest();

	ParentGraph->Modify();
	if (FromPin)
		FromPin->Modify();
	Quest->Modify();

	UEdGraphNode* GraphNode = CreateGraphNode(Quest, ParentGraph, FromPin, Location, bSelectNewNode);
	Quest->PostEditChange();
	Quest->MarkPackageDirty();
	ParentGraph->NotifyGraphChanged();

	return GraphNode;
}

UEdGraphNode* FGraphSchemaAction_NewQuestNode::CreateGraphNode(UQuest* Quest, UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D& Location, bool bSelectNewNode)
{
	check(GraphNodeClass != nullptr);

	if (GraphNodeClass == UQuestGraphNode_Root::StaticClass())
	{
		FGraphNodeCreator<UQuestGraphNode_Root> NodeCreator(*ParentGraph);
		UQuestGraphNode_Root* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

		GraphNode->ConstructQuestNode();

		NodeCreator.Finalize(); 
		GraphNode->SetPosition(Location.X, Location.Y);
		return CastChecked<UEdGraphNode>(GraphNode);
	}
	else if (GraphNodeClass == UQuestGraphNode_Selector::StaticClass())
	{
		FGraphNodeCreator<UQuestGraphNode_Selector> NodeCreator(*ParentGraph);
		UQuestGraphNode_Selector* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

		GraphNode->ConstructQuestNode();

		NodeCreator.Finalize();
		GraphNode->AutowireNewNode(FromPin);

		GraphNode->SetPosition(Location.X, Location.Y);
		return CastChecked<UEdGraphNode>(GraphNode);
	}
	else if (GraphNodeClass == UQuestGraphNode_End::StaticClass())
	{
		FGraphNodeCreator<UQuestGraphNode_End> NodeCreator(*ParentGraph);
		UQuestGraphNode_End* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

		GraphNode->ConstructQuestNode();

		NodeCreator.Finalize();

		GraphNode->AutowireNewNode(FromPin);
		GraphNode->SetPosition(Location.X, Location.Y);
		return CastChecked<UEdGraphNode>(GraphNode);
	}
	else if (GraphNodeClass == UQuestGraphNode_Stage::StaticClass())
	{
		FGraphNodeCreator<UQuestGraphNode_Stage> NodeCreator(*ParentGraph);
		UQuestGraphNode_Stage* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

		GraphNode->ConstructQuestNode();

		if (UQuestGraph* QuestGraph = Cast<UQuestGraph>(ParentGraph))
		{
			UQuestNode_Stage* Stage = GraphNode->GetQuestNodeChecked<UQuestNode_Stage>();
			Stage->SuggestLogicalName(QuestGraph->GetNextSuggestedStageName());
		}

		NodeCreator.Finalize();

		GraphNode->AutowireNewNode(FromPin);
		GraphNode->SetPosition(Location.X, Location.Y);
		return CastChecked<UEdGraphNode>(GraphNode);
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE

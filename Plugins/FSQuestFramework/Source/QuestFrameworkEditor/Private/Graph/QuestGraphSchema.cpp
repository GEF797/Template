// Copyright Sami Kangasmaa, 2022


#include "Graph/QuestGraphSchema.h"
#include "Graph/QuestGraph.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "Graph/QuestConnectionDrawingPolicy.h"
#include "Graph/Actions/GraphSchemaAction_NewQuestNode.h"

#define LOCTEXT_NAMESPACE "QuestGraphSchema"

void UQuestGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetNodeActions(ContextMenuBuilder);
}

void UQuestGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Node && !Context->bIsDebugging)
	{
		FToolMenuSection& Section = Menu->AddSection("QuestGraphSchemaNodeActions", LOCTEXT("NodeActionsTitle", "Actions"));
		Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
	}
	Super::GetContextMenuActions(Menu, Context);
}

void UQuestGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	check(Graph.Nodes.Num() == 0);

	// Create root node
	FGraphNodeCreator<UQuestGraphNode_Root> GraphNodeCreator(Graph);
	UQuestGraphNode_Root* GraphNode_Root = GraphNodeCreator.CreateUserInvokedNode(false);

	GraphNode_Root->ConstructQuestNode();

	GraphNodeCreator.Finalize();
	GraphNode_Root->SetPosition(0,0);
}

FPinConnectionResponse UQuestGraphSchema::MovePinLinks(UEdGraphPin& MoveFromPin, UEdGraphPin& MoveToPin, bool bIsIntermediateMove, bool bNotifyLinkedNodes) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

FPinConnectionResponse UQuestGraphSchema::CopyPinLinks(UEdGraphPin& CopyFromPin, UEdGraphPin& CopyToPin, bool bIsIntermediateCopy) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT(""));
}

const FPinConnectionResponse UQuestGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	UQuestGraphNodeBase* NodeA = CastChecked<UQuestGraphNodeBase>(PinA->GetOwningNode());
	UQuestGraphNodeBase* NodeB = CastChecked<UQuestGraphNodeBase>(PinB->GetOwningNode());

	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connection to same node"));
	}

	if (PinA == PinB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connection to same pin"));
	}

	if (!NodeA->CanHaveOutputConnections())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Output connections not supported in this node"));
	}

	if (!NodeA->CanHaveMultipleOutputConnections() && PinA->Direction == EEdGraphPinDirection::EGPD_Output)
	{
		if (PinA == NodeA->GetOutputPinA() && NodeA->GetOutputPinA()->LinkedTo.Num() > 0)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Only one child allowed"));
		}
		else if (NodeA->HasOutputPinB() && PinA == NodeA->GetOutputPinB() && NodeA->GetOutputPinB()->LinkedTo.Num() > 0)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Only one child allowed"));
		}
	}

	if (!NodeB->CanHaveInputConnections())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Input connections not supported in target node"));
	}

	if (NodeA->HasConnectionToNode(NodeB, EEdGraphPinDirection::EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connection exists already"));
	}

	if (NodeA->IsA(UQuestGraphNode_Connection::StaticClass()) && NodeB->IsA(UQuestGraphNode_Connection::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connections can't connect to other connections"));
	}

	if (NodeA->IsA(UQuestGraphNode_Connection::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
	}

	if (NodeB->IsA(UQuestGraphNode_Connection::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT(""));
	}

	if (!NodeA->IsA(UQuestGraphNode_Connection::StaticClass()) && !NodeB->IsA(UQuestGraphNode_Connection::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create connection"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

bool UQuestGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	if (PinA == PinB)
	{
		return false;
	}

	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return false;
	}

	UEdGraph* Graph = PinA->GetOwningNode()->GetGraph();
	PinA->GetOwningNode()->Modify();
	PinB->GetOwningNode()->Modify();
	Graph->Modify();
	GetQuest(Graph)->Modify();

	const bool bConnectionSuccess = Super::TryCreateConnection(PinA, PinB);
	if (bConnectionSuccess)
	{
		GetQuest(Graph)->CompileInEditor();
	}
	return bConnectionSuccess;
}

bool UQuestGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	UQuestGraphNodeCommonBase* NodeA = CastChecked<UQuestGraphNodeCommonBase>(PinA->GetOwningNode());
	UQuestGraphNodeCommonBase* NodeB = CastChecked<UQuestGraphNodeCommonBase>(PinB->GetOwningNode());
	UEdGraph* Graph = NodeA->GetGraph();

	FGraphNodeCreator<UQuestGraphNode_Connection> GraphNodeCreator(*Graph);
	UQuestGraphNode_Connection* GraphNode_Connection = GraphNodeCreator.CreateUserInvokedNode(false);
	GraphNodeCreator.Finalize();
	GraphNode_Connection->SetPosition(NodeA->GetPosition().X, NodeA->GetPosition().Y);
	GraphNode_Connection->SetFlags(RF_Transactional);

	if (PinA->Direction == EGPD_Output)
	{
		const bool bBPin = NodeA->HasOutputPinB() ? PinA == NodeA->GetOutputPinB() : false;
		GraphNode_Connection->CreateConnections(NodeA, NodeB, bBPin);
	}
	else
	{
		GraphNode_Connection->CreateConnections(NodeB, NodeA, false);
	}

	Graph->NotifyGraphChanged();

	return true;
}

bool UQuestGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return true;
}

void UQuestGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	UEdGraph* Graph = TargetNode.GetGraph();
	UQuest* Quest = GetQuest(Graph);

	Graph->Modify();
	TargetNode.Modify();
	Quest->Modify();

	Super::BreakNodeLinks(TargetNode);

	Quest->CompileInEditor();
}

void UQuestGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakPinLinks", "Quest Editor: Break Pin Links"));

	UEdGraphNode* Node = TargetPin.GetOwningNode();
	UEdGraph* Graph = Node->GetGraph();
	UQuest* Quest = GetQuest(Graph);

	Node->Modify();
	Graph->Modify();
	Quest->Modify();

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	if (bSendsNodeNotifcation)
	{
		Quest->CompileInEditor();
	}
}

void UQuestGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakSinglePinLink", "Quest Editor: Break Pin Link"));
	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

void UQuestGraphSchema::BreakPinLinkToPin(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakPinLink", "Quest Editor: Break Pin Link"));

	UEdGraphNode* SourceNode = SourcePin->GetOwningNode();
	UEdGraphNode* TargetNode = TargetPin->GetOwningNode();
	UEdGraph* Graph = SourceNode->GetGraph();
	UQuest* Quest = GetQuest(Graph);

	SourceNode->Modify();
	TargetNode->Modify();
	Graph->Modify();
	Quest->Modify();

	SourcePin->BreakLinkTo(TargetPin);

	SourceNode->PinConnectionListChanged(SourcePin);
	TargetNode->PinConnectionListChanged(TargetPin);

	if (bSendsNodeNotifcation)
	{
		SourceNode->NodeConnectionListChanged();
		TargetNode->NodeConnectionListChanged();

		Quest->CompileInEditor();
	}
}

int32 UQuestGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	return 0; // TODO
}

FConnectionDrawingPolicy* UQuestGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FQuestConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

UQuest* UQuestGraphSchema::GetQuest(UEdGraph* Graph) const
{
	return CastChecked<UQuestGraph>(Graph)->GetQuest();
}

void UQuestGraphSchema::GetNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	const FText Category(LOCTEXT("QuestNodeAction", "Quest Node"));
	const int32 Group = 0;

	// Selector
	{
		const FText MenuDesc = FText::FromString(TEXT("Selector"));
		const FText Tooltip = FText::FromString(TEXT("Add Selector"));
		TSharedPtr<FGraphSchemaAction_NewQuestNode> Action(new FGraphSchemaAction_NewQuestNode(Category, MenuDesc, Tooltip, Group, UQuestGraphNode_Selector::StaticClass()));
		ActionMenuBuilder.AddAction(Action);
	}

	// End
	{
		const FText MenuDesc = FText::FromString(TEXT("End Quest"));
		const FText Tooltip = FText::FromString(TEXT("Add End Quest"));
		TSharedPtr<FGraphSchemaAction_NewQuestNode> Action(new FGraphSchemaAction_NewQuestNode(Category, MenuDesc, Tooltip, Group, UQuestGraphNode_End::StaticClass()));
		ActionMenuBuilder.AddAction(Action);
	}

	// Stage
	{
		const FText MenuDesc = FText::FromString(TEXT("Quest Stage"));
		const FText Tooltip = FText::FromString(TEXT("Add Quest Stage"));
		TSharedPtr<FGraphSchemaAction_NewQuestNode> Action(new FGraphSchemaAction_NewQuestNode(Category, MenuDesc, Tooltip, Group, UQuestGraphNode_Stage::StaticClass()));
		ActionMenuBuilder.AddAction(Action);
	}

}

#undef LOCTEXT_NAMESPACE

// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNodeCommonBase.h"
#include "Nodes/QuestGraphNode_Connection.h"

bool UQuestGraphNodeCommonBase::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bWasModified = Super::Modify(bAlwaysMarkDirty);

	if (QuestNode)
	{
		bWasModified = bWasModified && QuestNode->Modify(bAlwaysMarkDirty);
	}

	if (HasOutputPinA() || HasOutputPinB())
	{
		TArray<UQuestGraphNode_Connection*> Connections = GetConnectionsToChildren();
		for (UQuestGraphNode_Connection* Connection : Connections)
		{
			bWasModified = bWasModified && Connection->ModifyBase();
		}
	}

	return bWasModified;
}

void UQuestGraphNodeCommonBase::PostLoad()
{
	Super::PostLoad();

	if (IsValid(QuestNode))
	{
		QuestNode->SetFlags(RF_Transactional);
	}
	else
	{
		ConstructQuestNode();
	}
}


TArray<UQuestGraphNode_Connection*> UQuestGraphNodeCommonBase::GetConnectionsToParents() const
{
	TArray<UQuestGraphNode_Connection*> Connections;
	for (const UEdGraphPin* LinkPin : GetInputPin()->LinkedTo)
	{
		Connections.Add(CastChecked<UQuestGraphNode_Connection>(LinkPin->GetOwningNode()));
	}

	return Connections;
}

TArray<UQuestGraphNode_Connection*> UQuestGraphNodeCommonBase::GetConnectionsToChildren() const
{
	TArray<UQuestGraphNode_Connection*> Connections;
	for (const UEdGraphPin* LinkPin : GetOutputPinA()->LinkedTo)
	{
		Connections.Add(CastChecked<UQuestGraphNode_Connection>(LinkPin->GetOwningNode()));
	}

	if (HasOutputPinB())
	{
		for (const UEdGraphPin* LinkPin : GetOutputPinB()->LinkedTo)
		{
			Connections.Add(CastChecked<UQuestGraphNode_Connection>(LinkPin->GetOwningNode()));
		}
	}

	return Connections;
}

TArray<UQuestGraphNode_Connection*> UQuestGraphNodeCommonBase::GetConnectionsToChildrenOfPin(bool bOutputPinA) const
{
	TArray<UQuestGraphNode_Connection*> Connections;

	if (bOutputPinA)
	{
		for (const UEdGraphPin* LinkPin : GetOutputPinA()->LinkedTo)
		{
			Connections.Add(CastChecked<UQuestGraphNode_Connection>(LinkPin->GetOwningNode()));
		}
	}
	else
	{
		if (HasOutputPinB())
		{
			for (const UEdGraphPin* LinkPin : GetOutputPinB()->LinkedTo)
			{
				Connections.Add(CastChecked<UQuestGraphNode_Connection>(LinkPin->GetOwningNode()));
			}
		}
	}

	return Connections;
}

TArray<UQuestGraphNodeCommonBase*> UQuestGraphNodeCommonBase::GetParentNodes() const
{
	TArray<UQuestGraphNodeCommonBase*> Nodes;
	TArray<UQuestGraphNode_Connection*> Connections = GetConnectionsToParents();
	for (const UQuestGraphNode_Connection* Connection : Connections)
	{
		Nodes.Add(Connection->GetParentNode());
	}

	return Nodes;
}

TArray<UQuestGraphNodeCommonBase*> UQuestGraphNodeCommonBase::GetChildrenNodes() const
{
	TArray<UQuestGraphNodeCommonBase*> Nodes;
	TArray<UQuestGraphNode_Connection*> Connections = GetConnectionsToChildren();
	for (const UQuestGraphNode_Connection* Connection : Connections)
	{
		Nodes.Add(Connection->GetChildNode());
	}

	return Nodes;
}

TArray<UQuestGraphNodeCommonBase*> UQuestGraphNodeCommonBase::GetChildrenNodesOfPin(bool bOutputPinA) const
{
	TArray<UQuestGraphNodeCommonBase*> Nodes;
	TArray<UQuestGraphNode_Connection*> Connections = GetConnectionsToChildrenOfPin(bOutputPinA);
	for (const UQuestGraphNode_Connection* Connection : Connections)
	{
		Nodes.Add(Connection->GetChildNode());
	}

	return Nodes;
}

bool UQuestGraphNodeCommonBase::HasConnectionToNode(const UEdGraphNode* TargetNode, EEdGraphPinDirection Direction) const
{
	if (Super::HasConnectionToNode(TargetNode, Direction))
	{
		return true;
	}

	// Check connections
	if (Direction == EGPD_Output)
	{
		TArray<UQuestGraphNodeCommonBase*> Children = GetChildrenNodes();
		for (UQuestGraphNodeCommonBase* Node : Children)
		{
			if (TargetNode == Node)
			{
				return true;
			}
		}
	}
	else
	{
		TArray<UQuestGraphNodeCommonBase*> Parents = GetParentNodes();
		for (UQuestGraphNodeCommonBase* Node : Parents)
		{
			if (TargetNode == Node)
			{
				return true;
			}
		}
	}

	return false;
}

void UQuestGraphNodeCommonBase::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (!FromPin)
	{
		return;
	}

	if (!ensure(FromPin->Direction == EEdGraphPinDirection::EGPD_Output))
	{
		return;
	}
	if (!ensure(FromPin->GetOwningNode() != this))
	{
		return;
	}
	
	const UQuestGraphSchema* Schema = GetQuestGraphSchema();
	check(Schema);

	Schema->TryCreateConnection(FromPin, GetInputPin());

	check(FromPin->GetOwningNode());
	FromPin->GetOwningNode()->NodeConnectionListChanged();
}

void UQuestGraphNodeCommonBase::CreateInputPin()
{
	static const FName PinCategory(TEXT("InputCategory"));
	static const FName PinName(TEXT("Input Pin"));
	FCreatePinParams PinParams;
	PinParams.Index = QUEST_GRAPH_PIN_INPUT;
	CreatePin(EGPD_Input, PinCategory, PinName, PinParams);
}

void UQuestGraphNodeCommonBase::CreateOutputPins()
{
	static const FName PinCategory(TEXT("OutputCategory"));
	static const FName PinName(TEXT("Output Pin 1"));
	FCreatePinParams PinParams;
	PinParams.Index = QUEST_GRAPH_PIN_OUTPUT1;

	CreatePin(EGPD_Output, PinCategory, PinName, PinParams);
	GetOutputPinA()->bNotConnectable = !CanHaveOutputConnections();
}

void UQuestGraphNodeCommonBase::ConstructQuestNode()
{
	if (!IsValid(QuestNode))
	{
		QuestNode = ConstructQuestNodeT<UQuestNode>(GetQuestNodeClass());
		QuestNode->SetFlags(RF_Transactional);
	}
}
// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "Nodes/QuestGraphNode_Selector.h"

bool UQuestGraphNode_Connection::ModifyBase(bool bAlwaysMarkDirty)
{
	return Super::Modify(bAlwaysMarkDirty);
}

bool UQuestGraphNode_Connection::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bModified = Super::Modify(bAlwaysMarkDirty);
	if (HasParentNode())
	{
		bModified = bModified && GetParentNode()->Modify(bAlwaysMarkDirty);
	}
	return bModified;
}

void UQuestGraphNode_Connection::CreateConnections(UQuestGraphNodeCommonBase* ParentNode, UQuestGraphNodeCommonBase* ChildNode, bool bBParentOutputPin)
{
	check(ParentNode != ChildNode);

	UEdGraphPin* InputPin = GetInputPin();
	UEdGraphPin* OutputPin = GetOutputPinA();
	InputPin->Modify();
	InputPin->LinkedTo.Empty();
	OutputPin->Modify();
	OutputPin->LinkedTo.Empty();

	if (!bBParentOutputPin)
	{
		ParentNode->GetOutputPinA()->MakeLinkTo(InputPin);
	}
	else
	{
		ParentNode->GetOutputPinB()->MakeLinkTo(InputPin);
	}

	OutputPin->MakeLinkTo(ChildNode->GetInputPin());

	check(InputPin->LinkedTo.Num() == 1);
	check(OutputPin->LinkedTo.Num() == 1);
}

void UQuestGraphNode_Connection::PinConnectionListChanged(UEdGraphPin* Pin)
{
	bool bDestroyConnectionNode = Pin == nullptr;
	if (Pin && Pin->LinkedTo.Num() <= 0)
	{
		bDestroyConnectionNode = true;
	}

	if (bDestroyConnectionNode)
	{
		Modify();
		DestroyNode();
	}
}

bool UQuestGraphNode_Connection::HasParentNode() const
{
	if (HasInputPin())
	{
		UEdGraphPin* InputPin = GetInputPin();
		if (InputPin->LinkedTo.Num() <= 0)
		{
			return false;
		}
		if (InputPin->LinkedTo[0] == nullptr)
		{
			return false;
		}
		if (InputPin->LinkedTo[0]->GetOwningNodeUnchecked() == nullptr)
		{
			return false;
		}
		return true;
	}
	return false;
}

bool UQuestGraphNode_Connection::HasChildNode() const
{
	if (HasOutputPinA())
	{
		UEdGraphPin* OutputPin = GetOutputPinA();
		if (OutputPin->LinkedTo.Num() <= 0)
		{
			return false;
		}
		if (OutputPin->LinkedTo[0] == nullptr)
		{
			return false;
		}
		if (OutputPin->LinkedTo[0]->GetOwningNodeUnchecked() == nullptr)
		{
			return false;
		}
		return true;
	}

	return false;
}

UQuestGraphNodeCommonBase* UQuestGraphNode_Connection::GetParentNode() const
{
	check(HasParentNode());
	return CastChecked<UQuestGraphNodeCommonBase>(GetInputPin()->LinkedTo[0]->GetOwningNode());
}

UQuestGraphNodeCommonBase* UQuestGraphNode_Connection::GetChildNode() const
{
	check(HasChildNode());
	return CastChecked<UQuestGraphNodeCommonBase>(GetOutputPinA()->LinkedTo[0]->GetOwningNode());
}

UEdGraphPin* UQuestGraphNode_Connection::GetParentLinkPin() const
{
	check(HasParentNode());
	return GetInputPin()->LinkedTo[0];
}

UEdGraphPin* UQuestGraphNode_Connection::GetChildLinkPin() const
{
	check(HasChildNode());
	return GetOutputPinA()->LinkedTo[0];
}

bool UQuestGraphNode_Connection::HasSelectorParent() const
{
	if (UQuestGraphNode_Selector* SelectorParent = Cast<UQuestGraphNode_Selector>(GetParentNode()))
	{
		return true;
	}
	return false;
}

void UQuestGraphNode_Connection::CreateInputPin()
{
	static const FName PinName(TEXT("Input"));
	static const FName CategoryName(TEXT("Connection"));
	FCreatePinParams PinParams;
	PinParams.Index = QUEST_GRAPH_PIN_INPUT;
	CreatePin(EGPD_Input, CategoryName, PinName, PinParams);
}

void UQuestGraphNode_Connection::CreateOutputPins()
{
	static const FName PinName(TEXT("Output"));
	static const FName CategoryName(TEXT("Connection"));
	FCreatePinParams PinParams;
	PinParams.Index = QUEST_GRAPH_PIN_OUTPUT1;
	CreatePin(EGPD_Output, CategoryName, PinName, PinParams);
}
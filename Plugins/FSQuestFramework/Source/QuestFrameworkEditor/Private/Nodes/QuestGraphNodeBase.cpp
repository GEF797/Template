// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestGraphNodeBase.h"

UQuestGraphNodeBase::UQuestGraphNodeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCanRenameNode = false;
}

void UQuestGraphNodeBase::PostLoad()
{
	Super::PostLoad();
	BindDelegates();
}

void UQuestGraphNodeBase::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		CreateNewGuid();
	}
}

void UQuestGraphNodeBase::PostEditImport()
{
	Super::PostEditImport();
	BindDelegates();
}

void UQuestGraphNodeBase::BindDelegates()
{
	check(GetQuest());
	GetQuest()->OnQuestPropertyChanged.AddUObject(this, &UQuestGraphNodeBase::OnQuestPropertyChanged);
}

void UQuestGraphNodeBase::PostPlacedNewNode() 
{
	BindDelegates();
}

void UQuestGraphNodeBase::AllocateDefaultPins()
{
	CreateInputPin();
	CreateOutputPins();
	ensure(Pins.Num() == 2 || Pins.Num() == 3);
}

void UQuestGraphNodeBase::ReconstructNode()
{
	Modify();
	ClearCompilerMessage();
	ErrorMsg.Reset();

	for (UEdGraphPin* Pin : Pins)
	{
		for (UEdGraphPin* OtherPin : Pin->LinkedTo)
		{
			if (!OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	UEdGraphPin* InputPin = HasInputPin() ? GetInputPin() : nullptr;
	UEdGraphPin* OutputPins[2];
	OutputPins[0] = HasOutputPinA() ? GetOutputPinA() : nullptr;
	OutputPins[1] = HasOutputPinB() ? GetOutputPinB() : nullptr;

	TArray<UEdGraphPin*, TInlineAllocator<4>> RemovePins;
	RemovePins.Append(Pins);
	Pins.Empty();

	AllocateDefaultPins();

	UEdGraphPin* NewInputPin = GetInputPin();
	UEdGraphPin* NewOutputPins[2];
	NewOutputPins[0] = GetOutputPinA();
	NewOutputPins[1] = HasOutputPinB() ? GetOutputPinB() : nullptr;

	if (InputPin)
	{
		NewInputPin->CopyPersistentDataFromOldPin(*InputPin);
	}

	for (int32 OutputPinIndex = 0; OutputPinIndex < 2; OutputPinIndex++)
	{
		if (NewOutputPins[OutputPinIndex] && OutputPins[OutputPinIndex])
		{
			NewOutputPins[OutputPinIndex]->CopyPersistentDataFromOldPin(*(OutputPins[OutputPinIndex]));
		}
	}

	for (UEdGraphPin* Pin : RemovePins)
	{
		Pin->Modify();
		Pin->BreakAllPinLinks();
		DestroyPin(Pin);
	}
}

bool UQuestGraphNodeBase::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema->IsA(UQuestGraphSchema::StaticClass());
}

FSlateIcon UQuestGraphNodeBase::GetIconAndTint(FLinearColor& OutColor) const
{
	static const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Graph.StateNode.Icon");
	OutColor = GetNodeStyleColor();
	return Icon;
}

FText UQuestGraphNodeBase::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return Super::GetNodeTitle(TitleType);
}

bool UQuestGraphNodeBase::HasConnectionToNode(const UEdGraphNode* TargetNode, EEdGraphPinDirection Direction) const
{
	if (Direction == EGPD_Output)
	{
		// Output pin 1 should always exist
		for (UEdGraphPin* OtherInputPin : GetOutputPinA()->LinkedTo)
		{
			if (OtherInputPin->GetOwningNode() == TargetNode)
			{
				return true;
			}
		}

		// Only certain nodes have 2 output pins
		if (HasOutputPinB())
		{
			for (UEdGraphPin* OtherInputPin : GetOutputPinB()->LinkedTo)
			{
				if (OtherInputPin->GetOwningNode() == TargetNode)
				{
					return true;
				}
			}
		}
	}
	else // Check input pin connection to parent output
	{
		for (UEdGraphPin* OtherOutputPin : GetInputPin()->LinkedTo)
		{
			if (OtherOutputPin->GetOwningNode() == TargetNode)
			{
				return true;
			}
		}
	}

	return false;
}


UEdGraphPin* UQuestGraphNodeBase::GetInputPin() const
{
	check(HasInputPin());
	return Pins[QUEST_GRAPH_PIN_INPUT];
}

UEdGraphPin* UQuestGraphNodeBase::GetOutputPinA() const
{
	check(HasOutputPinA());
	return Pins[QUEST_GRAPH_PIN_OUTPUT1];
}

UEdGraphPin* UQuestGraphNodeBase::GetOutputPinB() const
{
	check(HasOutputPinB());
	return Pins[QUEST_GRAPH_PIN_OUTPUT2];
}

bool UQuestGraphNodeBase::HasInputPin() const
{
	return Pins.IsValidIndex(QUEST_GRAPH_PIN_INPUT) && Pins[QUEST_GRAPH_PIN_INPUT] != nullptr && Pins[QUEST_GRAPH_PIN_INPUT]->Direction == EGPD_Input;
}

bool UQuestGraphNodeBase::HasOutputPinA() const
{
	return Pins.IsValidIndex(QUEST_GRAPH_PIN_OUTPUT1) && Pins[QUEST_GRAPH_PIN_OUTPUT1] != nullptr && Pins[QUEST_GRAPH_PIN_OUTPUT1]->Direction == EGPD_Output;
}

bool UQuestGraphNodeBase::HasOutputPinB() const
{
	return Pins.IsValidIndex(QUEST_GRAPH_PIN_OUTPUT2) && Pins[QUEST_GRAPH_PIN_OUTPUT2] != nullptr && Pins[QUEST_GRAPH_PIN_OUTPUT2]->Direction == EGPD_Output;
}

UQuestGraph* UQuestGraphNodeBase::GetQuestGraph() const
{ 
	return CastChecked<UQuestGraph>(GetGraph());
}

UQuest* UQuestGraphNodeBase::GetQuest() const
{
	return GetQuestGraph()->GetQuest();
}

const UQuestGraphSchema* UQuestGraphNodeBase::GetQuestGraphSchema() const
{ 
	return GetQuestGraph()->GetQuestGraphSchema(); 
}

void UQuestGraphNodeBase::SetCompilerWarningMessage(FString Message)
{
	bHasCompilerMessage = true;
	ErrorType = EMessageSeverity::Warning;
	ErrorMsg = Message;
}

void UQuestGraphNodeBase::SetCompilerErrorMessage(FString Message)
{
	bHasCompilerMessage = true;
	ErrorType = EMessageSeverity::Error;
	ErrorMsg = Message;
}

void UQuestGraphNodeBase::ClearCompilerMessage()
{
	bHasCompilerMessage = false;
	ErrorType = EMessageSeverity::Info;
	ErrorMsg.Empty();
}
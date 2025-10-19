// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestGraphNodeBase.h"
#include "Nodes/Slate/SQuestGraphPin.h"

#define LOCTEXT_NAMESPACE "QuestGraphNodeBase"

void SQuestGraphNodeBase::Construct(const FArguments& InArgs, UQuestGraphNodeBase* InNode)
{
	GraphNode = Cast<UEdGraphNode>(InNode);
	QuestGraphNode = InNode;
}

void SQuestGraphNodeBase::CreatePinWidgets()
{
	check(QuestGraphNode);

	UEdGraphPin* Input = QuestGraphNode->GetInputPin();
	UEdGraphPin* Output = QuestGraphNode->GetOutputPinA();

	if (!EnsurePin(Input) || !EnsurePin(Output))
	{
		return;
	}

	if (QuestGraphNode->CanHaveInputConnections())
	{
		CreateStandardPinWidget(Input);
	}

	if (QuestGraphNode->CanHaveOutputConnections())
	{
		CreateStandardPinWidget(Output);

		if (QuestGraphNode->HasOutputPinB())
		{
			CreateStandardPinWidget(QuestGraphNode->GetOutputPinB());
		}
	}
}

void SQuestGraphNodeBase::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	Super::CreateStandardPinWidget(Pin);
}

void SQuestGraphNodeBase::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	InputPinsNodeBox.Reset();
	SAssignNew(InputPinsNodeBox, SHorizontalBox);

	OutputPinsNodeBox.Reset();
	SAssignNew(OutputPinsNodeBox, SHorizontalBox);

	SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &Self::GetVisibility)));
}

void SQuestGraphNodeBase::SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel)
{
	check(!OwnerGraphPanelPtr.IsValid());
	SetParentPanel(OwnerPanel);
	OwnerGraphPanelPtr = OwnerPanel;
}

void SQuestGraphNodeBase::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	
	if (PinToAdd->GetDirection() == EGPD_Output)
	{
		OutputPinsNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.4f)
			.Padding(10.0f, 0, 10.0f, 0)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
	else
	{
		InputPinsNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.4f)
			.Padding(10.0f, 0, 10.0f, 0)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
}

TSharedPtr<SGraphPin> SQuestGraphNodeBase::CreatePinWidget(UEdGraphPin* Pin) const
{
	return SNew(SQuestGraphPin, Pin);
}

bool SQuestGraphNodeBase::EnsurePin(UEdGraphPin* Pin)
{
	const bool bValid = Pin != nullptr && Pin->GetOuter() == GraphNode;
	ensureMsgf(bValid, TEXT("%s has an invalid pin"), *GraphNode->GetPathName());
	return bValid;
}


#undef LOCTEXT_NAMESPACE
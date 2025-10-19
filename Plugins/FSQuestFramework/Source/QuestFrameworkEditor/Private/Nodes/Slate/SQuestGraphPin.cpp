// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestGraphPin.h"
#include "ScopedTransaction.h"
#include "SGraphPanel.h"
#include "SGraphNode.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "QuestEditor/QuestEditorStyles.h"

#define LOCTEXT_NAMESPACE "QuestGraphPin"

void SQuestGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	check(InPin);
	SetCursor(EMouseCursor::Default);
	bShowLabel = true;
	GraphPinObj = InPin;

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &Self::GetPinBorder)
		.BorderBackgroundColor(this, &Self::GetPinColor)
		.OnMouseButtonDown(this, &Self::OnPinMouseDown)
		.Cursor(this, &Self::GetPinCursor)
		.Padding(FMargin(10.f))
	);

	TAttribute<FText> ToolTipAttribute = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &Self::GetTooltipText));
	SetToolTipText(ToolTipAttribute);
}

void SQuestGraphPin::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsHovered() && GraphPinObj && !GraphPinObj->IsPendingKill() && GraphPinObj->GetOuter())
	{
		TSharedPtr<SGraphPanel> OwnerPanelPtr = OwnerNodePtr.Pin()->GetOwnerPanel();
		check(OwnerPanelPtr.IsValid());

		HoverPinSet.Add(GraphPinObj);
		OwnerPanelPtr->AddPinToHoverSet(GraphPinObj);

		if (GraphPinObj->LinkedTo.Num() > 0)
		{
			if (UQuestGraphNodeCommonBase* GraphNode = Cast<UQuestGraphNodeCommonBase>(GraphPinObj->GetOwningNode()))
			{
				if (GraphPinObj->Direction == EGPD_Output)
				{
					for (UQuestGraphNodeCommonBase* ChildNode : GraphNode->GetChildrenNodes())
					{
						HoverPinSet.Add(ChildNode->GetInputPin());
						OwnerPanelPtr->AddPinToHoverSet(ChildNode->GetInputPin());
					}
				}
			}
		}
	}

	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SQuestGraphPin::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);
}

FReply SQuestGraphPin::OnPinMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	bIsMovingLinks = false;
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsEditingEnabled())
	{
		if (!GraphPinObj->bNotConnectable && GetPinObj()->Direction == EEdGraphPinDirection::EGPD_Output)
		{
			TSharedPtr<SGraphNode> ThisOwnerNodeWidget = OwnerNodePtr.Pin();
			if (ensure(ThisOwnerNodeWidget.IsValid()))
			{
				TArray<TSharedRef<SGraphPin>> StartingPins;
				StartingPins.Add(SharedThis(this));
				return FReply::Handled().BeginDragDrop(SpawnPinDragEvent(ThisOwnerNodeWidget->GetOwnerPanel().ToSharedRef(), StartingPins));
			}

			return FReply::Unhandled();
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SQuestGraphPin::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

TSharedRef<SWidget>	SQuestGraphPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

FSlateColor SQuestGraphPin::GetPinColor() const
{
	const FSlateColor& DefaultColor = IsHovered() ? FQuestEditorStyles::InputPinColor_Hover : FQuestEditorStyles::InputPinColor;
	if (GraphPinObj == nullptr)
	{
		return DefaultColor;
	}

	if (GraphPinObj->Direction == EGPD_Output)
	{
		if (UQuestGraphNodeCommonBase* CommonBaseNode = Cast<UQuestGraphNodeCommonBase>(GraphPinObj->GetOwningNodeUnchecked()))
		{
			if (CommonBaseNode->HasOutputPinB())
			{
				return IsHovered() ? FQuestEditorStyles::OutputPinColor_Hover : (CommonBaseNode->GetOutputPinA() == GraphPinObj ? FQuestEditorStyles::OutputPinColor_Completed : FQuestEditorStyles::OutputPinColor_Failed);
			}
			else
			{
				return IsHovered() ? FQuestEditorStyles::OutputPinColor_Hover : FQuestEditorStyles::OutputPinColor;
			}
		}
	}

	// Input -> return default
	return DefaultColor;
}

const FSlateBrush* SQuestGraphPin::GetPinBorder() const
{
	return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
}

#undef LOCTEXT_NAMESPACE
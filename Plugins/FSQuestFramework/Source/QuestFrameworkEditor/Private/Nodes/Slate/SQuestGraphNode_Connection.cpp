// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestGraphNode_Connection.h"

#include "Graph/QuestConnectionDrawingPolicy.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestGraphNode_Root.h"
#include "Nodes/QuestGraphNode_Selector.h"
#include "Nodes/QuestGraphNode_Connection.h"
#include "Nodes/QuestGraphNode_End.h"
#include "Nodes/QuestGraphNode_Stage.h"

#include "Styling/StyleColors.h"

#define LOCTEXT_NAMESPACE "SQuestGraphNode_Connection"

void SQuestGraphNode_Connection::Construct(const FArguments& InArgs, UQuestGraphNode_Connection* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	QuestGraphNode_Connection = InNode;
	UpdateGraphNode();
}

void SQuestGraphNode_Connection::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphPanel> GraphPanel = GetOwnerPanel();
	GraphPanel->AddPinToHoverSet(QuestGraphNode_Connection->GetInputPin());
	Super::OnMouseEnter(MyGeometry, MouseEvent);
}

void SQuestGraphNode_Connection::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphPanel> GraphPanel = GetOwnerPanel();
	GraphPanel->RemovePinFromHoverSet(QuestGraphNode_Connection->GetInputPin());
	Super::OnMouseLeave(MouseEvent);
}

void SQuestGraphNode_Connection::PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const
{
	FGeometry StartGeom;
	FGeometry EndGeom;
	static constexpr int32 NodeIndex = 0;
	static constexpr int32 NumberOfEdges = 1;

	UQuestGraphNodeCommonBase* ParentNode = QuestGraphNode_Connection->GetParentNode();
	UQuestGraphNodeCommonBase* ChildNode = QuestGraphNode_Connection->GetChildNode();
	const TSharedRef<SNode>* pPrevNodeWidget = NodeToWidgetLookup.Find(ParentNode);
	const TSharedRef<SNode>* pNextNodeWidget = NodeToWidgetLookup.Find(ChildNode);
	if (pPrevNodeWidget != nullptr && pNextNodeWidget != nullptr)
	{
		const TSharedRef<SNode>& PrevNodeWidget = *pPrevNodeWidget;
		const TSharedRef<SNode>& NextNodeWidget = *pNextNodeWidget;

		StartGeom = FGeometry(FVector2D(ParentNode->NodePosX, ParentNode->NodePosY), FVector2D::ZeroVector, PrevNodeWidget->GetDesiredSize(), 1.0f);
		EndGeom = FGeometry(FVector2D(ChildNode->NodePosX, ChildNode->NodePosY), FVector2D::ZeroVector, NextNodeWidget->GetDesiredSize(), 1.0f);

	}

	// Position Node
	PositionBetweenTwoNodesWithOffset(StartGeom, EndGeom, NodeIndex, NumberOfEdges);
}

void SQuestGraphNode_Connection::UpdateGraphNode()
{
	Super::UpdateGraphNode();

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.TransitionNode.ColorSpill"))
				.ColorAndOpacity(this, &Self::GetConnectionColor)
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.TransitionNode.Icon"))
			]
		];
}

void SQuestGraphNode_Connection::PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
	check(NodeIndex >= 0);
	check(MaxNodes > 0);

	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) / 2.0f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	// Position ourselves halfway along the connecting line between the nodes, elevated away perpendicular to the direction of the line
	static constexpr float Height = 30.0f;
	const FVector2D DesiredNodeSize = GetDesiredSize();

	FVector2D DeltaPos(EndAnchorPoint - StartAnchorPoint);
	if (DeltaPos.IsNearlyZero())
	{
		DeltaPos = FVector2D(10.0f, 0.0f);
	}

	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();
	const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) + (Height * Normal);
	const FVector2D DeltaNormal = DeltaPos.GetSafeNormal();

	static constexpr float MultiNodeSpace = 0.2f; // Space between multiple edge nodes (in units of <size of node> )
	static constexpr float MultiNodeStep = 1.f + MultiNodeSpace; // Step between node centres (Size of node + size of node spacer)

	const float MultiNodeStart = -((MaxNodes - 1) * MultiNodeStep) / 2.f;
	const float MultiNodeOffset = MultiNodeStart + (NodeIndex * MultiNodeStep);

	// Now we need to adjust the new center by the node size, zoom factor and multi node offset
	const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (DeltaNormal * MultiNodeOffset * DesiredNodeSize.Size());

	GraphNode->NodePosX = NewCorner.X;
	GraphNode->NodePosY = NewCorner.Y;
}

FSlateColor SQuestGraphNode_Connection::GetConnectionColor() const
{
	return IsHovered() ? FQuestEditorStyles::ConnectionColor_Hover : HasConditions() ? FQuestEditorStyles::ConnectionColor_Conditions : FQuestEditorStyles::ConnectionColor;
}

bool SQuestGraphNode_Connection::HasConditions() const
{
	if (QuestGraphNode_Connection)
	{
		return QuestGraphNode_Connection->HasSelectorParent();
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
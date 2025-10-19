// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "Nodes/QuestGraphNode_Connection.h"
#include "SQuestGraphNodeBase.h"

/**
 * Representation of connection between nodes
 */
class SQuestGraphNode_Connection : public SQuestGraphNodeBase
{
	typedef SQuestGraphNodeBase Super;
	typedef SQuestGraphNode_Connection Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestGraphNode_Connection* InNode);

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual bool RequiresSecondPassLayout() const override { return true; }

	virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& InNodeToWidgetLookup) const override;

	virtual void UpdateGraphNode() override;

	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	FSlateColor GetConnectionColor() const;

	bool HasConditions() const;

protected:

	UQuestGraphNode_Connection* QuestGraphNode_Connection = nullptr;
};

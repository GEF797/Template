// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"

#include "Nodes/Slate/SQuestGraphNodeBase.h"
#include "Nodes/QuestGraphNodeCommonBase.h"

class SVerticalBox;

/**
 * Slate representation of common quest graph node
 */
class SQuestGraphNodeCommonBase : public SQuestGraphNodeBase
{
private:

	typedef SQuestGraphNodeBase Super;
	typedef SQuestGraphNodeCommonBase Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestGraphNodeCommonBase* InNode);

	virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;

	virtual void UpdateGraphNode() override;

protected:

	virtual bool UseLowDetailNodeTitles() const override
	{
		if (const SGraphPanel* MyOwnerPanel = GetOwnerPanel().Get())
		{
			return MyOwnerPanel->GetCurrentLOD() <= EGraphRenderingLOD::LowestDetail;
		}

		return false;
	}

	TSharedRef<SWidget> GetNodeBodyWidget();
	TSharedRef<SWidget> GetTitleWidget();
	TSharedRef<SWidget> GetDescriptionWidget();
	virtual TSharedRef<SWidget> GetSubBodyWidget();

	FSlateColor GetCommentColor() const override { return QuestGraphNode->GetNodeStyleColor(); }
	FSlateColor GetNodeBodyColor() const { return QuestGraphNode->GetNodeStyleColor(); }

	FText GetDescription() const;

	EVisibility GetDescriptionVisibility() const
	{
		TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
		return !MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail ? EVisibility::Visible : EVisibility::Collapsed;
	}

	// Index text for overlay widget to show node index
	FText GetIndexText() const { return FText::AsNumber(QuestGraphNode->GetQuestNodeIndex()); }

protected:

	// Actual body of the node which is wrapped inside border
	TSharedPtr<SBorder> NodeBodyWidget;

	// Title widget inside node body
	TSharedPtr<SWidget> TitleWidget;

	// Description widget inside node body
	TSharedPtr<SWidget> DescriptionWidget;

	// Widget to display node index as small bubble overlay at the corner
	TSharedPtr<SWidget> IndexOverlayWidget;

	// Body to contain any sub widgets inside NodeBodyWidget
	TSharedPtr<SWidget> SubBodyWidget;

protected:

	UQuestGraphNodeCommonBase* QuestGraphNode = nullptr;
};

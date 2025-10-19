// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestGraphNodeCommonBase.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SToolTip.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "IDocumentation.h"
#include "GraphEditorDragDropAction.h"
#include "Styling/StyleColors.h"
#include "QuestEditor/QuestEditorStyles.h"
#include "Nodes/Slate/SQuestNodeOverlay.h"

#define LOCTEXT_NAMESPACE "QuestGraphNodeCommonBase"

void SQuestGraphNodeCommonBase::Construct(const FArguments& InArgs, UQuestGraphNodeCommonBase* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	QuestGraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

TArray<FOverlayWidgetInfo> SQuestGraphNodeCommonBase::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	check(IndexOverlayWidget.IsValid());

	TArray<FOverlayWidgetInfo> Widgets;
	FVector2D Origin(0.0f, 0.0f);

	// Index overlay
	{
		FOverlayWidgetInfo Overlay(IndexOverlayWidget);
		Overlay.OverlayOffset = FVector2D(WidgetSize.X - IndexOverlayWidget->GetDesiredSize().X / 2.0f, Origin.Y);
		Widgets.Add(Overlay);
		Origin.Y += IndexOverlayWidget->GetDesiredSize().Y + 1.5f;
	}

	Widgets.Append(Super::GetOverlayWidgets(bSelected, WidgetSize));
	return Widgets;
}


void SQuestGraphNodeCommonBase::UpdateGraphNode()
{
	Super::UpdateGraphNode();
	SetupErrorReporting();

	IndexOverlayWidget = SNew(SQuestNodeOverlay)
		.OverlayBody(
			SNew(STextBlock)
			.Text(this, &Self::GetIndexText)
			.ColorAndOpacity(FLinearColor::Black)
			.Font(FAppStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
		);


	// Set Default tooltip
	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &Super::GetNodeTooltip), nullptr,
			GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
		SetToolTip(DefaultToolTip);
	}

	const FMargin NodePadding = 10.0f;
	{
		this->ContentScale.Bind(this, &Super::GetContentScale);
		this->GetOrAddSlot(ENodeZone::Center)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(0)
				.BorderBackgroundColor(FQuestEditorStyles::CommonNodeBorderColor)
				[
					SNew(SOverlay)

					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SVerticalBox)

						// Input
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.MinDesiredHeight(NodePadding.Top)
							[
								SAssignNew(LeftNodeBox, SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(20.0f, 0.0f)
								.FillHeight(1.0f)
								[
									InputPinsNodeBox.ToSharedRef()
								]
							]
						]

						// Body
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						.Padding(NodePadding)
						[
							SNew(SBox)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.MinDesiredWidth(200.0f)
							[
								GetNodeBodyWidget()
							]
						]

						// Output
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.MinDesiredHeight(NodePadding.Bottom)
							[
								SAssignNew(RightNodeBox, SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								.Padding(20.0f, 0.0f)
								.FillHeight(1.0f)
								[
									OutputPinsNodeBox.ToSharedRef()
								]
							]
						]
					]
				]
			];

		CreatePinWidgets();
	}
}


TSharedRef<SWidget> SQuestGraphNodeCommonBase::GetNodeBodyWidget()
{
	if (NodeBodyWidget.IsValid())
	{
		return NodeBodyWidget.ToSharedRef();
	}

	NodeBodyWidget =
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
		.BorderBackgroundColor(this, &Self::GetNodeBodyColor)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		.Visibility(EVisibility::Visible)
		[
			// Main
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				GetTitleWidget()
			]

			// Description
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				GetDescriptionWidget()
			]

			// Sub body
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				GetSubBodyWidget()
			]
		];

	return NodeBodyWidget.ToSharedRef();
}

TSharedRef<SWidget> SQuestGraphNodeCommonBase::GetTitleWidget()
{
	if (TitleWidget.IsValid())
	{
		return TitleWidget.ToSharedRef();
	}

	// Title
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
		return FMath::Max(75.0f, DesiredWidth);
	};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
		return FMath::Max(22.0f, DesiredHeight);
	};

	// Icon + tint
	IconColor = FLinearColor::White;
	const FSlateBrush* IconBrush = nullptr;
	if (GraphNode->ShowPaletteIconOnNode())
	{
		IconBrush = GraphNode->GetIconAndTint(IconColor).GetIcon();
	}

	TitleWidget = SNew(SHorizontalBox)
		// Error message + title
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SNew(SHorizontalBox)

			// Error message
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				// POPUP ERROR MESSAGE
				ErrorReporting->AsWidget()
			]

			// Title
			+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SLevelOfDetailBranchNode)
					.UseLowDetailSlot(this, &Self::UseLowDetailNodeTitles)
				.LowDetail()
				[
					SNew(SBox)
					.WidthOverride_Lambda(GetNodeTitlePlaceholderWidth)
				.HeightOverride_Lambda(GetNodeTitlePlaceholderHeight)
				]
				.HighDetail()
				[
					SNew(SHorizontalBox)
					// Icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage)
						.Image(IconBrush)
						.ColorAndOpacity(this, &Super::GetNodeTitleIconColor)
					]

					// Title content
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SVerticalBox)

						// Text element
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.TextStyle(FAppStyle::Get(), "Graph.StateNode.NodeTitle")
							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
						]

						+SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitle.ToSharedRef()
						]
					]
				]
			]
		];

	return TitleWidget.ToSharedRef();
}

TSharedRef<SWidget> SQuestGraphNodeCommonBase::GetDescriptionWidget()
{
	if (DescriptionWidget.IsValid())
	{
		return DescriptionWidget.ToSharedRef();
	}

	{
		const float fWrapAt = 100.f;
		const float fMargin = 5.0f;

		DescriptionWidget = SNew(STextBlock)
			.Visibility(this, &Self::GetDescriptionVisibility)
			.Text(this, &Self::GetDescription)
			.WrapTextAt(fWrapAt)
			.Margin(fMargin);
	}

	return DescriptionWidget.ToSharedRef();
}

TSharedRef<SWidget> SQuestGraphNodeCommonBase::GetSubBodyWidget()
{
	if (SubBodyWidget.IsValid())
	{
		return SubBodyWidget.ToSharedRef();
	}

	{
		// Add dummy collapsed box by default
		SubBodyWidget = SNew(SBox)
		.Visibility(EVisibility::Collapsed);
	}

	return SubBodyWidget.ToSharedRef();
}

FText SQuestGraphNodeCommonBase::GetDescription() const
{
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE

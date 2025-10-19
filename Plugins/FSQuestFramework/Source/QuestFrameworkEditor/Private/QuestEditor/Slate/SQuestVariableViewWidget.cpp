// Copyright Sami Kangasmaa, 2022


#include "QuestEditor/Slate/SQuestVariableViewWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "QuestEditor/QuestEditorStyles.h"

void SQuestVariableViewEntryWidget::Construct(const FArguments& InArgs)
{
	VariableInfo = InArgs._QuestVariableInfo;

	const float TextMargin = 4.0f;
	const float DetailTextMargin = 2.0f;

	// Get Name of the variable
	const FText NameText = FText::FromName(VariableInfo.QuestVariable.Variable);
	
	// Get type of the variable and determine color and type text
	FText TypeText = FText::FromString(TEXT("Int"));
	FLinearColor BackgroundColor = FQuestEditorStyles::VariableLabelColor_Int;
	if (VariableInfo.QuestVariable.Type == EQuestVariableType::Bool)
	{
		BackgroundColor = FQuestEditorStyles::VariableLabelColor_Bool;
		TypeText = FText::FromString(TEXT("Bool"));
	}
	else if (VariableInfo.QuestVariable.Type == EQuestVariableType::Float)
	{
		BackgroundColor = FQuestEditorStyles::VariableLabelColor_Float;
		TypeText = FText::FromString(TEXT("Float"));
	}

	// Get additional details
	const FText VariableCreationTypeText = FText::FromString(FString::Printf(TEXT("Creation type: %s"), *(VariableInfo.bIsDefaultVariable ? FString("Default") : FString("Dynamic"))));
	const FText ReplicationTypeText = FText::FromString(FString::Printf(TEXT("Replicates: %s"), *(VariableInfo.bReplicates ? FString("Yes") : FString("No"))));

	FString InitialValueStr;
	switch (VariableInfo.QuestVariable.Type)
	{
	case EQuestVariableType::Bool:
		InitialValueStr = VariableInfo.DefaultBoolValue ? FString("True") : FString("False");
		break;
	case EQuestVariableType::Float:
		InitialValueStr = FString::SanitizeFloat(VariableInfo.DefaultFloatValue, 2);
		break;
	case EQuestVariableType::Integer:
	default:
		InitialValueStr = FString::FromInt(VariableInfo.DefaultIntegerValue);
		break;
	}
	const FText InitialValueText = FText::FromString(FString::Printf(TEXT("Initial value: %s"), *InitialValueStr));

	FTextBlockStyle TextBlockStyle = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
	TextBlockStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));

	FTextBlockStyle DetailTextBlockStyle = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
	DetailTextBlockStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 8));

	ChildSlot
		[
			SNew(SBorder)
			.BorderBackgroundColor(BackgroundColor)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot() // Name and type in upper section
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Fill)
					[
						SNew(STextBlock)
						.Margin(TextMargin)
						.Text(NameText)
						.TextStyle(&TextBlockStyle)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Fill)
					[
						SNew(STextBlock)
						.Margin(TextMargin)
						.Text(TypeText)
						.TextStyle(&TextBlockStyle)
					]
				]
				+ SVerticalBox::Slot() // Details in bottom section
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					[
						SNew(STextBlock)
						.Margin(DetailTextMargin)
						.Text(VariableCreationTypeText)
						.TextStyle(&DetailTextBlockStyle)
					]
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					[
						SNew(STextBlock)
						.Margin(DetailTextMargin)
						.Text(InitialValueText)
						.TextStyle(&DetailTextBlockStyle)
					]
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					[
						SNew(STextBlock)
						.Margin(DetailTextMargin)
						.Text(ReplicationTypeText)
						.TextStyle(&DetailTextBlockStyle)
					]
				]
			]
		];

	// Create attribute for tooltip and bind function to get description of the variable as the tooltip
	TAttribute<FText> ToolTipAttribute = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &Self::GetTooltip));
	SetToolTipText(ToolTipAttribute);
}

FText SQuestVariableViewEntryWidget::GetTooltip() const
{
	return VariableInfo.Description;
}

void SQuestVariableViewWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		GetVariableWidgets()
	];
}

void SQuestVariableViewWidget::UpdateVariableList(const TArray<FQuestVariableInfo>& InVariableInfo)
{
	// Update variable info array
	VariableInfoData = InVariableInfo;

	// Reset the widgets to make them dirty
	VariableBox.Reset();
	VariableVerticalBox.Reset();
	VariableWidgets.Empty();

	// Recreate listing by getting new variable widgets
	ChildSlot
		[
			GetVariableWidgets()
		];
}

TSharedRef<SScrollBox> SQuestVariableViewWidget::GetVariableWidgets()
{
	if (VariableBox.IsValid())
	{
		return VariableBox.ToSharedRef();
	}

	// Create verticla box to add variable widgets
	VariableVerticalBox = SNew(SVerticalBox)
	.Visibility(EVisibility::Visible);

	const FText TitleText = FText::FromString(TEXT("Variables"));
	VariableVerticalBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
		[
			SNew(STextBlock)
			.Margin(5.0f)
			.Text(TitleText)
			.TextStyle(FAppStyle::Get(), "Graph.StateNode.NodeTitle")
		];

	// Create scroll box
	VariableBox = SNew(SScrollBox)
	.Visibility(EVisibility::Visible);

	// Add vertical box to scroll box
	VariableBox->AddSlot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			VariableVerticalBox.ToSharedRef()
		];

	for (const FQuestVariableInfo& VarInfo : VariableInfoData)
	{
		AddEntry(VarInfo);
	}

	return VariableBox.ToSharedRef();
}

void SQuestVariableViewWidget::AddEntry(const FQuestVariableInfo& VariableInfo)
{
	check(VariableVerticalBox.IsValid());

	const FMargin Padding = FMargin(2.0f, 2.0f, 2.0f, 2.0f);
	VariableVerticalBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(Padding)
		[
			SNew(SQuestVariableViewEntryWidget)
			.QuestVariableInfo(VariableInfo)
		];
}

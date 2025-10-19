// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestGraphNode_Stage.h"
#include "Nodes/QuestGraphNode_Stage.h"
#include "Nodes/QuestNode_Stage.h"

#include "QuestEditor/QuestEditorStyles.h"

void SQuestGraphNode_Stage::Construct(const FArguments& InArgs, UQuestGraphNode_Stage* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
}

void SQuestGraphNode_Stage::UpdateGraphNode()
{
	SubBodyWidget.Reset();
	ObjectiveBox.Reset();
	ObjectiveWidgets.Empty();

	Super::UpdateGraphNode();
}

TSharedRef<SWidget> SQuestGraphNode_Stage::GetSubBodyWidget()
{
	if (SubBodyWidget.IsValid())
	{
		return SubBodyWidget.ToSharedRef();
	}

	{

		SubBodyWidget = SNew(SVerticalBox)
		.Visibility(EVisibility::Visible)
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				GetObjectiveWidgets()
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				GetServiceWidgets()
			];
	}

	return SubBodyWidget.ToSharedRef();
}

TSharedRef<SVerticalBox> SQuestGraphNode_Stage::GetObjectiveWidgets()
{
	if (ObjectiveBox.IsValid())
	{
		return ObjectiveBox.ToSharedRef();
	}

	TArray<UQuestObjective*> Objectives;
	if (UQuestNode_Stage* Node = QuestGraphNode->GetQuestNodeUnchecked<UQuestNode_Stage>())
	{
		Objectives = Node->GetObjectives();
	}

	ObjectiveBox = SNew(SVerticalBox)
	.Visibility(EVisibility::Visible);

	if (Objectives.Num() > 0)
	{
		const FText ObjectivesLabelText = FText::FromString(TEXT("Objectives:"));
		ObjectiveBox->AddSlot()
			.AutoHeight()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(2.0f, 5.0f, 2.0f, 5.0f))
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
				.BorderBackgroundColor(this, &Self::GetNodeBodyColor)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Padding(1.0f)
				.Visibility(EVisibility::Visible)
				[
					SNew(STextBlock)
					.Margin(5.0f)
					.Text(ObjectivesLabelText)
					.TextStyle(FAppStyle::Get(), "Graph.StateNode.NodeTitle")
				]
			];
	}

	for (const UQuestObjective* Objective : Objectives)
	{
		if (!Objective)
		{
			continue;
		}
		AddObjective(Objective);
	}

	return ObjectiveBox.ToSharedRef();
}

void SQuestGraphNode_Stage::AddObjective(const UQuestObjective* Objective)
{
	check(ObjectiveBox.IsValid());
	check(Objective);

	const FMargin NodePadding = FMargin(2.0f, 5.0f, 2.0f, 5.0f);
	const float fMargin = 5.0f;
	FText ObjectiveNameText = FText::FromName(Objective->LogicalName);

	static TMap<EQuestObjectiveSignificancy, FLinearColor> Colors;
	if (Colors.Num() <= 0)
	{
		Colors.Add(EQuestObjectiveSignificancy::Regular, FQuestEditorStyles::NodeBodyColor_Objective);
		Colors.Add(EQuestObjectiveSignificancy::Optional, FQuestEditorStyles::NodeBodyColor_Objective_Optional);
		Colors.Add(EQuestObjectiveSignificancy::RequiredToComplete, FQuestEditorStyles::NodeBodyColor_Objective_RequireComplete);
		Colors.Add(EQuestObjectiveSignificancy::ForceComplete, FQuestEditorStyles::NodeBodyColor_Objective_ForceComplete);
		Colors.Add(EQuestObjectiveSignificancy::ForceFailure, FQuestEditorStyles::NodeBodyColor_Objective_ForceFail);
		Colors.Add(EQuestObjectiveSignificancy::ForceCompleteAndFailure, FQuestEditorStyles::NodeBodyColor_Objective_ForceCompleteFail);
	}

	FLinearColor* ColorPtr = Colors.Find(Objective->Significancy);
	const FLinearColor BackgroundColor = ColorPtr != nullptr ? *ColorPtr : FQuestEditorStyles::NodeBodyColor_Objective;

	ObjectiveBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(NodePadding)
		[
			SNew(SBorder)
			.BorderBackgroundColor(BackgroundColor)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.VAlign(VAlign_Fill)
			[
				SNew(STextBlock)
				.Margin(fMargin)
				.Text(ObjectiveNameText)
			]
		];
}

TSharedRef<SVerticalBox> SQuestGraphNode_Stage::GetServiceWidgets()
{
	if (ServiceBox.IsValid())
	{
		return ServiceBox.ToSharedRef();
	}

	TArray<UQuestService*> Services;
	if (UQuestNode_Stage* Node = QuestGraphNode->GetQuestNodeUnchecked<UQuestNode_Stage>())
	{
		Services = Node->GetServices();
	}

	ServiceBox = SNew(SVerticalBox)
		.Visibility(EVisibility::Visible);
	
	if (Services.Num() > 0)
	{
		const FText ServiceLabelText = FText::FromString(TEXT("Services:"));
		ServiceBox->AddSlot()
			.AutoHeight()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(2.0f, 5.0f, 2.0f, 5.0f))
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
				.BorderBackgroundColor(this, &Self::GetNodeBodyColor)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Padding(1.0f)
				.Visibility(EVisibility::Visible)
				[
					SNew(STextBlock)
					.Margin(5.0f)
					.Text(ServiceLabelText)
					.TextStyle(FAppStyle::Get(), "Graph.StateNode.NodeTitle")
				]
			];
	}

	for (const UQuestService* Service : Services)
	{
		if (!Service)
		{
			continue;
		}
		AddService(Service);
	}

	return ServiceBox.ToSharedRef();
}

void SQuestGraphNode_Stage::AddService(const UQuestService* Service)
{
	check(ServiceBox.IsValid());
	check(Service);

	const FMargin NodePadding = FMargin(2.0f, 5.0f, 2.0f, 5.0f);
	const float fMargin = 5.0f;
	FText ServiceNameText = Service->GetClass()->GetDisplayNameText();

	const FLinearColor BackgroundColor = FQuestEditorStyles::NodeBodyColor_Service;

	ServiceBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(NodePadding)
		[
			SNew(SBorder)
			.BorderBackgroundColor(BackgroundColor)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.VAlign(VAlign_Fill)
			[
				SNew(STextBlock)
				.Margin(fMargin)
				.Text(ServiceNameText)
			]
		];
}
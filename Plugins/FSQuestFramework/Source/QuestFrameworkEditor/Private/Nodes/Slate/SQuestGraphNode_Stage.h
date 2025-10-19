// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Nodes/Slate/SQuestGraphNodeCommonBase.h"

#include "Nodes/QuestNode_Stage.h"
#include "Nodes/QuestGraphNode_Stage.h"


class SQuestGraphNode_Stage : public SQuestGraphNodeCommonBase
{
private:

	typedef SQuestGraphNodeCommonBase Super;
	typedef SQuestGraphNode_Stage Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestGraphNode_Stage* InNode);

	virtual void UpdateGraphNode() override;

protected:

	virtual TSharedRef<SWidget> GetSubBodyWidget() override;

	TSharedRef<SVerticalBox> GetObjectiveWidgets();
	void AddObjective(const UQuestObjective* Objective);

	TSharedPtr<SVerticalBox> ObjectiveBox;
	TArray<TSharedPtr<SWidget>> ObjectiveWidgets;

	TSharedRef<SVerticalBox> GetServiceWidgets();
	void AddService(const UQuestService* Service);

	TSharedPtr<SVerticalBox> ServiceBox;
	TArray<TSharedPtr<SWidget>> ServiceWidgets;
};

// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"

#include "QuestMemory.h"

/**
 * Slate widget to represent single quest variable entry in variable view
 */
class SQuestVariableViewEntryWidget : public SCompoundWidget
{
private:
	typedef SCompoundWidget Super;
	typedef SQuestVariableViewEntryWidget Self;

public:

	SLATE_BEGIN_ARGS(SQuestVariableViewEntryWidget) {}
	SLATE_ARGUMENT(FQuestVariableInfo, QuestVariableInfo)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	FQuestVariableInfo VariableInfo;
	FText GetTooltip() const;
};

/**
 * Slate widget to show quest variables in a tab
 */
class SQuestVariableViewWidget : public SCompoundWidget
{
private:
	typedef SCompoundWidget Super;
	typedef SQuestVariableViewWidget Self;

public:

	SLATE_BEGIN_ARGS(SQuestVariableViewWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateVariableList(const TArray<FQuestVariableInfo>& InVariableInfo);

private:

	TSharedRef<SScrollBox> GetVariableWidgets();
	void AddEntry(const FQuestVariableInfo& VariableInfo);

	TSharedPtr<SScrollBox> VariableBox;
	TSharedPtr<SVerticalBox> VariableVerticalBox;
	TArray<TSharedPtr<SWidget>> VariableWidgets;

	TArray<FQuestVariableInfo> VariableInfoData;
};

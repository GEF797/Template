// Copyright Sami Kangasmaa, 2022


#include "ViewModels/QuestObjectiveViewModel.h"

UQuestObjectiveViewModel::UQuestObjectiveViewModel()
{
	Text = FText::GetEmpty();
	ObjectiveState = EQuestResult::NotStarted;
	IsOptional = false;
}

void UQuestObjectiveViewModel::SetText(const FText& InText)
{
	if (!Text.IdenticalTo(InText))
	{
		Text = InText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Text);
	}
}

void UQuestObjectiveViewModel::SetObjectiveState(EQuestResult NewState)
{
	if (ObjectiveState != NewState)
	{
		UE_MVVM_SET_PROPERTY_VALUE(ObjectiveState, NewState);
	}
}

void UQuestObjectiveViewModel::SetIsOptional(bool bNewOptional)
{
	if (IsOptional != bNewOptional)
	{
		UE_MVVM_SET_PROPERTY_VALUE(IsOptional, bNewOptional);
	}
}
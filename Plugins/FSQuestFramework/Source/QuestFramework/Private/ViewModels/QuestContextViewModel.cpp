// Copyright Sami Kangasmaa, 2022


#include "ViewModels/QuestContextViewModel.h"

#include "QuestContext.h"
#include "QuestObjects/QuestObjective.h"
#include "ViewModels/QuestObjectiveViewModel.h"
#include "Components/QuestTrackerComponent.h"
#include "QuestUtilityLibrary.h"

#include "GameFramework/PlayerController.h"


UQuestContextViewModel::UQuestContextViewModel()
{
	ObjectiveViewModels.Empty();

	Title = FText::GetEmpty();
	JournalText = FText::GetEmpty();
	QuestState = EQuestResult::NotStarted;
	IsTracked = false;
}

void UQuestContextViewModel::SetQuestContext(UQuestContext* Context)
{
	MyQuestContext = Context;
}

UQuestContext* UQuestContextViewModel::GetQuestContext() const
{
	return MyQuestContext.IsValid() ? MyQuestContext.Get() : nullptr;
}

void UQuestContextViewModel::SetActiveObjectives(const UQuestContext* Context, const TArray<UQuestObjective*>& Objectives)
{
	if (Context)
	{
		TArray<TObjectPtr<UQuestObjectiveViewModel>, TInlineAllocator<8>> Old;
		Old.Append<TObjectPtr<UQuestObjectiveViewModel>, FDefaultAllocator>(ObjectiveViewModels);
		ObjectiveViewModels.Empty(Objectives.Num());

		for (UQuestObjective* Objective : Objectives)
		{
			if (UQuestObjectiveViewModel* VM = Context->GetObjectiveViewModel(Objective))
			{
				ObjectiveViewModels.Add(VM);
			}
		}

		bool bChanged = ObjectiveViewModels.Num() != Old.Num();
		if (!bChanged)
		{
			check(ObjectiveViewModels.Num() == Old.Num());
			for (int32 Idx = 0; Idx < ObjectiveViewModels.Num(); Idx++)
			{
				if (ObjectiveViewModels[Idx] != Old[Idx])
				{
					bChanged = true;
					break;
				}
			}
		}
		if (bChanged)
		{
			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetActiveObjectives);
		}
	}
}

void UQuestContextViewModel::Clear()
{
	
	//for (auto ContextViewModel : ObjectiveViewModels)
	//{
	//	ContextViewModel=nullptr;
	//}

	//ObjectiveViewModels.Empty();
	//MyQuestContext=nullptr;
	
}

void UQuestContextViewModel::SetTitle(const FText& InText)
{
	if (!Title.IdenticalTo(InText))
	{
		Title = InText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Title);
	}
}

void UQuestContextViewModel::SetJournalText(const FText& InText)
{
	if (!JournalText.IdenticalTo(InText))
	{
		JournalText = InText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(JournalText);
	}
}

void UQuestContextViewModel::SetQuestState(EQuestResult NewState)
{
	UE_MVVM_SET_PROPERTY_VALUE(QuestState, NewState);
}

void UQuestContextViewModel::SetIsTracked(bool bNewTracked)
{
	UE_MVVM_SET_PROPERTY_VALUE(IsTracked, bNewTracked);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTrackedVisibility);
}

void UQuestContextViewModel::TrackQuest(bool bTrack)
{
	if (IsTracked != bTrack)
	{
		// Ask tracker component to track the quest
		if (MyQuestContext.IsValid())
		{
			if (UQuestTrackerComponent* Tracker = UQuestUtilityLibrary::GetPlayerQuestTrackerComponent(GetTypedOuter<APlayerController>()))
			{
				if (bTrack)
				{
					Tracker->TrackQuest(MyQuestContext.Get());
				}
				else
				{
					Tracker->StopTrackQuest(MyQuestContext.Get());
				}
				// Component should call SetIsTracked, if the track/untrack succeed
			}
		}
	}
}
// Copyright Sami Kangasmaa, 2022


#include "ViewModels/QuestListViewModel.h"
#include "ViewModels/QuestContextViewModel.h"
#include "QuestContext.h"

UQuestListViewModel::UQuestListViewModel()
{
	ContextViewModels.Empty();
}

void UQuestListViewModel::SetQuestContexts(const TArray<UQuestContext*>& Contexts, bool bForceRefresh)
{
	TArray<TObjectPtr<UQuestContextViewModel>, TInlineAllocator<32>> Old;
	Old.Append<TObjectPtr<UQuestContextViewModel>, FDefaultAllocator>(ContextViewModels);
	ContextViewModels.Empty(Contexts.Num());

	for (const UQuestContext* Context : Contexts)
	{
		if (!Context)
		{
			continue;
		}
		if (UQuestContextViewModel* VM = Context->GetViewModel())
		{
			ContextViewModels.Add(VM);
		}
	}

	// Sort in enum order of quest result (InProgress -> Completed -> Failed)
	ContextViewModels.Sort([](const UQuestContextViewModel& A, const UQuestContextViewModel& B)
		{
			return (int32)A.GetQuestState() < (int32)B.GetQuestState();
		});

	bool bChanged = ContextViewModels.Num() != Old.Num() || bForceRefresh;
	if (!bChanged)
	{
		check(ContextViewModels.Num() == Old.Num());
		for (int32 Idx = 0; Idx < ContextViewModels.Num(); Idx++)
		{
			if (ContextViewModels[Idx] != Old[Idx])
			{
				bChanged = true;
				break;
			}
		}
	}
	if (bChanged)
	{
		// Pick first context for more detailed view by default
		if (!IsValid(DetailContextViewModel))
		{
			if (ContextViewModels.Num() > 0)
			{
				DetailContextViewModel = ContextViewModels[0];
			}
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQuestContexts);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInProgressQuestContexts);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCompletedQuestContexts);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetFailedQuestContexts);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDetailQuestContextViewModel);
	}

	OnQuestContextsSet();
}

namespace QuestListViewModelHelpers
{
	void FilterViewModelsByState(EQuestResult InState, const TArray<TObjectPtr<UQuestContextViewModel>>& InViewModels, TArray<TObjectPtr<UQuestContextViewModel>>& OutViewModels)
	{
		OutViewModels.Reserve(InViewModels.Num());
		for (TObjectPtr<UQuestContextViewModel> VM : InViewModels)
		{
			if (VM != nullptr && VM->GetQuestContext() != nullptr && VM->GetQuestContext()->GetQuestState() == InState)
			{
				OutViewModels.Add(VM);
			}
		}
	}
}

TArray<UQuestContextViewModel*> UQuestListViewModel::GetQuestContexts() const
{
	TArray<TObjectPtr<UQuestContextViewModel>> ViewModelList;
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::InProgress, ContextViewModels, ViewModelList);
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::Completed, ContextViewModels, ViewModelList);
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::Failed, ContextViewModels, ViewModelList);
	return ViewModelList;
}

TArray<UQuestContextViewModel*> UQuestListViewModel::GetInProgressQuestContexts() const
{
	TArray<TObjectPtr<UQuestContextViewModel>> ViewModelList;
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::InProgress, ContextViewModels, ViewModelList);
	return ViewModelList;
}

TArray<UQuestContextViewModel*> UQuestListViewModel::GetCompletedQuestContexts() const
{
	TArray<TObjectPtr<UQuestContextViewModel>> ViewModelList;
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::Completed, ContextViewModels, ViewModelList);
	return ViewModelList;
}

TArray<UQuestContextViewModel*> UQuestListViewModel::GetFailedQuestContexts() const
{
	TArray<TObjectPtr<UQuestContextViewModel>> ViewModelList;
	QuestListViewModelHelpers::FilterViewModelsByState(EQuestResult::Failed, ContextViewModels, ViewModelList);
	return ViewModelList;
}

void UQuestListViewModel::SetDetailQuestContextViewModel(UQuestContextViewModel* ViewModel)
{
	if (DetailContextViewModel != ViewModel)
	{
		DetailContextViewModel = ViewModel;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDetailQuestContextViewModel);
	}
}

void UQuestListViewModel::Clear()
{
	for (auto ContextViewModel : ContextViewModels)
	{
		if (ContextViewModel)
		{
			ContextViewModel->Clear();
			ContextViewModel = nullptr;
		}

	}
	ContextViewModels.Empty();
	DetailContextViewModel->Clear();
	DetailContextViewModel = nullptr;
}

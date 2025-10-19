// Copyright Sami Kangasmaa, 2022


#include "ViewModels/QuestTrackerViewModel.h"
#include "ViewModels/QuestContextViewModel.h"
#include "QuestContext.h"

UQuestTrackerViewModel::UQuestTrackerViewModel()
{
	ContextViewModels.Empty();
}

void UQuestTrackerViewModel::SetTrackedQuestContexts(const TArray<UQuestContext*>& Contexts)
{
	TArray<TObjectPtr<UQuestContextViewModel>, TInlineAllocator<8>> Old;
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

	bool bChanged = ContextViewModels.Num() != Old.Num();
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
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTrackedQuestContexts);
	}
}

void UQuestTrackerViewModel::Clear()
{
  for(auto ContextViewModel:ContextViewModels)
  {
	  ContextViewModel->Clear();
	  ContextViewModel=nullptr;
  }
  ContextViewModels.Empty();
}

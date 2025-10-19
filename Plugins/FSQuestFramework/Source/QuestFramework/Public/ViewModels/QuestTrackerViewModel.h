// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"

#include "QuestData.h"

#include "QuestTrackerViewModel.generated.h"

class UQuestContext;
class UQuestContextViewModel;

/**
 * View model to represent tracked quests in UI
 */
UCLASS(BlueprintType)
class QUESTFRAMEWORK_API UQuestTrackerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:

	UQuestTrackerViewModel();

	// Returns array of view models of quest contexts that should be listed in quest tracker UI
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest Tracker View Model")
	TArray<UQuestContextViewModel*> GetTrackedQuestContexts() const
	{
		return ContextViewModels;
	}

	// Sets list of tracked quest contexts. Updates ContextViewModels array.
	void SetTrackedQuestContexts(const TArray<UQuestContext*>& Contexts);


	UFUNCTION(BlueprintCallable, Category = "Quest Tracker View Model")
	void Clear();


protected:

	// View models of quest contexts that are currently being tracked (shown in UI)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestContextViewModel>> ContextViewModels;
};

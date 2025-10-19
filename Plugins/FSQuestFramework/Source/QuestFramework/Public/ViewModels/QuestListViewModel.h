// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "QuestData.h"
#include "QuestListViewModel.generated.h"

class UQuestContext;
class UQuestContextViewModel;

/**
 * View model to fetch quest listing from quest tracker component to UI.
 * Use this to build journal/quest log etc.
 */
UCLASS(BlueprintType)
class QUESTFRAMEWORK_API UQuestListViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UQuestListViewModel();

	// Returns array of view models of quest contexts that should be listed in quest log / journal
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest List View Model")
	TArray<UQuestContextViewModel*> GetQuestContexts() const;

	// Returns array of view models of quest contexts that are in progress
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest List View Model")
	TArray<UQuestContextViewModel*> GetInProgressQuestContexts() const;

	// Returns array of view models of quest contexts that are completed
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest List View Model")
	TArray<UQuestContextViewModel*> GetCompletedQuestContexts() const;

	// Returns array of view models of quest contexts that are failed
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest List View Model")
	TArray<UQuestContextViewModel*> GetFailedQuestContexts() const;

	// Returns view model to use to display detailed information about an quest
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest List View Model")
	UQuestContextViewModel* GetDetailQuestContextViewModel() const { return DetailContextViewModel; }

public:

	// Sets list of tracked quest contexts. Updates ContextViewModels array.
	void SetQuestContexts(const TArray<UQuestContext*>& Contexts, bool bForceRefresh = false);

	// Sets view model to use to preview quest details
	UFUNCTION(BlueprintCallable, Category = "Quest List View Model")
	void SetDetailQuestContextViewModel(UQuestContextViewModel* ViewModel);

	UFUNCTION(BlueprintCallable, Category = "Quest List View Model")
	void Clear();

protected:

	// Override to implement custom behavior when the quest contexts are set
	virtual void OnQuestContextsSet() {}
	
protected:

	// View models of quest contexts of the player
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestContextViewModel>> ContextViewModels;

	// View model of quest context that is currently selected to be viewed more detailed
	UPROPERTY(Transient)
	TObjectPtr<UQuestContextViewModel> DetailContextViewModel;
};

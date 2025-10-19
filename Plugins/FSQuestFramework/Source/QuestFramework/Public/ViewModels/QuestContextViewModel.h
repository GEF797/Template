// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "QuestData.h"

#include "Components/SlateWrapperTypes.h"

#include "QuestContextViewModel.generated.h"

class UQuestContext;
class UQuestObjective;
class UQuestObjectiveViewModel;

/**
 * View model to represent quest context in UI
 */
UCLASS(BlueprintType)
class QUESTFRAMEWORK_API UQuestContextViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UQuestContextViewModel();

	void SetQuestContext(UQuestContext* Context);
	UQuestContext* GetQuestContext() const;

	// Override to implement custom update logic in derived classes. Called per frame by quest context.
	virtual void CustomUpdateViewModel(UQuestContext* Context) {}
	
public:

	// Title of the quest. (DisplayName of the QuestAsset)
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Context View Model")
	FText Title;

	// Text shown in journal / quest briefing (NodeText of the active quest node)
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Context View Model")
	FText JournalText;
	
	// Current state of the quest
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Context View Model")
	EQuestResult QuestState;

	// Is this quest currently tracked?
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Context View Model")
	bool IsTracked;

	// Should any indication that indicates that the quest is tracked be visible or hidden?
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest Context View Model")
	ESlateVisibility GetTrackedVisibility() const { return IsTracked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden; }

	// Returns array of objective view models of objectives that should be listed in UI
	UFUNCTION(BlueprintPure, FieldNotify, Category = "Quest Context View Model")
	TArray<UQuestObjectiveViewModel*> GetActiveObjectives() const
	{
		return ObjectiveViewModels;
	}

	/**
	 * Asks to track / untrack the quest
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Context View Model")
	void TrackQuest(bool bTrack);

public:

	FText GetTitle() const { return Title; }
	void SetTitle(const FText& InText);

	FText GetJournalText() const { return JournalText; }
	void SetJournalText(const FText& InText);

	EQuestResult GetQuestState() const { return QuestState; }
	void SetQuestState(EQuestResult NewState);

	bool GetIsTracked() const { return IsTracked; }
	void SetIsTracked(bool bNewTracked);

	// Sets list of active objectives. Updates ObjectiveViewModels array.
	void SetActiveObjectives(const UQuestContext* Context, const TArray<UQuestObjective*>& Objectives);

	UFUNCTION(BlueprintCallable, Category = "Quest Context View Model")
	void Clear();

private:

	// View models of objectives that are active currently (shown in UI)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestObjectiveViewModel>> ObjectiveViewModels;

	// Pointer to quest context that this view model represents
	TWeakObjectPtr<UQuestContext> MyQuestContext;
};

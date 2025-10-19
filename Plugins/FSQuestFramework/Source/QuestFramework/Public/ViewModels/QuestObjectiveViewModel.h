// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"

#include "QuestData.h"

#include "QuestObjectiveViewModel.generated.h"

/**
 * View model to represent quest objective in UI
 */
UCLASS(BlueprintType)
class QUESTFRAMEWORK_API UQuestObjectiveViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UQuestObjectiveViewModel();

	// Override to implement custom update logic in derived classes. Called per frame by quest context when the objective is active
	virtual void CustomUpdateViewModel(class UQuestContext* Context, class UQuestObjective* Objective) {}

public:

	// Short description of the objective.
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Objective View Model")
	FText Text;

	// State of the objective
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Objective View Model")
	EQuestResult ObjectiveState;

	// Is the objective optional?
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Quest Objective View Model")
	bool IsOptional;

public:

	FText GetText() const { return Text; }
	void SetText(const FText& InText);

	EQuestResult GetObjectiveState() const { return ObjectiveState; }
	void SetObjectiveState(EQuestResult NewState);

	bool GetIsOptional() const { return IsOptional; }
	void SetIsOptional(bool bNewOptional);
};

// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestData.h"
#include "QuestMemory.h"
#include "QuestObjectiveObserver.generated.h"

class UQuestContext;

/**
 * Observer to determine if an objective is InProgress, Completed or Failed.
 * To use custom observers, derive blueprint from this to implement the custom logic.
 * By default, ObjectiveObserver_VariableQuery is used. 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestObjectiveObserver : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	EQuestResult NotifyEvaluateResult(const UQuestContext* Context) const;

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Objective")
	EQuestResult Evaluate(UQuestContext* Context) const;
	virtual EQuestResult Evaluate_Implementation(UQuestContext* Context) const;
};

/**
 * Quest objective observer that completes or fails the objective if certain variable queries match
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew)
class QUESTFRAMEWORK_API UObjectiveObserver_VariableQuery : public UQuestObjectiveObserver
{
	GENERATED_BODY()

public:

	virtual EQuestResult Evaluate_Implementation(UQuestContext* Context) const override;

	/**
	 * Variable query to use for checking if the objective completes
	 * Leave empty when the objective can't complete
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FQuestMemoryQuery CompleteQuery;

	/**
	 * Variable query to use for checking if the objective fails
	 * Leave empty when the objective can't fail
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FQuestMemoryQuery FailQuery;
};
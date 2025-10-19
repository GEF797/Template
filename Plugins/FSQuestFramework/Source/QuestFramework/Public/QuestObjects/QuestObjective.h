// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "QuestData.h"
#include "QuestObjectBase.h"
#include "QuestTextTypes.h"

#include "QuestObjective.generated.h"

class UQuestObjectiveObserver;
class UQuestContext;

// Describes significancy of an objective. Stage evaluates each objective based on their significancy
UENUM(BlueprintType)
enum class EQuestObjectiveSignificancy : uint8
{
	/*
	* Default.
	* Stage end when all regular objectives are not in progress anymore
	* Stage fails if objective is failed and there are no more objectives in progress
	* Stage completes if all objectives are completed
	*/
	Regular = 0,

	/**
	* Not evaluated towards stage completion/failure
	*/
	Optional,


	/**
	* The stage completes immediately upon objective completion (exception when RequiredToComplete is set).
	*/
	ForceComplete,

	/**
	* Same as Regular but on failure, the quest stage is failed immediately when the objective fails
	*/
	ForceFailure,

	/**
	* The stage is completed immediately upon objective completion (exception when RequiredToComplete is set).
	* The stage is failed immediately upon objective failure.
	*/
	ForceCompleteAndFailure,

	/**
	* Objective must be completed before stage can complete.
	* This overrules ForceComplete and ForceCompleteAndFailure
	* Stage automatically fails if this fails
	*/
	RequiredToComplete,
};

/**
 * Represents single objective inside quest stage
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew)
class QUESTFRAMEWORK_API UQuestObjective : public UQuestObjectBase
{
	GENERATED_BODY()

public:

	UQuestObjective();

	// Logical name of the objective
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName LogicalName = NAME_None;

	/*
	* Short description of the objective.
	* Can be used to show objectives in a list in HUD etc.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective", meta = (ShowOnlyInnerProperties))
	FQuestText ShortDescription;

	/*
	* Long description of the objective.
	* Can be used to give more detailed information about the objective in journal etc.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective", meta = (ShowOnlyInnerProperties))
	FQuestText LongDescription;

	/**
	 * Significancy of the objective.
	 * See comments in EQuestObjectiveSignificancy
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	EQuestObjectiveSignificancy Significancy = EQuestObjectiveSignificancy::Regular;

	/**
	 * Observer logic to determine when this objective is completed/failed
	 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Objective")
	TObjectPtr<UQuestObjectiveObserver> Observer;

	/**
	* Should the objective automatically activated when a stage is entered?
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective", AdvancedDisplay)
	bool bAutoActivate = true;

	/**
	 * Should objective use custom observer class to determine completed/failed logic?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective", AdvancedDisplay)
	bool bUseCustomObserver = false;

public:

	// Runs evaluation if this objective is completed/failed
	EQuestResult EvaluateObserver(const UQuestContext* Context) const;

	// Checks if the objective is active in given quest context
	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool IsActive(const UQuestContext* Context) const;

	/*
	* Checks if this objective uses any text arguments in any of its quest text fields
	* This information can be used to update the objective description per tick
	*/
	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool HasTextArguments() const;

public:

#if WITH_EDITOR

	bool SuggestLogicalName(const FName& InName);

	virtual void PostLoad() override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

#endif

};

/**
 * Represents list of objectives in quest stage
 */
USTRUCT(BlueprintType)
struct FQuestStageObjectives
{
	GENERATED_BODY()

public:

	// List of objectives
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Objective", meta = (ShowOnlyInnerProperties))
	TArray<TObjectPtr<UQuestObjective>> ObjectiveArray;

public:

	FQuestStageObjectives() {}

	// Returns reference to array of instanced objectives
	const TArray<TObjectPtr<UQuestObjective>>& GetObjectives() const { return ObjectiveArray; }
};

USTRUCT()
struct FQuestObjectiveRepResult
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TObjectPtr<UQuestObjective> Obj;

	UPROPERTY()
	EQuestResult Result;

	FQuestObjectiveRepResult() 
	{
		Obj = nullptr;
		Result = EQuestResult::NotStarted;
	}
	FQuestObjectiveRepResult(UQuestObjective* InObj, EQuestResult InResult)
	{
		Obj = InObj;
		Result = InResult;
	}
};

/**
 * Replicates objective results from server to clients
 */
USTRUCT()
struct FQuestObjectiveResultReplicator
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TArray<FQuestObjectiveRepResult> RepResults;

	bool bDirty = false;

public:

	FQuestObjectiveResultReplicator() {}

	const TArray<FQuestObjectiveRepResult>& Get() const { return RepResults; }

	void MakeDirty() { bDirty = true; }

	void Replicate(const TMap<TObjectPtr<UQuestObjective>, EQuestResult>& ResultMap, bool bForceReplicate)
	{
		if (bForceReplicate || bDirty)
		{
			// Write map to the array to get it replicated
			for (const TPair<UQuestObjective*, EQuestResult>& ResultPair : ResultMap)
			{
				bool bAssigned = false;
				for (FQuestObjectiveRepResult& Res : RepResults)
				{
					if (Res.Obj == ResultPair.Key)
					{
						Res.Result = ResultPair.Value;
						bAssigned = true;
						break;
					}
				}
				if (bAssigned)
				{
					continue;
				}
				check(!bAssigned);
				RepResults.Add(FQuestObjectiveRepResult(ResultPair.Key, ResultPair.Value));
			}
			bDirty = false;
		}
	}

	void Reconstruct(TMap<TObjectPtr<UQuestObjective>, EQuestResult>& ResultMap, TFunctionRef<void(TObjectPtr<UQuestObjective>, EQuestResult, EQuestResult)> ChangeFunc)
	{
		for (const FQuestObjectiveRepResult& Res : RepResults)
		{
			if (!Res.Obj)
			{
				continue;
			}

			EQuestResult* ResEnumPtr = ResultMap.Find(Res.Obj);
			if (!ResEnumPtr) // New
			{
				ResultMap.Add(Res.Obj, Res.Result);
				ChangeFunc(Res.Obj, EQuestResult::NotStarted, Res.Result);
			}
			else // Changed due replication
			{
				check(ResEnumPtr);
				const EQuestResult Old = *ResEnumPtr;
				*ResEnumPtr = Res.Result;
				ChangeFunc(Res.Obj, Old, Res.Result);
			}
		}
	}
};
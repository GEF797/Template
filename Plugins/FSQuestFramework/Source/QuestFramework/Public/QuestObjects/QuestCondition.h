// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestData.h"
#include "QuestMemory.h"
#include "QuestCondition.generated.h"

// Comparison method for conditions when there are multiple conditions to evaluate
UENUM(BlueprintType)
enum class EQuestConditionCompareMethod : uint8
{
	// All conditions must pass to pass the test
	All = 0,

	// If any of conditions pass, the test passes
	Any,
};

/**
 * Condition object to determine simple true/false conditions
 * Implements overridable function to determine if condition pass or fail
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestCondition : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	// Internal
	bool ExecuteCheckCondition(class UQuestContext* Context) const;

protected:

	/**
	 * Override in BP or CheckCondition_Implementation in C++ to implement the condition check
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Condition")
	bool CheckCondition(class UQuestContext* Context) const;
	virtual bool CheckCondition_Implementation(class UQuestContext* Context) const { return false; }
};

/**
 * Condition to compare against quest variables
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew)
class QUESTFRAMEWORK_API UQuestCondition_VariableQuery : public UQuestCondition
{
	GENERATED_BODY()

protected:

	/**
	 * Variable query to use for checking the condition
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FQuestMemoryQuery Query;

	virtual bool CheckCondition_Implementation(class UQuestContext* Context) const override;
};

/**
 * Reperesents list of conditions
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestConditions
{
	GENERATED_BODY()

public:

	// List of conditions
	UPROPERTY(Instanced, EditAnywhere, Category = "Condition")
	TArray<TObjectPtr<UQuestCondition>> Conditions;

	// Method to use when evaluating multiple conditions
	UPROPERTY(EditAnywhere, Category = "Condition")
	EQuestConditionCompareMethod ComparisonMethod = EQuestConditionCompareMethod::All;

public:

	// Runs evalutation on conditions listed in the array
	bool EvaluateConditions(class UQuestContext* Context) const;

	// Creates a new instance of conditions for given outer by copying all condition instances (used by compiler)
	static FQuestConditions MakeNewInstance(UObject* Outer, const FQuestConditions& Template);
};
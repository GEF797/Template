// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestNode.h"
#include "QuestObjects/QuestCondition.h"
#include "QuestNode_Selector.generated.h"

/**
 * When this node is entered, selector chooses first satifying child based on conditions (from left to right)
 */
UCLASS()
class QUESTFRAMEWORK_API UQuestNode_Selector : public UQuestNode
{
	GENERATED_BODY()
	
public:

	/*
	* Runs evalutation for children for selection.
	* First satisfied child is chosen (from left to right)
	* Returns Index of child in children array, NONE if not any satisfies
	*/
	int32 EvaluateChildrenConditions(class UQuestContext* Context) const;

	/**
	 * Returns list of conditions to entry into child nodes.
	 */
	const TArray<FQuestConditions>& GetConditions() const { return Conditions; }

	// Returns condition for child by index in Children array
	const FQuestConditions* GetConditionsForChild(int32 ChildIndex) const;

	/**
	* Called by compiler
	*/
	void SetConditions(const TArray<FQuestConditions>& InConditions, const TMap<int32, int32>& InIndexTable)
	{
		Conditions = InConditions;
		ChildToConditionIndexTable = InIndexTable;
	}


protected:

	/**
	 * Conditions to enter into child nodes
	 */
	UPROPERTY()
	TArray<FQuestConditions> Conditions;

	/**
	 * Mapping set by compiler. Child index to index in Conditions array
	 */
	UPROPERTY()
	TMap<int32, int32> ChildToConditionIndexTable;

};

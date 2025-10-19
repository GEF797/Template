// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestNode.h"
#include "QuestNode_End.generated.h"

/**
 * 
 */
UCLASS()
class QUESTFRAMEWORK_API UQuestNode_End : public UQuestNode
{
	GENERATED_BODY()

public:

	// Does the quest end in a "Completed" or in a "Failed" result?
	UFUNCTION(BlueprintPure, Category = "Quest Node")
	EQuestResult GetEndResultType() const { return !bCompleted ? EQuestResult::Failed : EQuestResult::Completed; }

//protected:

	// Does the quest end in a "Completed" or in a "Failed" result?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node")
	bool bCompleted = true;
};

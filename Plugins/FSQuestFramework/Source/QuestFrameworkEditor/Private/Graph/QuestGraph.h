// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"

#include "Quest.h"

#include "QuestGraph.generated.h"

class UQuestGraphSchema;

class UQuestGraphNodeBase;
class UQuestGraphNode_Root;
class UQuestGraphNode_Selector;
class UQuestGraphNode_Connection;
class UQuestGraphNode_End;
class UQuestGraphNode_Stage;

/**
 * Editor graph for quests.
 * This is compiled into runtime format.
 */
UCLASS()
class UQuestGraph : public UEdGraph
{
	GENERATED_BODY()

public:

	const TArray<UEdGraphNode*>& GetGraphNodes() const { return Nodes; }
	TArray<UQuestGraphNode_Root*> GetGraphNodes_Root() const;
	TArray<UQuestGraphNode_Selector*> GetGraphNodes_Selector() const;
	TArray<UQuestGraphNode_Connection*> GetGraphNodes_Connection() const;
	TArray<UQuestGraphNode_End*> GetGraphNodes_End() const;
	TArray<UQuestGraphNode_Stage*> GetGraphNodes_Stage() const;

	bool RemoveQuestGraphNode(UEdGraphNode* Node);
	void RemoveAllNodes();

public:

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

public: // Helpers

	UQuest* GetQuest() const;

	const UQuestGraphSchema* GetQuestGraphSchema() const;

	FName GetNextSuggestedStageName()
	{
		const FString NameStr = FString::Printf(TEXT("Stage_%d"), NextStageIndex);
		NextStageIndex++;
		return FName(NameStr);
	}

protected:

	UPROPERTY()
	int32 NextStageIndex = 0;

};

// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "Nodes/QuestNode.h"

#include "QuestObjects/QuestObjective.h"
#include "QuestObjects/QuestService.h"

#include "QuestNode_Stage.generated.h"


/**
 * Node that consists of single quest stage.
 * Quest stage has objectives and ends to completion or failure.
 */
UCLASS()
class QUESTFRAMEWORK_API UQuestNode_Stage : public UQuestNode
{
	GENERATED_BODY()
	
public:

	// Returns logical name of this stage
	const FName& GetLogicalName() const { return LogicalName; }

	// Returns list of all objective default instances
	const TArray<TObjectPtr<UQuestObjective>>& GetObjectives() const { return Objectives.GetObjectives(); }

	// Returns list of objectives in this stage node that are active in given quest context 
	TArray<UQuestObjective*> GetActiveObjectives(class UQuestContext* Context) const;

	// Outputs all objective logical names into OutNames
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void GetAllObjectiveNames(TArray<FName>& OutNames) const;

	// Returns objective by given name. Null if not found
	UFUNCTION(BlueprintCallable, Category = "Quest")
	UQuestObjective* FindObjectiveByName(const FName& Name) const;

	// Returns list of all services (default instances)
	const TArray<UQuestService*>& GetServices() const { return Services; }

	// Getters for service objects
	TArray<UQuestService*> GetInstancedServices() const;
	TArray<UQuestService*> GetNonInstancedServices() const;

	/**
	 * Evaluates all objectives. Returns Index in children array to jump. NONE if should stay in the node
	 */
	int32 EvaluateObjectives(class UQuestContext* Context, EQuestResult& OutResult, TMap<UQuestObjective*, EQuestResult>& OutObjectiveResults) const;

protected:

	// Logical name of the stage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node")
	FName LogicalName = NAME_None;

	// Display name of this quest stage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node", meta = (ShowOnlyInnerProperties))
	FQuestText StageName;

	/*
	* Description text of this quest stage.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node", meta = (ShowOnlyInnerProperties))
	FQuestText StageDescription;

	/**
	 * List of objectives that are active during the quest stage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node")
	FQuestStageObjectives Objectives;

	/**
	 * Services to run when this stage is active
	 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Quest Node")
	TArray<TObjectPtr<UQuestService>> Services;

public:
#if WITH_EDITOR

	// Suggests logical name for this stage node. Called from editor module. Will not set the name if name already exists
	void SuggestLogicalName(const FName& InName);

	// Creates a new instance of objectives for all entries in FQuestStageObjectives that are null
	void CreateObjectiveObjects();

	// Generates placeholder names for objectives that are not having name yet
	void GenerateMissingObjectiveNames();

	// Returns true if all objectives have unique name
	bool HasUniqueObjectiveNames() const;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

#endif

protected:

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	int32 NextObjectiveNameIndex = 0;

#endif
};

// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "UObject/ObjectSaveContext.h"
#endif

#include "QuestCompilerAccess.h"

#include "Nodes/QuestNode.h"
#include "QuestData.h"
#include "QuestTextTypes.h"
#include "QuestMemory.h"
#include "QuestObjects/QuestService.h"
#include "QuestObjects/QuestCustomData.h"

#include "Quest.generated.h"

/**
 * Quest asset that contains graph of nodes.
 */
UCLASS(BlueprintType, meta = (DisplayThumbnail = "true"))
class QUESTFRAMEWORK_API UQuest : public UObject
{
	GENERATED_BODY()

public:

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostEditImport() override;

protected:

	// Display name of the quest
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText DisplayName;

	/**
	 * Custom data to store any game specific information such as quest rewards etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Quest")
	TObjectPtr<UQuestCustomData> CustomData;

	/**
	 * List of variables and their initial values.
	 * Variables in this list are created to the quest context's memory on creation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FDefaultQuestVariable> DefaultVariables;

	/*
	* Configuration for quest variable value bounds.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestVariableBounds> VariableBounds;

	/**
	 * Services to run while the quest context is in progress.
	 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuestService>> Services;

	/*
	* Unique identifier of this quest.
	* GUID is used to reference quests when their states are saved/loaded to prevent invalid save files in case the assets are moved
	*/
	UPROPERTY(VisibleAnywhere, Category = "Quest", AdvancedDisplay)
	FGuid QuestGuid;

	// Compiled quest nodes
	UPROPERTY(Instanced)
	TArray<TObjectPtr<UQuestNode>> Nodes;

public:

	// Generates unique identifier for this quest.
	void GenerateGuid();

	// Returns unique identifier of the quest
	UFUNCTION(BlueprintPure, Category = "Quest")
	FGuid GetGuid() const { return QuestGuid; }

	// Checks if the quest is having a valid identifier
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasValidGuid() const { return QuestGuid.IsValid(); }

public:

	// Returns display name of the quest
	UFUNCTION(BlueprintPure, Category = "Quest")
	FText GetDisplayName() const { return DisplayName; }

	// Returns custom data object associated with the asset.
	UFUNCTION(BlueprintPure, Category = "Quest")
	UQuestCustomData* GetCustomData() const { return CustomData; }

	// returns variable initializers that are used to create and initialize certain variables to certain initial value on quest context creation
	const TArray<FDefaultQuestVariable>& GetDefaultQuestVariables() const { return DefaultVariables; }

	// returns variable value bounds that should be used with this quest when initializing quest memory block
	const TArray<FQuestVariableBounds>& GetQuestVariableBounds() const { return VariableBounds; }

	// Returns list of all quest nodes. DON'T edit the nodes!
	const TArray<UQuestNode*>& GetQuestNodes() const { return Nodes; }

	// Sets quests nodes into this quest (called by compiler)
	void SetQuestNodes(const TArray<UQuestNode*>& InNodes) { Nodes = InNodes; }

	// Returns root node of the quest
	UFUNCTION(BlueprintCallable, Category = "Quest")
	class UQuestNode_Root* GetRootNode() const;

	// Outputs all stage node names into OutNames
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void GetAllStageNodesNames(TArray<FName>& OutNames) const;

	// Returns stage node by given name. Null if not found
	UFUNCTION(BlueprintCallable, Category = "Quest")
	class UQuestNode_Stage* FindStageNodeByName(const FName& Name) const;

	// Returns list of all services (default instances)
	const TArray<UQuestService*>& GetServices() const { return Services; }

	// Getters for service objects
	TArray<UQuestService*> GetInstancedServices() const;
	TArray<UQuestService*> GetNonInstancedServices() const;

/// ------ EDITOR ------------- //
#if WITH_EDITOR
public:

	const UEdGraph* GetGraph() const { check(QuestGraph); return QuestGraph; }
	UEdGraph* GetGraph() { check(QuestGraph); return QuestGraph; }
	bool HasGraph() const { return QuestGraph != nullptr; }
	void SetGraph(UEdGraph* Graph);
	
	DECLARE_EVENT_OneParam(UQuest, FQuestPropertyChanged, const FPropertyChangedEvent&);
	FQuestPropertyChanged OnQuestPropertyChanged;

	void SetCompilerAccess(IQuestCompilerAccess* Access) { CompilerAccess = Access; }
	void CompileInEditor();

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#endif

protected:

#if WITH_EDITORONLY_DATA

	/*
	* Graph for Quest.
	* This is compiled into more compact data format and is available only in editor
	* */
	UPROPERTY()
	TObjectPtr<UEdGraph> QuestGraph;

	// Set from editor module when this is being edited.
	IQuestCompilerAccess* CompilerAccess = nullptr;

#endif

/// ------ END EDITOR ------------- //
};

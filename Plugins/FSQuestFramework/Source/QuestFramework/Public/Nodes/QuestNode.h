// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "QuestData.h"
#include "QuestTextTypes.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestObjects/QuestCustomData.h"

#include "QuestNode.generated.h"

/**
 * Abstract base for runtime nodes in quest framework.
 * These are contained inside quest asset
 */
UCLASS(Abstract, BlueprintType, EditInlineNew)
class QUESTFRAMEWORK_API UQuestNode : public UObject
{
	GENERATED_BODY()

public:

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostEditImport() override;
	
public:

	// Returns text from this node that can be used in journals etc.
	UFUNCTION(BlueprintCallable, Category = "Quest Node")
	FText GetNodeText(class UQuestContext* Context) const;

	// Returns custom data object associated with the asset.
	UFUNCTION(BlueprintPure, Category = "Quest")
	UQuestCustomData* GetCustomData() const { return CustomData; }

	// Returns index of the node (index in quest node array)
	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetNodeIndex() const { return NodeIndex; }

	// Sets index for this node. Set from compiler
	void SetNodeIndex(int32 InIndex) { NodeIndex = InIndex; }

	// Returns unique identifier of the node
	UFUNCTION(BlueprintPure, Category = "Quest")
	FGuid GetNodeGuid() const { return NodeGuid; }

	// Called by compiler to set guid for the node
	void SetNodeGuid(const FGuid& InGuid) { NodeGuid = InGuid; }

	// Generates a new guid for the node if the guid is not set
	void GenerateNodeGuid();

	// Set from compiler
	void SetChildrenIndicies(const TArray<int32>& ChildIndicies) { Children = ChildIndicies; }

	// Returns list of node indicies that are child of this node
	const TArray<int32>& GetChildren() const { return Children; }

	// Executes events from EntryEvents array
	void ExecuteEntryEvents(class UQuestContext* Context, bool bWasStateRestored);

protected:

	// Index of this node. Assigned in compilation
	UPROPERTY()
	int32 NodeIndex = INDEX_NONE;

	// Unique identifier of this node
	UPROPERTY()
	FGuid NodeGuid = FGuid();

	// Indicies of children nodes
	UPROPERTY()
	TArray<int32> Children;

	/**
	 * Events that are executed when this node is entered
	 */
	UPROPERTY(Instanced, EditAnywhere, Category = "Quest Node")
	TArray<TObjectPtr<UQuestEvent>> EntryEvents;

	/**
	 * Custom data to store any game specific information such as quest rewards etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Quest Node")
	TObjectPtr<UQuestCustomData> CustomData;

	/*
	* Display text for this node.
	* Can be used to display text in journals etc.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Node", meta = (ShowOnlyInnerProperties))
	FQuestText NodeText;

#if WITH_EDITOR

public:

	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

#endif
};

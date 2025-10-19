// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"

#include "Graph/QuestGraphSchema.h"
#include "Graph/QuestGraph.h"

#include "QuestEditor/QuestEditorStyles.h"

#include "QuestGraphNodeBase.generated.h"

#define QUEST_GRAPH_PIN_INPUT ((int32)0)
#define QUEST_GRAPH_PIN_OUTPUT1 ((int32)1)
#define QUEST_GRAPH_PIN_OUTPUT2 ((int32)2)

/**
 * Abstract base for all nodes that can exist in quest graph
 */
UCLASS(Abstract)
class UQuestGraphNodeBase : public UEdGraphNode
{
	GENERATED_BODY()

public:

	UQuestGraphNodeBase(const FObjectInitializer& ObjectInitializer);

	// Object
	virtual void PostLoad() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostEditImport() override;
	virtual void BindDelegates();

	// GraphNode
	virtual void PostPlacedNewNode() override;
	virtual void AllocateDefaultPins() override;
	virtual void ReconstructNode() override;
	virtual bool CanDuplicateNode() const override { return true; }
	virtual bool CanUserDeleteNode() const override { return true; }
	virtual void PrepareForCopying() override { Super::PrepareForCopying(); }
	virtual void RemovePinAt(int32 PinIndex, EEdGraphPinDirection PinDirection) override {}
	virtual bool CanSplitPin(const UEdGraphPin* Pin) const override { return false; }
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;

	
	virtual bool HasConnectionToNode(const UEdGraphNode* TargetNode, EEdGraphPinDirection Direction = EEdGraphPinDirection::EGPD_Output) const;
	virtual bool CanHaveInputConnections() const { return true; }
	virtual bool CanHaveOutputConnections() const { return true; }
	virtual bool CanHaveMultipleOutputConnections() const { return false; }

	// Slate & styles
	virtual FLinearColor GetNodeStyleColor() const { return FLinearColor::White; }
	virtual bool ShowPaletteIconOnNode() const override { return true; }
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor::White; }
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FIntPoint GetPosition() const { return FIntPoint(NodePosX, NodePosY); }
	virtual void SetPosition(int32 X, int32 Y) { NodePosX = X; NodePosY = Y; }

public: // ------------ PINS -------------- //

	UEdGraphPin* GetInputPin() const;
	UEdGraphPin* GetOutputPinA() const;
	UEdGraphPin* GetOutputPinB() const;

	bool HasInputPin() const;
	bool HasOutputPinA() const;
	bool HasOutputPinB() const;

protected:

	// Override these to create input and output pins required by the derived node
	virtual void CreateInputPin() { checkNoEntry(); }
	virtual void CreateOutputPins() { checkNoEntry(); }


protected: // Delegate binds

	// Called when any of properties in quest asset changes
	virtual void OnQuestPropertyChanged(const FPropertyChangedEvent& e) {}

public: // Helpers

	UQuestGraph* GetQuestGraph() const;
	UQuest* GetQuest() const;
	const UQuestGraphSchema* GetQuestGraphSchema() const;

public: // Compiler

	void SetCompilerWarningMessage(FString Message);
	void SetCompilerErrorMessage(FString Message);
	void ClearCompilerMessage();
};

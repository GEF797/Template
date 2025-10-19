// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectIterator.h"
#include "EdGraph/EdGraphSchema.h"
#include "AssetRegistry/AssetData.h"
#include "GraphEditorActions.h"
#include "ToolMenu.h"

#include "QuestGraphSchema.generated.h"

class UQuest;

/**
 * 
 */
UCLASS()
class UQuestGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public: // Schema

	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;

	virtual FPinConnectionResponse MovePinLinks(UEdGraphPin& MoveFromPin, UEdGraphPin& MoveToPin, bool bIsIntermediateMove = false, bool bNotifyLinkedNodes = false) const override;

	virtual FPinConnectionResponse CopyPinLinks(UEdGraphPin& CopyFromPin, UEdGraphPin& CopyToPin, bool bIsIntermediateCopy = false) const override;

	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;

	virtual bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

	virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;

	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;

	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;

	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	virtual void BreakPinLinkToPin(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin, bool bSendsNodeNotifcation) const;

	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;

	virtual bool ShouldAlwaysPurgeOnModification() const override { return true; }

	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;


public: // Helpers

	UQuest* GetQuest(UEdGraph* Graph) const;

private:

	void GetNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder) const;
};

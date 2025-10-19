// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphNode.h"
#include "SGraphPin.h"

/**
 * Factories to create nodes in quest graph
 */

struct FQuestGraphPinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<SGraphPin> CreatePin(class UEdGraphPin* Pin) const override;
};


struct FQuestGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<SGraphNode> CreateNode(class UEdGraphNode* InNode) const override;
};
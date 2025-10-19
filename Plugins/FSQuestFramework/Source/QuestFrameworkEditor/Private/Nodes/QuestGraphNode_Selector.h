// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "QuestGraphNode_Selector.generated.h"

/**
 * 
 */
UCLASS()
class UQuestGraphNode_Selector : public UQuestGraphNodeCommonBase
{
	GENERATED_BODY()

public:

	virtual FLinearColor GetNodeStyleColor() const override {
		return FQuestEditorStyles::NodeBodyColor_Selector;
	}
	
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual bool CanHaveMultipleOutputConnections() const override { return true; }

	virtual TSubclassOf<UQuestNode> GetQuestNodeClass() const override;
};

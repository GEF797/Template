// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "QuestGraphNode_Root.generated.h"

/**
 * 
 */
UCLASS()
class UQuestGraphNode_Root : public UQuestGraphNodeCommonBase
{
	GENERATED_BODY()
	
public:

	virtual bool CanHaveInputConnections() const override { return false; }
	virtual bool CanHaveOutputConnections() const override { return true; }

	virtual FLinearColor GetNodeStyleColor() const override {
		return FQuestEditorStyles::NodeBodyColor_Root;
	}

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual TSubclassOf<UQuestNode> GetQuestNodeClass() const override;
};

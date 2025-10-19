// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "QuestGraphNode_Stage.generated.h"

/**
 * 
 */
UCLASS()
class UQuestGraphNode_Stage : public UQuestGraphNodeCommonBase
{
	GENERATED_BODY()
	
public:

	virtual FLinearColor GetNodeStyleColor() const override {
		return FQuestEditorStyles::NodeBodyColor_Stage;
	}

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual TSubclassOf<UQuestNode> GetQuestNodeClass() const override;

protected:

	virtual void CreateOutputPins() override;
};

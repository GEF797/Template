// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestGraphNodeCommonBase.h"
#include "QuestGraphNode_End.generated.h"

/**
 * 
 */
UCLASS()
class UQuestGraphNode_End : public UQuestGraphNodeCommonBase
{
	GENERATED_BODY()
	
public:

	virtual bool CanHaveInputConnections() const override { return true; }
	virtual bool CanHaveOutputConnections() const override { return false; }

	virtual FLinearColor GetNodeStyleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual TSubclassOf<UQuestNode> GetQuestNodeClass() const override;

public:

	EQuestResult GetResultType() const;
};

// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"
#include "Nodes/QuestGraphNodeBase.h"

#include "GraphSchemaAction_NewQuestNode.generated.h"

class UQuestGraphNodeBase;
class UQuest;

/** Action to add a node to the graph */
USTRUCT()
struct FGraphSchemaAction_NewQuestNode : public FEdGraphSchemaAction
{
private:

	typedef FGraphSchemaAction_NewQuestNode Self;

public:

	GENERATED_BODY();

	FGraphSchemaAction_NewQuestNode() : FEdGraphSchemaAction() {}
	FGraphSchemaAction_NewQuestNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping, TSubclassOf<UQuestGraphNodeBase> InGraphNodeClass);

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

private:

	UEdGraphNode* CreateGraphNode(UQuest* Quest, UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D& Location, bool bSelectNewNode);
	TSubclassOf<UQuestGraphNodeBase> GraphNodeClass;
};


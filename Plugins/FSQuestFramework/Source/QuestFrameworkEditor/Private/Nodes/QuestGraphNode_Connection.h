// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "Nodes/QuestGraphNodeBase.h"
#include "QuestObjects/QuestCondition.h"

#include "QuestGraphNode_Connection.generated.h"

class UQuestGraphNodeCommonBase;

/**
 * Graph node for links between two quest nodes
 */
UCLASS()
class UQuestGraphNode_Connection : public UQuestGraphNodeBase
{
	GENERATED_BODY()
	
public:

	virtual bool ModifyBase(bool bAlwaysMarkDirty = true);
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	
public: // Graph

	void CreateConnections(UQuestGraphNodeCommonBase* ParentNode, UQuestGraphNodeCommonBase* ChildNode, bool bBParentOutputPin);

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	bool HasParentNode() const;
	bool HasChildNode() const;

	UQuestGraphNodeCommonBase* GetParentNode() const;
	UQuestGraphNodeCommonBase* GetChildNode() const;

	UEdGraphPin* GetParentLinkPin() const;
	UEdGraphPin* GetChildLinkPin() const;

	const FQuestConditions& GetConditions() const { return Conditions; }

	bool HasSelectorParent() const;

protected:

	virtual void CreateInputPin() override;
	virtual void CreateOutputPins() override;

protected:

	/**
	 * Conditions to enter into next node.
	 * Compiled into selector node for runtime usage
	 */
	UPROPERTY(EditAnywhere, Category = "Connection")
	FQuestConditions Conditions;
};

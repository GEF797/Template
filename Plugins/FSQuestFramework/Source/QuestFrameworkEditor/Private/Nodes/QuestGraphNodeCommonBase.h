// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Nodes/QuestGraphNodeBase.h"
#include "Nodes/QuestNode.h"
#include "QuestGraphNodeCommonBase.generated.h"

class UQuestGraphNode_Connection;

/**
 * Abstract base for all "common" nodes in the graph
 */
UCLASS()
class UQuestGraphNodeCommonBase : public UQuestGraphNodeBase
{
	GENERATED_BODY()

public:

	// Object
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostLoad() override;

	// Graph
	TArray<UQuestGraphNode_Connection*> GetConnectionsToParents() const;
	TArray<UQuestGraphNode_Connection*> GetConnectionsToChildren() const;
	TArray<UQuestGraphNode_Connection*> GetConnectionsToChildrenOfPin(bool bOutputPinA) const;

	TArray<UQuestGraphNodeCommonBase*> GetParentNodes() const;
	TArray<UQuestGraphNodeCommonBase*> GetChildrenNodes() const;

	TArray<UQuestGraphNodeCommonBase*> GetChildrenNodesOfPin(bool bOutputPinA) const;

	int32 GetQuestNodeIndex() const { return NodeIndex; }
	void SetQuestNodeIndex(int32 InIndex) { NodeIndex = InIndex; }

	virtual bool HasConnectionToNode(const UEdGraphNode* TargetNode, EEdGraphPinDirection Direction = EEdGraphPinDirection::EGPD_Output) const override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

protected:

	virtual void CreateInputPin() override;
	virtual void CreateOutputPins() override;

public:

	// Template to create underlying quest node
	template<class TQuestNodeType>
	TQuestNodeType* ConstructQuestNodeT(TSubclassOf<UQuestNode> NodeClass = TQuestNodeType::StaticClass())
	{
		return NewObject<TQuestNodeType>(this, NodeClass, NAME_None, RF_Transactional);
	}

	// Template to get quest node as certain tyte
	template<class TQuestNodeType>
	TQuestNodeType* GetQuestNodeChecked() const
	{
		TQuestNodeType* Node = Cast<TQuestNodeType>(QuestNode);
		check(Node);
		return Node;
	}

	// Template to get quest node as certain tyte
	template<class TQuestNodeType>
	TQuestNodeType* GetQuestNodeUnchecked() const
	{
		return Cast<TQuestNodeType>(QuestNode);
	}

	virtual TSubclassOf<UQuestNode> GetQuestNodeClass() const { checkNoEntry(); return nullptr; }
	virtual void ConstructQuestNode();

	static FName GetMemberNameChecked_QuestNode() { return GET_MEMBER_NAME_CHECKED(UQuestGraphNodeCommonBase, QuestNode); }

protected:

	UPROPERTY(EditAnywhere, Instanced, Category = "Quest Node", meta = (ShowOnlyInnerProperties))
	TObjectPtr<UQuestNode> QuestNode = nullptr;

	UPROPERTY()
	int32 NodeIndex = INDEX_NONE;
};

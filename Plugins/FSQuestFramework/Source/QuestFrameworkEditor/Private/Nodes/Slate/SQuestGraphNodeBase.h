// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "SGraphNode.h"
#include "SGraphPanel.h"

#include "Layout/Visibility.h"
#include "Widgets/Layout/SBox.h"

#include "Nodes/QuestGraphNodeBase.h"

/**
 * Slate widget (base) for quest graph nodes
 */
class SQuestGraphNodeBase : public SGraphNode
{
	typedef SGraphNode Super;
	typedef SQuestGraphNodeBase Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

public: // Slate

	void Construct(const FArguments& InArgs, UQuestGraphNodeBase* InNode);

	virtual void CreatePinWidgets() override;

	virtual void CreateStandardPinWidget(UEdGraphPin* Pin) override;

	virtual void UpdateGraphNode() override;

	virtual void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;

protected:

	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	virtual TSharedPtr<SGraphPin> CreatePinWidget(UEdGraphPin* Pin) const override;

	bool EnsurePin(UEdGraphPin* Pin);

public:

	virtual EVisibility GetVisibility() const { return EVisibility::Visible; }

protected:

	UQuestGraphNodeBase* QuestGraphNode = nullptr;

	TSharedPtr<SHorizontalBox> InputPinsNodeBox;
	TSharedPtr<SHorizontalBox> OutputPinsNodeBox;
};
// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "Widgets/SCompoundWidget.h"
#include "ConnectionDrawingPolicy.h"

/**
 * Slate representation of pin in quest graph
 */
class SQuestGraphPin : public SGraphPin
{
	typedef SGraphPin Super;
	typedef SQuestGraphPin Self;

public:

	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FReply OnPinMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

protected:

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	virtual FSlateColor GetPinColor() const override;

	const FSlateBrush* GetPinBorder() const;
};

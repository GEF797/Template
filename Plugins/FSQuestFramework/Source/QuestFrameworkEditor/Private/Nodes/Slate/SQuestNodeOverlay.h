// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Overlay widgets that can be overlayed into quest graph node widgets
 */
class SQuestNodeOverlay : public SCompoundWidget
{
	typedef SQuestNodeOverlay Self;
	typedef SCompoundWidget Super;

public:

	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool);
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetBackgroundColor, bool);

	SLATE_BEGIN_ARGS(SQuestNodeOverlay) {}
	SLATE_ATTRIBUTE(TSharedPtr<SWidget>, OverlayBody)

	SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
	SLATE_EVENT(FOnGetBackgroundColor, OnGetBackgroundColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(true);
		SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	}

	void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(false);
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}

	FSlateColor GetBackgroundColor() const
	{
		if (OnGetBackgroundColorEvent.IsBound())
			return OnGetBackgroundColorEvent.Execute(IsHovered());

		return FSlateColor::UseForeground();
	}

private:

	FOnHoverStateChanged OnHoverStateChangedEvent;
	FOnGetBackgroundColor OnGetBackgroundColorEvent;

	TSharedPtr<SWidget> OverlayBody;
};

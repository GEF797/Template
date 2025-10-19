// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDocumentation.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Layout/WidgetPath.h"


/**
 * Custom picker widget for selecting quest variables
 */
class SQuestVariablePicker : public SCompoundWidget
{
	typedef SQuestVariablePicker Self;
	typedef SCompoundWidget Super;

public:

	SLATE_BEGIN_ARGS(Self)
		: _OnTextChanged()
		, _OnTextCommitted()
		, _OnKeyDownHandler()
		, _PropertyHandle()
		, _AvailableVariableNames(TArray<FName>())
	{}

	SLATE_EVENT(FOnTextChanged, OnTextChanged)

	SLATE_EVENT(FOnTextCommitted, OnTextCommitted)

	SLATE_EVENT(FOnKeyDown, OnKeyDownHandler)

	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)

	SLATE_ATTRIBUTE(TArray<FName>, AvailableVariableNames)

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	void SetPropertyHandle(const TSharedPtr<IPropertyHandle>& InPropertyHandle);

	void SetText(const TAttribute<FText>& InNewText);

	virtual FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual bool SupportsKeyboardFocus() const override { return true; }

	virtual bool HasKeyboardFocus() const override { return SearchBoxWidget->HasKeyboardFocus(); }

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

private:

	TSharedRef<SWidget> GetMenuWidget();

	TSharedRef<SWidget> GetSearchBoxWidget();

	TSharedRef<SWidget> GetListViewWidget();

	TSharedRef<ITableRow> HandleListGenerateRow(TSharedPtr<FName> Text, const TSharedRef<STableViewBase>& OwnerTable);

	FText GetHighlightText() const { return SearchBoxWidget->GetText(); }

	void HandleMenuOpenChanged(bool bOpen);

	void HandleComboBoxOpened();

	void HandleTextChanged(const FText& InSearchText);

	void HandleTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo);

	FReply HandleKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	void HandleListSelectionChanged(TSharedPtr<FName> NewValue, ESelectInfo::Type SelectInfo);

	void UpdateSearchResults();

	TSharedPtr<FName> GetSelectedVariableName() const;

	void SetFocusToSearchBoxWidget() const;

private:

	FOnTextChanged OnTextChanged;

	FOnTextCommitted OnTextCommitted;

	FOnKeyDown OnKeyDownHandler;

	TSharedPtr<IPropertyHandle> PropertyHandle;

	TAttribute<TArray<FName>> VariableNameAttributes;

	TAttribute<FText> TextAttribute;

	TSharedPtr<SComboButton> ComboButtonWidget;

	TSharedPtr<STextBlock> ComboButtonTextBlockWidget;

	TSharedPtr<SVerticalBox> MenuWidget;

	TSharedPtr<SOverlay> ListViewOverlayWidget;

	TSharedPtr<SBorder> ListViewBorderWidget;

	TSharedPtr<SListView<TSharedPtr<FName>>> ListViewWidget;

	TSharedPtr<SWidget> SearchBoxWrapperWidget;

	TSharedPtr<SSearchBox> SearchBoxWidget;

	TArray<TSharedPtr<FName>> SearchResults;
};

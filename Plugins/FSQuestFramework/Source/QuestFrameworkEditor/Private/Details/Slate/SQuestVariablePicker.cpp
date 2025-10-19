// Copyright, Sami Kangasmaa 2022


#include "Details/Slate/SQuestVariablePicker.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "SQuestVariablePicker"

void SQuestVariablePicker::Construct(const FArguments& InArgs)
{
	OnTextChanged = InArgs._OnTextChanged;
	OnTextCommitted = InArgs._OnTextCommitted;
	OnKeyDownHandler = InArgs._OnKeyDownHandler;
	
	if (InArgs._PropertyHandle.IsValid())
	{
		SetPropertyHandle(InArgs._PropertyHandle);
	}

	VariableNameAttributes = InArgs._AvailableVariableNames;

	ComboButtonWidget = SNew(SComboButton)
		.ButtonStyle(FAppStyle::Get(), "PropertyEditor.AssetComboStyle")
		.ForegroundColor(FAppStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
		.IsEnabled(true)
		.IsFocusable(false)
		.ContentPadding(2.25f)
		.OnGetMenuContent(this, &Self::GetMenuWidget)
		.OnMenuOpenChanged(this, &Self::HandleMenuOpenChanged)
		.OnComboBoxOpened(this, &Self::HandleComboBoxOpened)
		.ButtonContent()
		[
			SAssignNew(ComboButtonTextBlockWidget, STextBlock)
			.TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
			.Text(TextAttribute)
		];

	TSharedPtr<SHorizontalBox> MainContent = SNew(SHorizontalBox);
	MainContent->AddSlot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				ComboButtonWidget.ToSharedRef()
			]
		];

	ChildSlot
		[
			MainContent.ToSharedRef()
		];
}

void SQuestVariablePicker::SetPropertyHandle(const TSharedPtr<IPropertyHandle>& InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;
	check(PropertyHandle.IsValid());

	FText PropertyText;
	const FPropertyAccess::Result ReadResult = PropertyHandle->GetValueAsFormattedText(PropertyText);
	if (ReadResult == FPropertyAccess::Success)
	{
		SetText(PropertyText);
	}
}

void SQuestVariablePicker::SetText(const TAttribute<FText>& InNewText)
{
	const FText NewText = InNewText.Get();
	TextAttribute.Set(NewText);

	PropertyHandle->SetValueFromFormattedString(*NewText.ToString());

	if (SearchBoxWidget.IsValid())
	{
		SearchBoxWidget->SetText(TextAttribute);
	}
	if (ComboButtonTextBlockWidget.IsValid())
	{
		ComboButtonTextBlockWidget->SetText(TextAttribute);
	}
}

FReply SQuestVariablePicker::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ListViewWidget->ClearSelection();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SQuestVariablePicker::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	if (IsEnabled() && ComboButtonWidget.IsValid() && ComboButtonWidget->IsEnabled())
	{
		return FReply::Handled().SetUserFocus(SearchBoxWidget.ToSharedRef(), InFocusEvent.GetCause());
	}
	return Super::OnFocusReceived(MyGeometry, InFocusEvent);
}

TSharedRef<SWidget> SQuestVariablePicker::GetMenuWidget()
{
	if (MenuWidget.IsValid())
	{
		return MenuWidget.ToSharedRef();
	}

	MenuWidget = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.f, 2.f, 2.f, 5.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 1.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SearchBarTitleText", "Press enter to create a new variable"))
				.TextStyle(FCoreStyle::Get(), TEXT("Menu.Heading"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 1)
			[
				GetSearchBoxWidget()
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(1.f, 2.f)
			[
				GetListViewWidget()
			]
		];

	return MenuWidget.ToSharedRef();
}

TSharedRef<SWidget> SQuestVariablePicker::GetSearchBoxWidget()
{
	if (SearchBoxWrapperWidget.IsValid())
	{
		return SearchBoxWrapperWidget.ToSharedRef();
	}

	SearchBoxWidget = SNew(SSearchBox)
		.InitialText(TextAttribute)
		.OnTextChanged(this, &Self::HandleTextChanged)
		.OnTextCommitted(this, &Self::HandleTextCommitted)
		.SelectAllTextWhenFocused(true)
		.DelayChangeNotificationsWhileTyping(true)
		.OnKeyDownHandler(this, &Self::HandleKeyDown);

	SearchBoxWrapperWidget = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SearchBoxWidget.ToSharedRef()
		];

	return SearchBoxWrapperWidget.ToSharedRef();
}

TSharedRef<SWidget> SQuestVariablePicker::GetListViewWidget()
{
	if (ListViewOverlayWidget.IsValid())
	{
		return ListViewOverlayWidget.ToSharedRef();
	}

	ListViewBorderWidget = SNew(SBorder)
		.Padding(0)
		.BorderImage(FAppStyle::GetBrush("NoBorder"));

	ListViewWidget = SNew(SListView<TSharedPtr<FName>>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&SearchResults)
		.OnGenerateRow(this, &Self::HandleListGenerateRow)
		.OnSelectionChanged(this, &Self::HandleListSelectionChanged)
		.ItemHeight(20);

	auto AddScrollBorder = [](TSharedRef<STableViewBase> Base) -> TSharedRef<SWidget>
	{
		return SNew(SScrollBorder, Base)
			[
				Base
			];
	};

	ListViewBorderWidget->SetContent(AddScrollBorder(ListViewWidget.ToSharedRef()));

	ListViewOverlayWidget = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			ListViewBorderWidget.ToSharedRef()
		];

	return ListViewOverlayWidget.ToSharedRef();
}

TSharedRef<ITableRow> SQuestVariablePicker::HandleListGenerateRow(TSharedPtr<FName> Text, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!ensure(Text.IsValid()))
	{
		return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(FText::FromString("INVALID!"))
			];
	}

	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromName(*Text.Get()))
			.HighlightText(this, &Self::GetHighlightText)
		];
}

void SQuestVariablePicker::HandleMenuOpenChanged(bool bOpen)
{
	if (!bOpen)
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::SetDirectly);
		ComboButtonWidget->SetMenuContent(SNullWidget::NullWidget);
	}
}

void SQuestVariablePicker::HandleComboBoxOpened()
{
	GetSearchBoxWidget();
	SetFocusToSearchBoxWidget();
	UpdateSearchResults();
}

void SQuestVariablePicker::HandleTextChanged(const FText& InSearchText)
{
	UpdateSearchResults();
	OnTextChanged.ExecuteIfBound(InSearchText);
}

void SQuestVariablePicker::HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::Default)
	{
		return;
	}

	TSharedPtr<FName> SelectedVariableName = GetSelectedVariableName();
	FText VariableNameText;
	if (SelectedVariableName.IsValid() && CommitType != ETextCommit::OnCleared)
	{
		VariableNameText = FText::FromName(*SelectedVariableName.Get());
	}
	else
	{
		if (CommitType == ETextCommit::OnCleared)
		{
			VariableNameText = FText::GetEmpty();
		}
		else
		{
			VariableNameText = NewText;
		}
	}

	if (VariableNameText.IdenticalTo(FText::GetEmpty()))
	{
		VariableNameText = FText::FromName(NAME_None);
	}

	SetText(VariableNameText);
	OnTextCommitted.ExecuteIfBound(VariableNameText, CommitType);

	if (CommitType != ETextCommit::OnUserMovedFocus)
	{
		ComboButtonWidget->SetIsOpen(false);
	}
}

FReply SQuestVariablePicker::HandleKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (OnKeyDownHandler.IsBound())
	{
		return OnKeyDownHandler.Execute(MyGeometry, InKeyEvent);
	}
	return FReply::Unhandled();
}

void SQuestVariablePicker::HandleListSelectionChanged(TSharedPtr<FName> NewValue, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo == ESelectInfo::OnMouseClick || SelectInfo == ESelectInfo::OnKeyPress || SelectInfo == ESelectInfo::Direct)
	{
		if (!NewValue.IsValid())
		{
			ComboButtonWidget->SetIsOpen(false);
		}
		else
		{
			SetText(FText::FromName(*NewValue.Get()));
			SetFocusToSearchBoxWidget();
		}
	}
}

void SQuestVariablePicker::UpdateSearchResults()
{
	const FString TypedText = SearchBoxWidget.IsValid() ? SearchBoxWidget->GetText().ToString() : TEXT("");
	SearchResults.Empty();

	TArray<FName> Results = VariableNameAttributes.Get();

	if (TypedText.Len() > 0 && TypedText != TextAttribute.Get().ToString())
	{
		for (const FName& Result : Results)
		{
			if (Result.ToString().Contains(TypedText))
			{
				SearchResults.Add(MakeShared<FName>(Result));
			}
		}
	}
	else
	{
		for (const FName& Result : Results)
		{
			SearchResults.Add(MakeShared<FName>(Result));
		}
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

TSharedPtr<FName> SQuestVariablePicker::GetSelectedVariableName() const
{
	TSharedPtr<FName> SelectedName;
	const TArray<TSharedPtr<FName>>& SelectedItemsList = ListViewWidget->GetSelectedItems();
	if (SelectedItemsList.Num() > 0)
	{
		ensure(SelectedItemsList.Num() == 1);
		SelectedName = SelectedItemsList[0];
	}
	return SelectedName;
}

void SQuestVariablePicker::SetFocusToSearchBoxWidget() const
{
	FWidgetPath WidgetToFocusPath;
	FSlateApplication::Get().GeneratePathToWidgetChecked(SearchBoxWidget.ToSharedRef(), WidgetToFocusPath);
	FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
}

#undef LOCTEXT_NAMESPACE
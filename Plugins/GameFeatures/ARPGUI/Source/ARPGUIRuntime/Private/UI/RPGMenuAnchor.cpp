// Copyright G.E.FC 2024


#include "UI/RPGMenuAnchor.h"

TSharedRef<SWidget> URPGMenuAnchor::RebuildWidget()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
		MyMenuAnchor = SNew(SMenuAnchor)
		.Placement(Placement)
		.FitInWindow(bFitInWindow)
		.OnGetMenuContent(BIND_UOBJECT_DELEGATE(FOnGetContent, HandleGetMenuContent))
		.OnMenuOpenChanged(BIND_UOBJECT_DELEGATE(FOnIsOpenChanged, HandleMenuOpenChanged))
		.ShouldDeferPaintingAfterWindowContent(ShouldDeferPaintingAfterWindowContent)
		.UseApplicationMenuStack(UseApplicationMenuStack);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
		if (GetChildrenCount() > 0)
		{
			MyMenuAnchor->SetContent(GetContentSlot()->Content ? GetContentSlot()->Content->TakeWidget() : SNullWidget::NullWidget);
		}

	return MyMenuAnchor.ToSharedRef();
}

TSharedRef<SWidget> URPGMenuAnchor::HandleGetMenuContent()
{
	TSharedPtr<SWidget> SlateMenuWidget;

	if (OnGetUserMenuContentEvent.IsBound())
	{
		UWidget* MenuWidget = OnGetUserMenuContentEvent.Execute();
		

		if (MenuWidget)
		{
			SlateMenuWidget = MenuWidget->TakeWidget();
			Widget = MenuWidget;
		}
	}
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	else if (OnGetMenuContentEvent.IsBound())
	{
		// Remove when OnGetMenuContentEvent is fully deprecated.
		UWidget* MenuWidget = OnGetMenuContentEvent.Execute();
		if (MenuWidget)
		{
			SlateMenuWidget = MenuWidget->TakeWidget();
			Widget = MenuWidget;
		}
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	else
	{
		if (MenuClass != nullptr && !MenuClass->HasAnyClassFlags(CLASS_Abstract))
		{
			if (UWorld* World = GetWorld())
			{
				if (UUserWidget* MenuWidget = CreateWidget(World, MenuClass))
				{
					SlateMenuWidget = MenuWidget->TakeWidget();
					Widget = MenuWidget;
				}
			}
		}
	}

	return SlateMenuWidget.IsValid() ? SlateMenuWidget.ToSharedRef() : SNullWidget::NullWidget;
}

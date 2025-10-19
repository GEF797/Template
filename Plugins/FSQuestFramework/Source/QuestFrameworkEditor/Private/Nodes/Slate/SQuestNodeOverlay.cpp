// Copyright Sami Kangasmaa, 2022


#include "Nodes/Slate/SQuestNodeOverlay.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"

void SQuestNodeOverlay::Construct(const FArguments& InArgs)
{
	OnHoverStateChangedEvent = InArgs._OnHoverStateChanged;
	OnGetBackgroundColorEvent = InArgs._OnGetBackgroundColor;
	OverlayBody = InArgs._OverlayBody.Get();

	const FSlateBrush* CircleBrush = FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Index"));
	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(CircleBrush->ImageSize.X)
				.HeightOverride(CircleBrush->ImageSize.Y)
			]

			+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(CircleBrush)
					.BorderBackgroundColor(this, &Self::GetBackgroundColor)
					.Padding(FMargin(4.0f))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						OverlayBody.ToSharedRef()
					]
				]
		];
}


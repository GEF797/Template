// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddWidget.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFeaturesSubsystemSettings.h"
#include "CommonUIExtensions.h"
#include "UI/LyraHUD.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddWidget)

#define LOCTEXT_NAMESPACE "LyraGameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWidgets

void UGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Remove the context first so unregister callbacks cannot recreate its UI state.
	FPerContextData ActiveData;
	if (ContextData.RemoveAndCopyValue(Context, ActiveData))
	{
		Reset(ActiveData);
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWidgets::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	Super::AddAdditionalAssetBundleData(AssetBundleData);

	for (const FLyraHUDLayoutRequest& Entry : Layout)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.LayoutClass.ToSoftObjectPath().GetAssetPath());
	}

	for (const FLyraHUDElementEntry& Entry : Widgets)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.WidgetClass.ToSoftObjectPath().GetAssetPath());
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidgets::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	{
		int32 EntryIndex = 0;
		for (const FLyraHUDLayoutRequest& Entry : Layout)
		{
			if (Entry.LayoutClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutHasNullClass", "Null WidgetClass at index {0} in Layout"), FText::AsNumber(EntryIndex)));
			}

			if (!Entry.LayerID.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutHasNoTag", "LayerID is not set at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	{
		int32 EntryIndex = 0;
		for (const FLyraHUDElementEntry& Entry : Widgets)
		{
			if (Entry.WidgetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNullClass", "Null WidgetClass at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
			}

			if (!Entry.SlotID.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNoTag", "SlotID is not set at index {0} in Widgets"), FText::AsNumber(EntryIndex)));
			}
			++EntryIndex;
		}
	}

	return Result;
}
#endif

void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{			
			TSoftClassPtr<AActor> HUDActorClass = ALyraHUD::StaticClass();

			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(
				HUDActorClass,
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));
			ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddWidgets::Reset(FPerContextData& ActiveData)
{
	ActiveData.ComponentRequests.Empty();

	for (TPair<FObjectKey, TSharedPtr<FPerActorData>>& Pair : ActiveData.ActorData)
	{
		Pair.Value->Reset();
	}
	ActiveData.ActorData.Empty();
}

void UGameFeatureAction_AddWidgets::FPerActorData::Reset()
{
	bIsRemoved = true;

	// Detach the handles before callbacks can request another cleanup.
	TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsToRemove = MoveTemp(LayoutsAdded);
	TArray<FUIExtensionHandle> ExtensionsToRemove = MoveTemp(ExtensionHandles);

	for (const TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : LayoutsToRemove)
	{
		if (UCommonActivatableWidget* Widget = AddedLayout.Get())
		{
			// Also remove inactive layouts underneath another widget in the layer stack.
			UCommonUIExtensions::PopContentFromLayer(Widget);
		}
	}

	for (FUIExtensionHandle& Handle : ExtensionsToRemove)
	{
		Handle.Unregister();
	}
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (!ActiveData)
	{
		return;
	}

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveWidgets(Actor, *ActiveData);
	}
	else if (EventName == TEXT("AddWidgets"))
	{
		AddWidgets(Actor, *ActiveData);
	}
}

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	AHUD* HUD = Cast<AHUD>(Actor);
	if (!IsValid(HUD))
	{
		return;
	}

	if (!HUD->GetOwningPlayerController())
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
	{
		UWorld* World = HUD->GetWorld();
		UUIExtensionSubsystem* ExtensionSubsystem = World ? World->GetSubsystem<UUIExtensionSubsystem>() : nullptr;
		if (!Widgets.IsEmpty() && !ExtensionSubsystem)
		{
			return;
		}

		TSharedPtr<FPerActorData>& ActorDataSlot = ActiveData.ActorData.FindOrAdd(HUD);
		if (!ActorDataSlot)
		{
			ActorDataSlot = MakeShared<FPerActorData>();
		}

		// Widget callbacks can remove this HUD or deactivate the feature while adding UI.
		const TSharedPtr<FPerActorData> ActorData = ActorDataSlot;
		if (ActorData->bIsAdding || !ActorData->LayoutsAdded.IsEmpty() || !ActorData->ExtensionHandles.IsEmpty())
		{
			return;
		}
		TGuardValue<bool> AddingGuard(ActorData->bIsAdding, true);

		for (const FLyraHUDLayoutRequest& Entry : Layout)
		{
			if (TSubclassOf<UCommonActivatableWidget> ConcreteWidgetClass = Entry.LayoutClass.Get())
			{
				UCommonActivatableWidget* AddedLayout = UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.LayerID, ConcreteWidgetClass);
				if (ActorData->bIsRemoved)
				{
					if (AddedLayout)
					{
						UCommonUIExtensions::PopContentFromLayer(AddedLayout);
					}
					return;
				}
				if (AddedLayout)
				{
					ActorData->LayoutsAdded.Add(AddedLayout);
				}
			}
		}

		for (const FLyraHUDElementEntry& Entry : Widgets)
		{
			FUIExtensionHandle Handle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(Entry.SlotID, LocalPlayer, Entry.WidgetClass.Get(), -1);
			if (ActorData->bIsRemoved)
			{
				if (Handle.IsValid())
				{
					Handle.Unregister();
				}
				return;
			}
			if (Handle.IsValid())
			{
				ActorData->ExtensionHandles.Add(Handle);
			}
		}
	}
}

void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	AHUD* HUD = Cast<AHUD>(Actor);
	if (!HUD)
	{
		return;
	}

	// Only unregister if this is the same HUD actor that was registered, there can be multiple active at once on the client
	TSharedPtr<FPerActorData> ActorData;
	if (ActiveData.ActorData.RemoveAndCopyValue(HUD, ActorData))
	{
		ActorData->Reset();
	}
}

#undef LOCTEXT_NAMESPACE


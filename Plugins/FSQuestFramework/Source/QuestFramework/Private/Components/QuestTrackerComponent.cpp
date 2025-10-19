// Copyright Sami Kangasmaa, 2022


#include "Components/QuestTrackerComponent.h"
#include "QuestFrameworkModule.h"

#include "GameFramework/PlayerController.h"

#include "QuestContext.h"
#include "Components/QuestComponent.h"

#include "ViewModels/QuestTrackerViewModel.h"
#include "ViewModels/QuestListViewModel.h"
#include "ViewModels/QuestContextViewModel.h"

#include "QuestUtilityLibrary.h"
#include "QuestFrameworkSettings.h"


UQuestTrackerComponent::UQuestTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	ViewModel = nullptr;
	ListViewModel = nullptr;
}

void UQuestTrackerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!IsLocallyControlled())
	{
		return;
	}

	check(IsLocallyControlled());
	if (bAddPlayerAsSourceOnInit)
	{
		if (UQuestComponent* QuestComponent = UQuestUtilityLibrary::GetQuestComponentFromActor(GetOwner()))
		{
			AddSourceQuestComponent(QuestComponent);
		}
	}

	// Create view models
	TSubclassOf<UQuestTrackerViewModel> TrackerVMClass = UQuestTrackerViewModel::StaticClass();
	TSubclassOf<UQuestListViewModel> ListVMClass = UQuestListViewModel::StaticClass();

	const UQuestFrameworkSettings* Settings = GetDefault<UQuestFrameworkSettings>();
	if (Settings)
	{
		if (IsValid(Settings->QuestTrackerViewModelClass))
		{
			TrackerVMClass = Settings->QuestTrackerViewModelClass;
		}

		if (IsValid(Settings->QuestListViewModelClass))
		{
			ListVMClass = Settings->QuestListViewModelClass;
		}
	}

	ViewModel = NewObject<UQuestTrackerViewModel>(this, TrackerVMClass, NAME_None, RF_Transient);
	ListViewModel = NewObject<UQuestListViewModel>(this, ListVMClass, NAME_None, RF_Transient);
}

void UQuestTrackerComponent::UninitializeComponent()
{
	for (int32 Idx = SourceQuestComponents.Num() - 1; Idx >= 0; Idx--)
	{
		if (SourceQuestComponents[Idx].IsValid())
		{
			OnQuestComponentRemoved(SourceQuestComponents[Idx].Get());
		}
	}
	SourceQuestComponents.Empty();

	Super::UninitializeComponent();
}

void UQuestTrackerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Untrack quests that are ended
	if (IsLocallyControlled() && bAutoTrackNewQuests)
	{
		TArray<UQuestContext*> Tracked;
		GetTrackedQuestContexts(Tracked);
		for (UQuestContext* Context : Tracked)
		{
			if (Context && Context->HasEnded())
			{
				StopTrackQuest(Context);
			}
		}

		if (TrackedQuestContexts.Num() <= 0)
		{
			AutoTrackNextQuest();
		}
	}
}

void UQuestTrackerComponent::AddSourceQuestComponent(UQuestComponent* QuestComponent)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(QuestComponent))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't add source quest component to %s because the component was invalid!"), *GetFName().ToString());
		return;
	}

	bool bFound = false;
	for (const TWeakObjectPtr<UQuestComponent>& Component : SourceQuestComponents)
	{
		if (Component.IsValid() && Component.Get() == QuestComponent)
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		SourceQuestComponents.Add(QuestComponent);
		OnQuestComponentAdded(QuestComponent);
	}
}

void UQuestTrackerComponent::RemoveSourceQuestComponent(UQuestComponent* QuestComponent)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(QuestComponent))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't remove source quest component to %s because the component was invalid!"), *GetFName().ToString());
		return;
	}

	for (int32 Idx = SourceQuestComponents.Num() - 1; Idx >= 0; Idx--)
	{
		const TWeakObjectPtr<UQuestComponent>& Component = SourceQuestComponents[Idx];
		if (Component.IsValid() && Component.Get() == QuestComponent)
		{
			SourceQuestComponents.RemoveAt(Idx);
			OnQuestComponentRemoved(QuestComponent);
			break;
		}
	}
}

void UQuestTrackerComponent::TrackQuest(UQuestContext* QuestContext)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(QuestContext))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't track quest in %s because the quest context was invalid!"), *GetFName().ToString());
		return;
	}

	if (!TrackedQuestContexts.Contains(QuestContext))
	{
		// Replace the current one
		if (bTrackOnlySingle && TrackedQuestContexts.Num() > 0)
		{
			ensure(TrackedQuestContexts.Num() == 1);
			StopTrackQuest(TrackedQuestContexts[0].Get());
			TrackedQuestContexts.Empty();
		}

		TrackedQuestContexts.Add(QuestContext);
		OnBeginTrackQuest.Broadcast(QuestContext);
		UE_LOG(LogQuestFramework, Log, TEXT("%s started tracking quest context %s (quest asset %s)"), *GetFName().ToString(), *QuestContext->GetFName().ToString(), *QuestContext->GetQuestAsset()->GetFName().ToString());

		if (UQuestContextViewModel* ContextViewModel = QuestContext->GetViewModel())
		{
			ContextViewModel->SetIsTracked(true);
		}
		RefreshViewModel(true, false);
	}
}

void UQuestTrackerComponent::StopTrackQuest(UQuestContext* QuestContext)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(QuestContext))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't stop track quest in %s because the quest context was invalid!"), *GetFName().ToString());
		return;
	}

	if (TrackedQuestContexts.Remove(QuestContext) > 0)
	{
		OnStopTrackQuest.Broadcast(QuestContext);
		UE_LOG(LogQuestFramework, Log, TEXT("%s stopped tracking quest context %s (quest asset %s)"), *GetFName().ToString(), *QuestContext->GetFName().ToString(), *QuestContext->GetQuestAsset()->GetFName().ToString());

		if (UQuestContextViewModel* ContextViewModel = QuestContext->GetViewModel())
		{
			ContextViewModel->SetIsTracked(false);
		}
		RefreshViewModel(true, false);
	}
}

void UQuestTrackerComponent::GetAllQuestContexts(TArray<UQuestContext*>& OutContexts) const
{
	if (!IsLocallyControlled())
	{
		return;
	}

	for (const TWeakObjectPtr<UQuestComponent>& QuestCompPtr : SourceQuestComponents)
	{
		if (QuestCompPtr.IsValid())
		{
			QuestCompPtr->GetAllActiveQuestContexts(OutContexts);
		}
	}
}

void UQuestTrackerComponent::GetTrackedQuestContexts(TArray<UQuestContext*>& OutContexts) const
{
	if (!IsLocallyControlled())
	{
		return;
	}

	OutContexts.Reserve(TrackedQuestContexts.Num());
	for (const TWeakObjectPtr<UQuestContext>& ContextPtr : TrackedQuestContexts)
	{
		if (ContextPtr.IsValid())
		{
			OutContexts.Add(ContextPtr.Get());
		}
	}
}

void UQuestTrackerComponent::RefreshViewModel(bool bTrackerViewModel, bool bQuestListViewModel)
{
	if (ViewModel != nullptr && bTrackerViewModel)
	{
		TArray<UQuestContext*> Contexts;
		GetTrackedQuestContexts(Contexts);
		ViewModel->SetTrackedQuestContexts(Contexts);
	}

	if (ListViewModel != nullptr && bQuestListViewModel)
	{
		TArray<UQuestContext*> Contexts;
		GetAllQuestContexts(Contexts);
		ListViewModel->SetQuestContexts(Contexts, true);
	}
}

void UQuestTrackerComponent::OnQuestComponentAdded(UQuestComponent* Comp)
{
	check(Comp);
	Comp->OnQuestContextCreated.AddDynamic(this, &UQuestTrackerComponent::HandleOnQuestContextCreated);
	Comp->OnQuestContextSuspend.AddDynamic(this, &UQuestTrackerComponent::HandleOnQuestContextSuspended);

	RefreshViewModel();
}

void UQuestTrackerComponent::OnQuestComponentRemoved(UQuestComponent* Comp)
{
	check(Comp);
	Comp->OnQuestContextCreated.RemoveDynamic(this, &UQuestTrackerComponent::HandleOnQuestContextCreated);
	Comp->OnQuestContextSuspend.RemoveDynamic(this, &UQuestTrackerComponent::HandleOnQuestContextSuspended);

	// Stop tracking when the component get removed
	TArray<UQuestContext*> Tracked;
	GetTrackedQuestContexts(Tracked);
	for (UQuestContext* Context : Tracked)
	{
		if (Context && Context->GetOwnerQuestComponent() == Comp)
		{
			StopTrackQuest(Context);
		}
	}

	RefreshViewModel();
}

void UQuestTrackerComponent::AutoTrackNextQuest(UQuestContext* IgnoreContext)
{
	// Pick first one in progress
	if (TrackedQuestContexts.Num() <= 0)
	{
		TArray<UQuestContext*> AvailableContexts;
		GetAllQuestContexts(AvailableContexts);
		for (UQuestContext* NewContext : AvailableContexts)
		{
			if (NewContext && NewContext != IgnoreContext && NewContext->GetQuestState() == EQuestResult::InProgress)
			{
				TrackQuest(NewContext);
				break;
			}
		}
	}
}

bool UQuestTrackerComponent::IsLocallyControlled() const
{
	if (APlayerController* PC = GetOwner<APlayerController>())
	{
		return PC->IsLocalPlayerController();
	}
	return false;
}

void UQuestTrackerComponent::Clear()
{
	ViewModel->Clear();
	ViewModel=nullptr;

	GetListViewModel()->Clear();
	ListViewModel=nullptr;

	for(auto SourceQuestComponent: SourceQuestComponents)
	{
		SourceQuestComponent->Clear();
	}
	SourceQuestComponents.Empty();

	for(auto TrackedQuestContext: TrackedQuestContexts)
	{
		TrackedQuestContext->Clear();
	}
	TrackedQuestContexts.Empty();
}

void UQuestTrackerComponent::HandleOnQuestContextCreated(UQuestComponent* QuestComponent, UQuestContext* Context)
{
	if (!Context)
	{
		return;
	}

	// Bind to delegates to notify when quest starts or ends
	Context->OnQuestStarted.AddDynamic(this, &UQuestTrackerComponent::HandleOnQuestStarted);
	Context->OnQuestEnded.AddDynamic(this, &UQuestTrackerComponent::HandleOnQuestEnded);

	RefreshViewModel();
}

void UQuestTrackerComponent::HandleOnQuestContextSuspended(UQuestComponent* QuestComponent, UQuestContext* Context)
{
	if (!Context)
	{
		return;
	}

	// Unbind from delegates to notify when quest starts or ends
	Context->OnQuestStarted.RemoveDynamic(this, &UQuestTrackerComponent::HandleOnQuestStarted);
	Context->OnQuestEnded.RemoveDynamic(this, &UQuestTrackerComponent::HandleOnQuestEnded);

	// Stop the suspended one
	StopTrackQuest(Context);

	// Pick first one in progress that is not the one suspended
	if (bAutoTrackNewQuests)
	{
		AutoTrackNextQuest(Context);
	}

	RefreshViewModel();
}

void UQuestTrackerComponent::HandleOnQuestStarted(UQuestContext* Context)
{
	if (!Context)
	{
		return;
	}

	// Start tracking the newly created quest
	if (bAutoTrackNewQuests)
	{
		if (bTrackOnlySingle && TrackedQuestContexts.Num() > 0)
		{
			return;
		}
		TrackQuest(Context);
	}

	RefreshViewModel();
}

void UQuestTrackerComponent::HandleOnQuestEnded(UQuestContext* Context)
{
	if (!Context)
	{
		return;
	}

	// Stop the ended one
	StopTrackQuest(Context);

	// Pick first one in progress
	if (bAutoTrackNewQuests)
	{
		AutoTrackNextQuest(Context);
	}

	RefreshViewModel();
}

// Copyright, Sami Kangasmaa 2022


#include "Components/QuestComponent.h"
#include "QuestFrameworkModule.h"
#include "QuestManager.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = false; // Manual replication via ReplicateSubobjects
}

void UQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate list of active quest contexts
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestComponent, ActiveQuestContexts, COND_None, REPNOTIFY_Always);
}

bool UQuestComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (Context)
		{
			bWrote |= Channel->ReplicateSubobject(Context, *Bunch, *RepFlags);
		}
	}

	return bWrote;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UQuestManager* Manager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->RegisterQuestComponent(this);
	}
}

void UQuestComponent::EndPlay(EEndPlayReason::Type Reason)
{
	// Suspend all quests of this component on end play
	if (GetOwnerRole() == ENetRole::ROLE_Authority)
	{
		SuspendAllQuests();
	}

	if (UQuestManager* Manager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->UnregisterQuestComponent(this);
	}
	Super::EndPlay(Reason);
}

void UQuestComponent::InternalEvaluate()
{
	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (IsValid(Context) && Context->ShouldUpdate())
		{
			Context->EvaluateContext();
		}
	}
}

void UQuestComponent::InternalReplicateContexts()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ClientStartLatentContexts();
		return;
	}

	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (IsValid(Context))
		{
			Context->ConditionalReplicate();
		}
	}
}

void UQuestComponent::InternalTickServices(float DeltaTime)
{
	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (IsValid(Context) && Context->ShouldUpdate())
		{
			Context->TickActiveServices(DeltaTime);
		}
	}
}

bool UQuestComponent::SaveQuestStates(FQuestSaveStateData& SaveData)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Only network authority can call SaveQuestStates"));
		return false;
	}

	UE_LOG(LogQuestFramework, Log, TEXT("%s saving quest system state..."), *GetFName().ToString());

	SaveData.Reset();

	// Stamp time
	SaveData.Timestamp = FDateTime::UtcNow();

	TSet<FGuid> FoundQuestAssetGuids;
	TSet<FGuid> FoundQuestContextGuids;

	// Map contexts under quest assets
	TMap<UQuest*, TArray<UQuestContext*>> QuestToContexts;
	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (!ensure(Context))
		{
			continue;
		}
		check(Context->GetQuestAsset());

		TArray<UQuestContext*>& Contexts = QuestToContexts.FindOrAdd(Context->GetQuestAsset());
		Contexts.Add(Context);

		if (FoundQuestContextGuids.Contains(Context->GetGuid()))
		{
			UE_LOG(LogQuestFramework, Error, TEXT("%s has duplicate guid! Save state data will be incomplete!"), *Context->GetFName().ToString());
			return false;
		}

		FoundQuestContextGuids.Add(Context->GetGuid());
	}

	for (const TPair<UQuest*, TArray<UQuestContext*>>& QuestContextPair : QuestToContexts)
	{
		UQuest* QuestAsset = QuestContextPair.Key;
		check(QuestAsset);

		const TArray<UQuestContext*>& Contexts = QuestContextPair.Value;
		check(Contexts.Num() > 0);

		if (FoundQuestAssetGuids.Contains(QuestAsset->GetGuid()))
		{
			UE_LOG(LogQuestFramework, Error, TEXT("%s has duplicate guid! Save state data will be incomplete!"), *QuestAsset->GetFName().ToString());
			return false;
		}
		FoundQuestAssetGuids.Add(QuestAsset->GetGuid());

		// Write contexts states
		FQuestContextSaveState ContextsSaveState = FQuestContextSaveState();
		ContextsSaveState.PrepareWrite(QuestAsset);
		for (const UQuestContext* Context : Contexts)
		{
			check(Context);
			ContextsSaveState.WriteContext(Context);
			UE_LOG(LogQuestFramework, Log, TEXT("Saved state of quest context (%s) that represents quest (%s)"), *Context->GetFName().ToString(), *QuestAsset->GetFName().ToString());
		}

		// Add to state
		SaveData.Data.Add(ContextsSaveState);
	}

	UE_LOG(LogQuestFramework, Log, TEXT("Quest system state was stored succesfully."));

	return true;
}

bool UQuestComponent::LoadQuestStates(const FQuestSaveStateData& SaveData, bool bForceRestore)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Only network authority can call LoadQuestStates"));
		return false;
	}

	UE_LOG(LogQuestFramework, Log, TEXT("Restoring quest system state..."));

	// Make sure that all quest assets are loaded as data contains quest references as guids
	UQuestManager::LoadAllQuestAssets(this);

	TArray<const UQuest*> AllQuestAssets;
	UQuestManager::GetAllLoadedQuestAssets(AllQuestAssets);

	UE_LOG(LogQuestFramework, Log, TEXT("Loading quest states (timestamp: %s), (num quests: %d)"), *SaveData.Timestamp.ToString(), SaveData.Data.Num());

	// Collect quests and how many contexts each quest needs before loading
	TMap<const UQuest*, TArray<FQuestContextSaveStateData>> ContextAllocations;
	for (const FQuestContextSaveState& ContextSaveState : SaveData.Data)
	{
		const UQuest* FoundQuestAsset = nullptr;
		const FGuid QuestAssetGuid = ContextSaveState.QuestGuid;
		for (const UQuest* QuestAsset : AllQuestAssets)
		{
			check(QuestAsset);
			if (QuestAsset->GetGuid() == QuestAssetGuid)
			{
				FoundQuestAsset = QuestAsset;
				break;
			}
		}

		if (!FoundQuestAsset && !bForceRestore)
		{
			UE_LOG(LogQuestFramework, Error, TEXT("Can't find quest asset by guid (%s) that match guid in save state! Either quest asset was not loaded correctly or was deleted. Use bForceRestore to load the state partially."), *QuestAssetGuid.ToString());
			return false;
		}

		if (FoundQuestAsset)
		{
			if (!ensure(!ContextAllocations.Contains(FoundQuestAsset)))
			{
				return false;
			}
			if (!ensure(ContextSaveState.ContextStates.Num() > 0))
			{
				return false;
			}
			ContextAllocations.Add(FoundQuestAsset, ContextSaveState.ContextStates);
		}
	}

	UE_LOG(LogQuestFramework, Log, TEXT("Clearing existing quest contexts..."));
	SuspendAllQuests();

	// Create new quest contexts for quest assets and restore their state
	for (const TPair<const UQuest*, TArray<FQuestContextSaveStateData>>& Allocation : ContextAllocations)
	{
		const UQuest* QuestAsset = Allocation.Key;
		check(QuestAsset);

		const TArray<FQuestContextSaveStateData>& ContextStates = Allocation.Value;
		if (ContextStates.Num() > 50)
		{
			UE_LOG(LogQuestFramework, Warning, TEXT("Over 50 contexts going to be loaded for quest %s. This is probably wrong."), *Allocation.Key->GetFName().ToString());
		}

		for (int32 Idx = 0; Idx < ContextStates.Num(); Idx++)
		{
			if (UQuestContext* Context = CreateQuestContext(const_cast<UQuest*>(QuestAsset), false))
			{
				const FQuestContextSaveStateData& State = ContextStates[Idx];
				State.RestoreQuestContext(Context);
			}
		}
	}

	UE_LOG(LogQuestFramework, Log, TEXT("Quest system state was restored succesfully."));

	return true;
}

UQuestContext* UQuestComponent::CreateQuestContext(UQuest* Quest, bool bStart)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Only network authority can call CreateQuestContext"));
		return nullptr;
	}

	if (!IsValid(Quest))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't start quest! Quest asset was invalid!"));
		return nullptr;
	}

	UQuestContext* NewContext = NewObject<UQuestContext>(this);
	check(NewContext);

	NewContext->InitializeContext(Quest);
	if (!NewContext->IsInitialized())
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't start %s. Quest context initialization failed!"), *Quest->GetFName().ToString());
		return nullptr;
	}

	ActiveQuestContexts.Add(NewContext);

	UE_LOG(LogQuestFramework, Log, TEXT("New quest context %s created for quest %s"), *NewContext->GetFName().ToString(), *Quest->GetFName().ToString());

	// Send event
	if (OnQuestContextCreated.IsBound())
	{
		OnQuestContextCreated.Broadcast(this, NewContext);
	}

	if (bStart)
	{
		NewContext->Start();
	}

	return NewContext;
}

void UQuestComponent::SuspendQuest(UQuestContext* QuestContext)
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Only network authority can call SuspendQuest"));
		return;
	}

	if (QuestContext)
	{
		// Send event
		if (OnQuestContextSuspend.IsBound())
		{
			OnQuestContextSuspend.Broadcast(this, QuestContext);
		}

		check(QuestContext->GetQuestAsset());
		UE_LOG(LogQuestFramework, Log, TEXT("Suspending quest context %s of quest %s"), *QuestContext->GetFName().ToString(), *QuestContext->GetQuestAsset()->GetFName().ToString());

		QuestContext->Uninitialize();
		ActiveQuestContexts.Remove(QuestContext);
	}
}

void UQuestComponent::SuspendAllQuests()
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("Only network authority can call SuspendAllQuests"));
		return;
	}

	TArray<UQuestContext*> SuspendList = ActiveQuestContexts;
	for (UQuestContext* Context : SuspendList)
	{
		SuspendQuest(Context);
	}
	ensure(ActiveQuestContexts.Num() == 0);
	ActiveQuestContexts.Empty();
}

void UQuestComponent::GetAllActiveQuestContexts(TArray<UQuestContext*>& OutContexts) const
{
	OutContexts.Append(ActiveQuestContexts);
}

void UQuestComponent::FindAllActiveQuestContexts(UQuest* Quest, TArray<UQuestContext*>& OutContexts) const
{
	for (UQuestContext* Context : ActiveQuestContexts)
	{
		if (IsValid(Context) && Context->IsInitialized())
		{
			const UQuest* QuestAsset = Context->GetQuestAsset();
			if (!ensure(QuestAsset))
			{
				continue;
			}
			if (Quest == QuestAsset)
			{
				OutContexts.Add(Context);
			}
		}
	}
}

UQuestContext* UQuestComponent::FindFirstActiveQuestContext(UQuest* Quest) const
{
	TArray<UQuestContext*> Contexts;
	FindAllActiveQuestContexts(Quest, Contexts);
	if (Contexts.Num() <= 0)
	{
		return nullptr;
	}
	check(Contexts.IsValidIndex(0) && Contexts[0] != nullptr);
	return Contexts[0];
}

bool UQuestComponent::HasActiveQuestContext(UQuest* Quest) const
{
	return FindFirstActiveQuestContext(Quest) != nullptr;
}

void UQuestComponent::Clear()
{
   for(auto QuestContext : ActiveQuestContexts)
   {
	   QuestContext->Clear();
	   QuestContext=nullptr;
   }
	ActiveQuestContexts.Empty();

	OnQuestContextSuspend.RemoveAll(this);
	OnQuestContextCreated.RemoveAll(this);
	for (auto ClientLatentContext : ClientLatentContexts)
	{
		ClientLatentContext->Clear();
		ClientLatentContext = nullptr;
	}
	ClientLatentContexts.Empty();
}

void UQuestComponent::OnRep_ActiveQuestContexts(const TArray<UQuestContext*>& OldActiveContexts)
{
	if (GetOwnerRole() == ENetRole::ROLE_Authority)
	{
		return;
	}

	// Find new and suspended contexts by comparing the arrays
	TArray<UQuestContext*, TInlineAllocator<8>> AddedContexts;
	TArray<UQuestContext*, TInlineAllocator<8>> RemovedContexts;

	// Find added
	for (UQuestContext* Context : ActiveQuestContexts)
	{
		ensure(Context);
		if (!OldActiveContexts.Contains(Context))
		{
			AddedContexts.Add(Context);
		}
	}

	// Find removed
	for (UQuestContext* Context : OldActiveContexts)
	{
		ensure(Context);
		if (!ActiveQuestContexts.Contains(Context))
		{
			RemovedContexts.Add(Context);
		}
	}

	// Broadcast events on clients
	for (UQuestContext* Context : AddedContexts)
	{
		if (Context)
		{
			if (Context->IsInitialized())
			{
				OnQuestContextCreated.Broadcast(this, Context);
				Context->ClientStartContext();
			}
			else
			{
				ClientLatentContexts.Add(Context);
			}
		}
	}
	for (UQuestContext* Context : RemovedContexts)
	{
		if (ClientLatentContexts.Contains(Context))
		{
			ClientLatentContexts.Remove(Context);
		}
		if (Context && Context->IsInitialized())
		{
			OnQuestContextSuspend.Broadcast(this, Context);
			Context->Uninitialize();
		}
	}
}

void UQuestComponent::ClientStartLatentContexts()
{
	ensure(GetOwnerRole() != ENetRole::ROLE_Authority);
	for (int32 Idx = ClientLatentContexts.Num() - 1; Idx >= 0; Idx--)
	{
		if (!ClientLatentContexts[Idx].IsValid())
		{
			ClientLatentContexts.RemoveAtSwap(Idx);
			continue;
		}
		check(ClientLatentContexts[Idx].IsValid());

		if (ClientLatentContexts[Idx].Get()->IsInitialized())
		{
			OnQuestContextCreated.Broadcast(this, ClientLatentContexts[Idx].Get());
			ClientLatentContexts[Idx].Get()->ClientStartContext();
			ClientLatentContexts.RemoveAtSwap(Idx);
			continue;
		}
	}
}
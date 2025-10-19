// Copyright Sami Kangasmaa, 2022


#include "QuestContext.h"
#include "QuestFrameworkModule.h"

#include "Nodes/QuestNode_Root.h"
#include "Nodes/QuestNode_Selector.h"
#include "Nodes/QuestNode_Stage.h"
#include "Nodes/QuestNode_End.h"

#include "Components/QuestComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "ViewModels/QuestContextViewModel.h"
#include "ViewModels/QuestObjectiveViewModel.h"

#include "QuestFrameworkSettings.h"

UQuestContext::UQuestContext()
{
	ContextMemory = nullptr;
	SetQuestState(EQuestResult::NotStarted, false);
	SetActiveNodeIndex(QUEST_INDEX_NONE, false);

	ViewModel = nullptr;
	ObjectiveViewModels.Empty();
}

void UQuestContext::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, ContextGuid, COND_InitialOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, QuestAsset, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, QuestStatePacked, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, CurrentNodeIndexPacked, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, ActiveObjectives, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, ObjectiveResultReplicator, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UQuestContext, MemoryReplicator, COND_None, REPNOTIFY_Always);
}

bool UQuestContext::ShouldExecuteNetworkAction(EQuestNetworkExecType ExecType) const
{
	if (ExecType == EQuestNetworkExecType::OnlyServer)
	{
		return HasAuthority();
	}
	else if (ExecType == EQuestNetworkExecType::OnlyClient)
	{
		return !HasAuthority();
	}
	return true; // Both
}

void UQuestContext::InitializeContext(UQuest* Quest)
{
	// Ensure that we aren't initializing the context again
	ensure(!bInitialized);

	if (!ensure(Quest))
	{
		return;
	}

	QuestAsset = Quest;

	// Allow only server to set node index and quest state. Clients get these via replication
	if (HasAuthority())
	{
		SetActiveNodeIndex(QUEST_INDEX_NONE, false);
		SetQuestState(EQuestResult::NotStarted, false);
	}

	// Create memory object to store variable values
	ContextMemory = NewObject<UQuestMemory>(this);

	// Initialize bounds for the variables
	check(ContextMemory);
	ContextMemory->InitializeValueBounds(Quest->GetQuestVariableBounds());

	// Initialize quest variable values from default variable initializer list
	const TArray<FDefaultQuestVariable>& VariableInitializer = Quest->GetDefaultQuestVariables();
	ContextMemory->InitializeDefaultVariables(VariableInitializer);

	// Set replication flags
	MemoryReplicator.SetVariableReplicationFlags(VariableInitializer);

	// Allow only server to initialize GUID
	if (GetAuthority() == EQuestNetworkAuthority::HasAuthority)
	{
		if (!ContextGuid.IsValid())
		{
			ContextGuid = FGuid::NewGuid();
		}
	}

	bInitialized = true;

	// Initialize view models to start sending data to widgets
	InitializeViewModels();
}


void UQuestContext::Uninitialize()
{
	if (bInitialized)
	{
		// Notify that the stage services comes deactive
		for (UQuestService* Service : ActiveStageServices)
		{
			if (ensure(IsValid(Service)))
			{
				Service->NotifyServiceBecomeDeactive(this);
			}
		}
		ActiveStageServices.Empty();

		InternalDeactivateContextServices();

		bInitialized = false;
	}
}

void UQuestContext::TickActiveServices(float DeltaTime)
{
	// Tick active context services
	for (UQuestService* Service : ActiveContextServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyTickService(this, DeltaTime);
		}
	}

	// Tick active stage services
	for (UQuestService* Service : ActiveStageServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyTickService(this, DeltaTime);
		}
	}

	// Refresh view models as services might have changed text arguments
	RefreshActiveObjectiveViewModels();
	if (ViewModel)
	{
		ViewModel->CustomUpdateViewModel(this);
	}
}

void UQuestContext::RestoreState(const FGuid& InGuid, int32 SavedNodeIndex, EQuestResult SavedQuestState, const FQuestMemoryBlock& SavedMemoryBlock)
{
	if (GetAuthority() != EQuestNetworkAuthority::HasAuthority)
	{
		return;
	}

	if (!ensure(bInitialized))
	{
		return;
	}

	if (InGuid.IsValid())
	{
		ContextGuid = InGuid;
	}

	SetQuestState(SavedQuestState, true);

	// Copy saved memory to memory object
	check(ContextMemory);
	FQuestMemoryBlock& Block = ContextMemory->GetMemoryBlock();
	Block = SavedMemoryBlock;
	Block.MakeReplicationDirty();
	ContextMemory->InitializeValueBounds(QuestAsset->GetQuestVariableBounds());

	// Update rep state
	ConditionalReplicate();

	// Switch to saved node
	if (GetQuestState() == EQuestResult::InProgress)
	{
		const int32 OldIndex = GetActiveNodeIndex();

		SetRequiresUpdatePipeline(true);
		InternalStartQuest(true);

		SetActiveNodeIndex(SavedNodeIndex, true);
		InternalOnQuestNodeChange(OldIndex, GetActiveNodeIndex(), EQuestResult::Completed, true);
	}
	else
	{
		SetActiveNodeIndex(SavedNodeIndex, true);
	}
	
	// Full refresh of view models
	RefreshViewModel(true, true, true);
}

void UQuestContext::Start()
{
	if (GetAuthority() != EQuestNetworkAuthority::HasAuthority)
	{
		return;
	}

	if (!ensure(bInitialized))
	{
		return;
	}

	if (GetQuestState() == EQuestResult::NotStarted)
	{
		SetQuestState(EQuestResult::InProgress, false);
		SetRequiresUpdatePipeline(true);
		InternalStartQuest();
	}
}

void UQuestContext::EvaluateContext()
{
	if (!IsInitialized())
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	// Don't evaluate the quest if it's not in progress
	if (GetQuestState() != EQuestResult::InProgress)
	{
		return;
	}

	check(QuestAsset);
	const TArray<UQuestNode*>& Nodes = QuestAsset->GetQuestNodes();
	if (!ensure(Nodes.IsValidIndex(GetActiveNodeIndex())))
	{
		return;
	}

	// Get current node from the list
	const UQuestNode* CurrentNode = Nodes[GetActiveNodeIndex()];

	// Next index to jump (none if we should stay in the current node)
	int32 NextIndex = QUEST_INDEX_NONE;
	EQuestResult Result = EQuestResult::InProgress;
	if (const UQuestNode_Root* RootNode = Cast<UQuestNode_Root>(CurrentNode))
	{
		NextIndex = InternalEvalNode_Root(RootNode);
	}
	else if (const UQuestNode_Selector* SelectorNode = Cast<UQuestNode_Selector>(CurrentNode))
	{
		NextIndex = InternalEvalNode_Selector(SelectorNode);
	}
	else if (const UQuestNode_Stage* StageNode = Cast<UQuestNode_Stage>(CurrentNode))
	{
		// Stage node is evaluated on remotes to predict objective results
		NextIndex = InternalEvalNode_Stage(StageNode, Result);
	}
	else if (const UQuestNode_End* EndNode = Cast<UQuestNode_End>(CurrentNode))
	{
		const bool bCompleted = InternalEvalNode_End(EndNode, Result);
		InternalEndQuest(bCompleted ? EQuestResult::Completed : EQuestResult::Failed);
		return;
	}

	// Should have ended and returned
	check(!CurrentNode->IsA<UQuestNode_End>());

	// Node should change
	if (NextIndex != QUEST_INDEX_NONE)
	{
		const int32 OldIndex = GetActiveNodeIndex();
		SetActiveNodeIndex(NextIndex, false);
		InternalOnQuestNodeChange(OldIndex, NextIndex, Result, false);
	}
}

UQuestNode* UQuestContext::GetActiveNode() const
{
	if (!IsInitialized() && !HasAuthority())
	{
		return nullptr;
	}

	if (!ensure(bInitialized))
	{
		return nullptr;
	}
	check(QuestAsset);

	if (QuestAsset->GetQuestNodes().IsValidIndex(GetActiveNodeIndex()))
	{
		return QuestAsset->GetQuestNodes()[GetActiveNodeIndex()];
	}
	return nullptr;
}

void UQuestContext::GetActiveObjectives(TArray<UQuestObjective*>& Objectives) const
{
	if (!IsInitialized() && !HasAuthority())
	{
		return;
	}

	if (UQuestNode_Stage* StageNode = GetActiveNodeUnchecked<UQuestNode_Stage>())
	{
		Objectives = ActiveObjectives;
	}
}

void UQuestContext::GetCurrentObjectives(TArray<UQuestObjective*>& Objectives) const
{
	if (!IsInitialized() && !HasAuthority())
	{
		return;
	}

	if (UQuestNode_Stage* StageNode = GetActiveNodeUnchecked<UQuestNode_Stage>())
	{
		TArray<UQuestObjective*> ObjectivesCopy = StageNode->GetObjectives();
		Objectives = ObjectivesCopy;
	}
}

bool UQuestContext::IsObjectiveActive(UQuestObjective* Objective) const
{
	if (!IsInitialized() && !HasAuthority())
	{
		return false;
	}

	if (!Objective)
	{
		return false;
	}
	return ActiveObjectives.Contains(Objective);
}

bool UQuestContext::TryActivateObjective(UQuestObjective* Objective)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!Objective)
	{
		return false;
	}

	if (UQuestNode_Stage* StageNode = GetActiveNodeUnchecked<UQuestNode_Stage>())
	{
		const bool bSuccess = InternalTryActivateObjective(Objective, StageNode);
		RefreshViewModel(false, true, false);
		return bSuccess;
	}

	return false;
}

void UQuestContext::DeactivateObjective(UQuestObjective* Objective)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Objective)
	{
		InternalDeactivateObjective(Objective);
		RefreshViewModel(false, true, false);
	}
}

EQuestResult UQuestContext::GetObjectiveResult(UQuestObjective* Objective, bool bReEvaluate) const
{
	if (!IsInitialized() && !HasAuthority())
	{
		return EQuestResult::NotStarted;
	}

	if (!Objective)
	{
		return EQuestResult::NotStarted;
	}

	if (bReEvaluate)
	{
		return Objective->EvaluateObserver(this);
	}
	
	if (LastObjectiveResults.Contains(Objective))
	{
		return LastObjectiveResults[Objective];
	}

	return EQuestResult::NotStarted;
}

bool UQuestContext::JumpToStageNode(const UQuestNode_Stage* StageNode)
{
	if (!ensure(IsInitialized()))
	{
		return false;
	}

	if (!HasAuthority())
	{
		return false;
	}

	check(QuestAsset);

	if (GetQuestState() != EQuestResult::InProgress)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("%s can't JumpToStageNode (quest %s) because the quest context is not in progress!"), *GetFName().ToString(), *QuestAsset->GetFName().ToString());
		return false;
	}

	bool bOwnedStageNode = false;
	const TArray<UQuestNode*>& Nodes = QuestAsset->GetQuestNodes();
	for (const UQuestNode* Node : Nodes)
	{
		if (Node && Node == StageNode)
		{
			bOwnedStageNode = true;
			break;
		}
	}

	if (!StageNode || !bOwnedStageNode)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("%s can't JumpToStageNode because either the provided stage node was invalid or quest %s doesn't own the given node!"), *GetFName().ToString(), *QuestAsset->GetFName().ToString());
		return false;
	}

	const int32 StageIndex = StageNode->GetNodeIndex();
	ensure(StageIndex != QUEST_INDEX_NONE && StageIndex >= 0);

	const int32 ActiveIndex = GetActiveNodeIndex();
	if (StageIndex == ActiveIndex) // Already in the stage
	{
		UE_LOG(LogQuestFramework, Warning, TEXT("%s can't JumpToStageNode in quest %s because the stage is already the currently active stage."), *GetFName().ToString(), *QuestAsset->GetFName().ToString());
		return false;
	}

	SetActiveNodeIndex(StageIndex, false);
	InternalOnQuestNodeChange(ActiveIndex, StageIndex, EQuestResult::Completed, false);
	return true;
}

bool UQuestContext::JumpToStageByName(FName StageName)
{
	if (!ensure(IsInitialized()))
	{
		return false;
	}

	if (!HasAuthority())
	{
		return false;
	}

	check(QuestAsset);
	const UQuestNode_Stage* Stage = QuestAsset->FindStageNodeByName(StageName);
	if (!Stage)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("%s can't JumpToStageNodeByName because %s doesn't have stage for name %s!"), *GetFName().ToString(), *QuestAsset->GetFName().ToString(), *StageName.ToString());
		return false;
	}
	return JumpToStageNode(Stage);
}

bool UQuestContext::EndQuest(bool Completed)
{
	if (!ensure(IsInitialized()))
	{
		return false;
	}

	if (!HasAuthority())
	{
		return false;
	}

	
	check(QuestAsset);
	for (UQuestNode* Node : QuestAsset->GetQuestNodes())
	{
		if (UQuestNode_End* EndNode = Cast<UQuestNode_End>(Node))
		{
			if (EndNode->bCompleted== Completed)
			{
				SetActiveNodeIndex(EndNode->GetNodeIndex(), false);
				InternalOnQuestNodeChange(EndNode->GetNodeIndex(), EndNode->GetNodeIndex(), Completed?EQuestResult::Completed: EQuestResult::Failed, false);

				return true;
			}
		}
	}

	return false;
}

AActor* UQuestContext::GetOwnerActor() const
{
	if (UQuestComponent* QuestComp = GetOwnerQuestComponent())
	{
		return QuestComp->GetOwner();
	}
	return nullptr;
}

AController* UQuestContext::GetOwnerController() const
{
	AActor* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		return nullptr;
	}

	AController* Controller = Cast<AController>(OwnerActor);
	if (!Controller)
	{
		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			Controller = OwnerPawn->GetController();
		}
	}
	return Controller;
}

int32 UQuestContext::InternalEvalNode_Root(const UQuestNode_Root* Node)
{
	check(Node);

	ensure(HasAuthority());

	// Root will pick the first child
	if (!Node->GetChildren().IsValidIndex(0))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("%s: root node doesn't have any children!"), *QuestAsset->GetFName().ToString());
		InternalEndQuest(EQuestResult::Failed);
		return QUEST_INDEX_NONE;
	}

	return Node->GetChildren()[0];
}

int32 UQuestContext::InternalEvalNode_Selector(const UQuestNode_Selector* Node)
{
	check(Node);

	ensure(HasAuthority());

	const int32 SatisfiedChildIndex = Node->EvaluateChildrenConditions(this);
	if (SatisfiedChildIndex == QUEST_INDEX_NONE)
	{
		UE_LOG(LogQuestFramework, Error, TEXT("%s: selector with node index %d does not have any satisfied children. Failing quest!"), *QuestAsset->GetFName().ToString(), Node->GetNodeIndex());
		InternalEndQuest(EQuestResult::Failed);
		return QUEST_INDEX_NONE;
	}

	const TArray<int32>& ChildrenIndicies = Node->GetChildren();
	if (!ensure(ChildrenIndicies.IsValidIndex(SatisfiedChildIndex)))
	{
		InternalEndQuest(EQuestResult::Failed);
		return QUEST_INDEX_NONE;
	}

	// Return satisfied node index
	return ChildrenIndicies[SatisfiedChildIndex];
}

int32 UQuestContext::InternalEvalNode_Stage(const UQuestNode_Stage* Node, EQuestResult& OutResult)
{
	check(Node);
	ensure(HasAuthority());

	TMap<UQuestObjective*, EQuestResult> NewObjectiveResults;
	const int32 NextNodeIndex = Node->EvaluateObjectives(this, OutResult, NewObjectiveResults);

	for (const TPair<UQuestObjective*, EQuestResult>& NewResult : NewObjectiveResults)
	{
		bool bChanged = false;
		if (LastObjectiveResults.Contains(NewResult.Key)) // Check if result changed
		{
			if (LastObjectiveResults[NewResult.Key] != NewResult.Value)
			{
				bChanged = true;
			}
		}
		else
		{
			bChanged = true; // Not yet contain the entry -> changed
		}

		if (bChanged)
		{
			LastObjectiveResults.Add(NewResult.Key, NewResult.Value);
			if (OnObjectiveResultChanged.IsBound())
			{
				OnObjectiveResultChanged.Broadcast(this, NewResult.Key, NewResult.Value);
			}

			// Refresh view model of the objective
			RefreshObjectiveViewModel(NewResult.Key);

			// Set replication state dirty when objective results change
			ObjectiveResultReplicator.MakeDirty();
		}
	}

	return NextNodeIndex;
}

bool UQuestContext::InternalEvalNode_End(const UQuestNode_End* Node, EQuestResult& OutResult)
{
	check(Node);
	ensure(HasAuthority());

	OutResult = Node->GetEndResultType();
	return Node->GetEndResultType() == EQuestResult::Completed;
}

void UQuestContext::SetRequiresUpdatePipeline(bool bShouldUpdate)
{
	if (bShouldUpdate != bShouldUpdateContext)
	{
		bShouldUpdateContext = true;
	}
}

void UQuestContext::InternalStartQuest(bool bSilent)
{
	check(QuestAsset);
	check(HasAuthority());

	// Find root node
	if (UQuestNode_Root* Root = QuestAsset->GetRootNode())
	{
		SetActiveNodeIndex(Root->GetNodeIndex(), false);
	}
	else
	{
		SetActiveNodeIndex(QUEST_INDEX_NONE, false);
		UE_LOG(LogQuestFramework, Error, TEXT("%s is missing root node! Can't start quest..."), *QuestAsset->GetFName().ToString());
		return;
	}

	if (!bSilent && GetActiveNodeIndex() != QUEST_INDEX_NONE)
	{
		if (OnQuestStarted.IsBound())
		{
			OnQuestStarted.Broadcast(this);
		}
	}

	// Activate context wide services
	InternalActivateContextServices();

	RefreshViewModel(true, true);
}

void UQuestContext::InternalEndQuest(EQuestResult Result)
{
	check(Result == EQuestResult::Completed || Result == EQuestResult::Failed);
	check(HasAuthority());

	SetQuestState(Result, false);

	// Deactivate context wide services when the quest ends
	InternalDeactivateContextServices();

	if (OnQuestEnded.IsBound())
	{
		OnQuestEnded.Broadcast(this);
	}

	RefreshViewModel(true, true);
}

void UQuestContext::InternalOnQuestNodeChange(int32 OldNodeIndex, int32 NewNodeIndex, EQuestResult OldResult, bool bWasStateRestored)
{
	check(QuestAsset);
	const TArray<UQuestNode*>& Nodes = QuestAsset->GetQuestNodes();
	if (!ensure(Nodes.IsValidIndex(NewNodeIndex)))
	{
		return;
	}

	// Notify that stage has ended
	if (Nodes.IsValidIndex(OldNodeIndex))
	{
		if (UQuestNode_Stage* OldStage = Cast<UQuestNode_Stage>(Nodes[OldNodeIndex]))
		{
			ensure(OldResult == EQuestResult::Completed || OldResult == EQuestResult::Failed);

			InternalDeactivateStageServices(OldStage);
			InternalDeactivateObjectives(OldStage);

			if (OnStageEnded.IsBound())
			{
				OnStageEnded.Broadcast(this, OldStage, OldStage->GetObjectives(), OldResult);
			}
		}
	}

	// Notify that active node changes to another
	if (OnActiveNodeChanged.IsBound())
	{
		OnActiveNodeChanged.Broadcast(this, Nodes[NewNodeIndex]);
	}

	// If the new node is stage node, notify that new stage has began
	if (Nodes.IsValidIndex(NewNodeIndex))
	{
		if (UQuestNode_Stage* NewStage = Cast<UQuestNode_Stage>(Nodes[NewNodeIndex]))
		{
			InternalActivateStageServices(NewStage);

			if (OnStageBegin.IsBound())
			{
				OnStageBegin.Broadcast(this, NewStage, NewStage->GetObjectives(), EQuestResult::InProgress);
			}
			InternalActivateObjectives(NewStage);
		}

		// Execute entry events
		Nodes[NewNodeIndex]->ExecuteEntryEvents(this, bWasStateRestored);
	}

	RefreshViewModel(true, false);
}

void UQuestContext::InternalActivateObjectives(UQuestNode_Stage* Node)
{
	check(Node);

	if (!HasAuthority())
	{
		return;
	}

	const TArray<UQuestObjective*>& Objectives = Node->GetObjectives();
	for (UQuestObjective* Objective : Objectives)
	{
		if (Objective->bAutoActivate)
		{
			InternalTryActivateObjective(Objective, Node);
		}
	}

	RefreshViewModel(false, true, false);
}

void UQuestContext::InternalDeactivateObjectives(UQuestNode_Stage* Node)
{
	check(Node);

	if (!HasAuthority())
	{
		return;
	}

	const TArray<UQuestObjective*>& Objectives = Node->GetObjectives();
	for (UQuestObjective* Objective : Objectives)
	{
		InternalDeactivateObjective(Objective);
	}

	RefreshViewModel(false, true, false);
}

bool UQuestContext::InternalTryActivateObjective(UQuestObjective* Objective, UQuestNode_Stage* Stage)
{
	check(Stage);
	check(Objective);

	if (Stage->GetObjectives().Contains(Objective))
	{
		if (!ActiveObjectives.Contains(Objective))
		{
			ActiveObjectives.Add(Objective);
			if (OnObjectiveActivated.IsBound())
			{
				OnObjectiveActivated.Broadcast(this, Objective);
			}
			RefreshObjectiveViewModel(Objective);
			return true;
		}
	}

	return false;
}

void UQuestContext::InternalDeactivateObjective(UQuestObjective* Objective)
{
	check(Objective);
	if (ActiveObjectives.Contains(Objective))
	{
		ActiveObjectives.Remove(Objective);
		if (OnObjectiveDeactivated.IsBound())
		{
			OnObjectiveDeactivated.Broadcast(this, Objective);
		}
	}
}

void UQuestContext::InternalActivateStageServices(UQuestNode_Stage* Node)
{
	check(Node);
	ensure(ActiveStageServices.Num() == 0);

	// Add non instanced services to the list as they are
	TArray<UQuestService*> NonInstanced = Node->GetNonInstancedServices();
	for (UQuestService* Service : NonInstanced)
	{
		ensure(Service);
		if (Service && ShouldExecuteNetworkAction(Service->GetNetworkExecutionType()))
		{
			ActiveStageServices.Add(Service);
		}
	}

	// Create instance for instanced services
	TArray<UQuestService*> InstancedServiceTemplates = Node->GetInstancedServices();
	for (UQuestService* ServiceTemplate : InstancedServiceTemplates)
	{
		if (!ensure(ServiceTemplate))
		{
			continue;
		}

		if (!ShouldExecuteNetworkAction(ServiceTemplate->GetNetworkExecutionType()))
		{
			return;
		}

		UClass* ServiceClass = ServiceTemplate->GetClass();
		if (!ensure(ServiceClass))
		{
			continue;
		}

		// Create a new service instance by copying the template
		UQuestService* Service = NewObject<UQuestService>(this, ServiceClass, NAME_None, RF_Transient, ServiceTemplate);
		check(Service);
		ActiveStageServices.Add(Service);
	}

	// Notify that services become active
	for (UQuestService* Service : ActiveStageServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyServiceBecomeActive(this);
		}
	}
}

void UQuestContext::InternalDeactivateStageServices(UQuestNode_Stage* Node)
{
	check(Node);

	// Notify that the services comes deactive
	for (UQuestService* Service : ActiveStageServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyServiceBecomeDeactive(this);
		}
	}

	// Just clear the active service list to destroy any instances
	ActiveStageServices.Empty();
}

void UQuestContext::InternalActivateContextServices()
{
	if (bContextQuestServicesActive)
	{
		return; // Already active
	}

	check(QuestAsset);
	ensure(ActiveContextServices.Num() == 0);

	// Add non instanced services to the list as they are
	TArray<UQuestService*> NonInstanced = QuestAsset->GetNonInstancedServices();
	for (UQuestService* Service : NonInstanced)
	{
		ensure(Service);
		if (Service && ShouldExecuteNetworkAction(Service->GetNetworkExecutionType()))
		{
			ActiveContextServices.Add(Service);
		}
	}

	// Create instance for instanced services
	TArray<UQuestService*> InstancedServiceTemplates = QuestAsset->GetInstancedServices();
	for (UQuestService* ServiceTemplate : InstancedServiceTemplates)
	{
		if (!ensure(ServiceTemplate))
		{
			continue;
		}

		if (!ShouldExecuteNetworkAction(ServiceTemplate->GetNetworkExecutionType()))
		{
			return;
		}

		UClass* ServiceClass = ServiceTemplate->GetClass();
		if (!ensure(ServiceClass))
		{
			continue;
		}

		// Create a new service instance by copying the template
		UQuestService* Service = NewObject<UQuestService>(this, ServiceClass, NAME_None, RF_Transient, ServiceTemplate);
		check(Service);
		ActiveContextServices.Add(Service);
	}

	// Notify that services become active
	for (UQuestService* Service : ActiveContextServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyServiceBecomeActive(this);
		}
	}

	bContextQuestServicesActive = true; // Activated services -> mark that those were activated, so we won't start them again
}

void UQuestContext::InternalDeactivateContextServices()
{
	// Notify that the services comes deactive
	for (UQuestService* Service : ActiveContextServices)
	{
		if (ensure(IsValid(Service)))
		{
			Service->NotifyServiceBecomeDeactive(this);
		}
	}

	// Just clear the active service list to destroy any instances
	ActiveContextServices.Empty();

	// No more active
	bContextQuestServicesActive = false;
}

UQuestComponent* UQuestContext::GetOwnerQuestComponent() const
{
	UQuestComponent* TypedOuterQuestComp = GetTypedOuter<UQuestComponent>();
	ensure(TypedOuterQuestComp);
	return TypedOuterQuestComp;
}

UQuestObjectiveViewModel* UQuestContext::GetObjectiveViewModel(UQuestObjective* Objective) const
{
	if (TObjectPtr<UQuestObjectiveViewModel> const* VM = ObjectiveViewModels.Find(Objective))
	{
		return *VM;
	}
	return nullptr;
}

void UQuestContext::RefreshViewModel(bool bUpdateQuestData, bool bUpdateObjectiveList, bool bRefreshObjectiveViewModels)
{
	if (!IsInitialized())
	{
		return;
	}

	if (ViewModel)
	{
		if (bUpdateQuestData)
		{
			check(QuestAsset);
			ViewModel->SetTitle(QuestAsset->GetDisplayName());
			ViewModel->SetQuestState(GetQuestState());
			if (UQuestNode* Node = GetActiveNode())
			{
				ViewModel->SetJournalText(Node->GetNodeText(this));
			}
		}

		if (bUpdateObjectiveList)
		{
			ViewModel->SetActiveObjectives(this, ActiveObjectives);
		}

		if (bRefreshObjectiveViewModels)
		{
			//TArray<const TObjectPtr<UQuestObjective>, TInlineAllocator<32>> Objectives;
		 // ObjectiveViewModels.GenerateKeyArray<TInlineAllocator<32>>(Objectives);
			//ObjectiveViewModels.GenerateKeyArray(Objectives);



			for(auto& It : ObjectiveViewModels)
			{
				UQuestObjective* Obj = It.Key;
				RefreshObjectiveViewModel(const_cast<UQuestObjective*>(Obj));
			}

			//for (const TObjectPtr<UQuestObjective> Objective : Objectives)
			//{
			//	UQuestObjective* Obj=Objective;
			//	RefreshObjectiveViewModel(const_cast<UQuestObjective*>(Obj));
			//}
		}
	}
}

void UQuestContext::RefreshObjectiveViewModel(UQuestObjective* Objective)
{
	if (!Objective)
	{
		return;
	}

	if (TObjectPtr<UQuestObjectiveViewModel> VM = GetObjectiveViewModel(Objective))
	{
		VM->SetIsOptional(Objective->Significancy == EQuestObjectiveSignificancy::Optional);
		VM->SetObjectiveState(GetObjectiveResult(Objective));
		VM->SetText(Objective->ShortDescription.GetFormattedText(this));
		VM->CustomUpdateViewModel(this, Objective);
	}
}

void UQuestContext::InitializeViewModels()
{
	if (!bViewModelsInitialized)
	{
		TSubclassOf<UQuestContextViewModel> ContextVMClass = UQuestContextViewModel::StaticClass();
		TSubclassOf<UQuestObjectiveViewModel> ObjectiveVMClass = UQuestObjectiveViewModel::StaticClass();

		const UQuestFrameworkSettings* Settings = GetDefault<UQuestFrameworkSettings>();
		if (Settings)
		{
			if (IsValid(Settings->QuestContextViewModelClass))
			{
				ContextVMClass = Settings->QuestContextViewModelClass;
			}

			if (IsValid(Settings->QuestObjectiveViewModelClass))
			{
				ObjectiveVMClass = Settings->QuestObjectiveViewModelClass;
			}
		}

		check(ContextVMClass);
		check(ObjectiveVMClass);

		// Quest context view model
		ViewModel = NewObject<UQuestContextViewModel>(this, ContextVMClass, NAME_None, RF_Transient);
		check(ViewModel);
		ViewModel->SetQuestContext(this);

		ObjectiveViewModels.Empty();
		check(QuestAsset);

		// Create one per each objective
		const TArray<UQuestNode*> Nodes = QuestAsset->GetQuestNodes();
		for (UQuestNode* Node : Nodes)
		{
			if (UQuestNode_Stage* StageNode = Cast<UQuestNode_Stage>(Node))
			{
				const TArray<TObjectPtr<UQuestObjective>> Objectives = StageNode->GetObjectives();
				for (const TObjectPtr<UQuestObjective> Objective : Objectives)
				{
					TObjectPtr<UQuestObjectiveViewModel> ObjectiveViewModel = NewObject<UQuestObjectiveViewModel>(this, ObjectiveVMClass, NAME_None, RF_Transient);
					check(ObjectiveViewModel);
					check(!ObjectiveViewModels.Contains(Objective));
					ObjectiveViewModels.Add(Objective, ObjectiveViewModel);
				}
			}
		}

		bViewModelsInitialized = true;

		UE_LOG(LogQuestFramework, Log, TEXT("%s initialized view models."), *GetFName().ToString());
	}
}

void UQuestContext::RefreshActiveObjectiveViewModels()
{
	if (!ViewModel)
	{
		return;
	}

	for (UQuestObjective* Objective : ActiveObjectives)
	{
		if (Objective)
		{
			RefreshObjectiveViewModel(Objective);
		}
	}
}

void UQuestContext::ConditionalReplicate()
{
	if (!HasAuthority())
	{
		return;
	}

	// Don't build rep data on standalone
	if (UKismetSystemLibrary::IsStandalone(this))
	{
		return;
	}

	// Write contents of memory block on server to memory replicator, so it gets send to the clients
	if (UQuestMemory* Memory = GetQuestMemory())
	{
		FQuestMemoryBlock& MemoryBlock = Memory->GetMemoryBlock();
		if (MemoryBlock.PollNetworkReplication())
		{
			MemoryReplicator.ReplicateMemoryBlock(MemoryBlock);
		}
	}

	// Replicate objective results
	ObjectiveResultReplicator.Replicate(LastObjectiveResults, false);
}

void UQuestContext::OnRep_QuestAsset()
{
	// Initialize context on remotes when quest asset is received
	if (GetAuthority() != EQuestNetworkAuthority::HasAuthority)
	{
		if (IsValid(QuestAsset) && !IsInitialized())
		{
			ClientHandleReceiveQuestAsset();
		}
	}
}

void UQuestContext::OnRep_QuestState()
{
	if (LastRepQuestStatePacked == QuestStatePacked)
	{
		return;
	}

	if (GetAuthority() != EQuestNetworkAuthority::HasAuthority)
	{
		if (IsInitialized() && bClientContextStarted)
		{
			bool bOldRestored;
			const EQuestResult OldState = UnpackRepStateValue<EQuestResult>(LastRepQuestStatePacked, bOldRestored);
			ClientHandleReceiveQuestState(OldState, GetQuestState(), IsQuestStateRestored());
		}
		else
		{
			LatentQuestState = FLatentRepState(LastRepQuestStatePacked, QuestStatePacked);
		}
	}

	LastRepQuestStatePacked = QuestStatePacked;
}

void UQuestContext::OnRep_CurrentNodeIndex()
{
	if (LastRepCurrentNodeIndexPacked == CurrentNodeIndexPacked)
	{
		return;
	}

	if (GetAuthority() != EQuestNetworkAuthority::HasAuthority)
	{
		if (IsInitialized() && bClientContextStarted)
		{
			bool bOldRestored;
			const int32 OldIndex = UnpackRepStateValue<int32>(LastRepCurrentNodeIndexPacked, bOldRestored);

			ClientHandleNodeIndexChanged(OldIndex, GetActiveNodeIndex(), IsActiveNodeIndexRestored());
		}
		else
		{
			LatentNodeIndex = FLatentRepState(LastRepCurrentNodeIndexPacked, CurrentNodeIndexPacked);
		}
	}

	LastRepCurrentNodeIndexPacked = CurrentNodeIndexPacked;
}

void UQuestContext::OnRep_ActiveObjectives(TArray<UQuestObjective*>& OldObjectives)
{
	if (!HasAuthority() && IsInitialized())
	{
		// Find new and removed
		TArray<UQuestObjective*, TInlineAllocator<8>> Added;
		TArray<UQuestObjective*, TInlineAllocator<8>> Removed;

		// Find added
		for (UQuestObjective* Objective : ActiveObjectives)
		{
			ensure(Objective);
			if (!OldObjectives.Contains(Objective))
			{
				Added.Add(Objective);
			}
		}

		// Find removed
		for (UQuestObjective* Objective : OldObjectives)
		{
			ensure(Objective);
			if (!ActiveObjectives.Contains(Objective))
			{
				Removed.Add(Objective);
			}
		}

		// Broadcast events on clients
		for (UQuestObjective* Objective : Added)
		{
			if (Objective)
			{
				OnObjectiveActivated.Broadcast(this, Objective);
			}
		}
		for (UQuestObjective* Objective : Removed)
		{
			if (Objective)
			{
				OnObjectiveDeactivated.Broadcast(this, Objective);
			}
		}

		// Refresh view model if objectives were added or removed
		if (Added.Num() > 0 || Removed.Num() > 0)
		{
			RefreshViewModel();
		}
	}
}

void UQuestContext::OnRep_MemoryReplicator()
{
	ClientHandleMemoryReplication();
}

void UQuestContext::OnRep_ObjectiveResultReplicator()
{
	if (!HasAuthority())
	{
		ObjectiveResultReplicator.Reconstruct(LastObjectiveResults, [this](TObjectPtr<UQuestObjective> Objective, EQuestResult OldResult, EQuestResult NewResult)
			{
				if (IsValid(Objective))
				{
					if (OnObjectiveResultChanged.IsBound())
					{
						OnObjectiveResultChanged.Broadcast(this, Objective, NewResult);
					}

					// Refresh view model of the objective
					RefreshObjectiveViewModel(Objective);
				}
			});
	}
}

void UQuestContext::ClientStartContext()
{
	if (HasAuthority())
	{
		return;
	}

	if (!bClientContextStarted)
	{
		bClientContextStarted = true;

		// Handle state change that was received before quest asset was valid
		if (LatentQuestState.IsSet())
		{
			uint64 OldRaw = LatentQuestState.GetValue().OldState;
			uint64 NewRaw = LatentQuestState.GetValue().NewState;
			LatentQuestState.Reset();

			bool bOldRestored;
			const EQuestResult OldState = UnpackRepStateValue<EQuestResult>(OldRaw, bOldRestored);

			bool bNewRestored;
			const EQuestResult NewState = UnpackRepStateValue<EQuestResult>(NewRaw, bNewRestored);

			ClientHandleReceiveQuestState(OldState, NewState, bNewRestored);
		}

		// Handle node change that was received before quest asset was valid
		if (LatentNodeIndex.IsSet())
		{
			uint64 OldRaw = LatentNodeIndex.GetValue().OldState;
			uint64 NewRaw = LatentNodeIndex.GetValue().NewState;
			LatentNodeIndex.Reset();

			bool bOldRestored;
			const int32 OldIndex = UnpackRepStateValue<int32>(OldRaw, bOldRestored);

			bool bNewRestored;
			const int32 NewIndex = UnpackRepStateValue<int32>(NewRaw, bNewRestored);

			ClientHandleNodeIndexChanged(OldIndex, NewIndex, bNewRestored);
		}

		ClientHandleMemoryReplication();

		RefreshViewModel(true, true);
	}
}

void UQuestContext::Clear()
{
	ViewModel=nullptr;
	ObjectiveViewModels.Empty();
}

void UQuestContext::ClientHandleReceiveQuestAsset()
{
	check(QuestAsset);
	if (!HasAuthority())
	{
		if (!IsInitialized())
		{
			InitializeContext(QuestAsset);
		}
	}
}

void UQuestContext::ClientHandleReceiveQuestState(EQuestResult OldState, EQuestResult NewState, bool bRestoredState)
{
	// Call quest start / end events if the state has changed and not restored (loaded from save state)
	if (OldState != NewState)
	{
		if (!bRestoredState)
		{
			if (OldState == EQuestResult::NotStarted && NewState == EQuestResult::InProgress) // Quest started
			{
				if (OnQuestStarted.IsBound())
				{
					OnQuestStarted.Broadcast(this);
				}
				InternalActivateContextServices(); // Activate context wide service on on the client
			}
			else if (OldState == EQuestResult::InProgress && (NewState == EQuestResult::Completed || NewState == EQuestResult::Failed)) // Quest ended
			{
				InternalDeactivateContextServices(); // Deactivate context wide services on the client
				if (OnQuestEnded.IsBound())
				{
					OnQuestEnded.Broadcast(this);
				}
			}
		}
	}

	RefreshViewModel(true, false);
	SetRequiresUpdatePipeline(NewState == EQuestResult::InProgress);
}

void UQuestContext::ClientHandleNodeIndexChanged(int32 OldIndex, int32 NewIndex, bool bRestoredState)
{
	// Handle change in node index. Don't run events on client if the state was restored (loaded from save state)
	if (OldIndex != NewIndex)
	{
		InternalOnQuestNodeChange(OldIndex, NewIndex, EQuestResult::Completed, bRestoredState);
	}
	RefreshViewModel(true, false);
}

void UQuestContext::ClientHandleMemoryReplication()
{
	// Read replicated values from server to client's memory block
	if (!HasAuthority() && bClientContextStarted)
	{
		if (UQuestMemory* Memory = GetQuestMemory())
		{
			FQuestMemoryBlock& MemoryBlock = Memory->GetMemoryBlock();
			MemoryReplicator.ReadToMemoryBlock(MemoryBlock);
		}
	}
}


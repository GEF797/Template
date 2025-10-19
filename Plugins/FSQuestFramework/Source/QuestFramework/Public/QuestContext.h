// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include "QuestObjectBase.h"

#include "QuestData.h"
#include "Quest.h"
#include "QuestMemory.h"

#include "Nodes/QuestNode.h"
#include "QuestObjects/QuestObjective.h"
#include "QuestObjects/QuestService.h"

#include "QuestContext.generated.h"

class UQuestNode_Root;
class UQuestNode_Selector;
class UQuestNode_Stage;
class UQuestNode_End;
class UQuestComponent;

class UQuestContextViewModel;
class UQuestObjectiveViewModel;

// ---------------------------------- //

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestContextNotifyDelegate, class UQuestContext*, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuestContextNodeDelegate, class UQuestContext*, Context, UQuestNode*, NewNode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FQuestContextStageDelegate, class UQuestContext*, Context, UQuestNode_Stage*, StageNode, TArray<UQuestObjective*>, Objectives, EQuestResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuestContextObjectiveDelegate, class UQuestContext*, Context, UQuestObjective*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuestContextObjectiveResultDelegate, class UQuestContext*, Context, UQuestObjective*, Objective, EQuestResult, Result);

// ---------------------------------- //


/**
 * Runtime version of quest.
 */
UCLASS(NotBlueprintable, BlueprintType, Config = Game)
class QUESTFRAMEWORK_API UQuestContext : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	UQuestContext();

public: // Replication

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Helper template to pack replication state variable to uint64 (contains data in most significant 32-bit and boolean if the data was restored in least significant 32-bit)
	 */
	template<typename TStoreType>
	uint64 PackRepStateValue(TStoreType InValue, bool bRestored) const
	{
		int32 RawVal = (int32)InValue;
		uint64 PackVal = RawVal >= 0 ? (uint64)RawVal : QUEST_INDEX_NONE;
		uint64 RestoreVal = bRestored ? 1 : 0;
		return PackVal << 32 | RestoreVal;
	}

	/**
	 * Helper template to unpack replication state variable from uint64 (contains data in most significant 32-bit and boolean if the data was restored in least significant 32-bit)
	 */
	template<typename TStoreType>
	TStoreType UnpackRepStateValue(uint64 InRaw, bool& bOutRestored) const
	{
		uint32 RestoreVal = InRaw & 0xffffffff;
		uint32 PackVal = InRaw >> 32;
		bOutRestored = RestoreVal > 0;
		return static_cast<TStoreType>(PackVal);
	}

	/**
	 * Returns owning actor of the object for network replication purposes
	 */
	AActor* GetOwningActor() const
	{
		return GetTypedOuter<AActor>();
	}

	/**
	 * Returns network authority level of the context on executing machine
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	EQuestNetworkAuthority GetAuthority() const
	{
		AActor* Actor = GetOwningActor();
		if (!Actor)
		{
			return EQuestNetworkAuthority::None;
		}
		return Actor->HasAuthority() ? EQuestNetworkAuthority::HasAuthority : EQuestNetworkAuthority::Remote;
	}

	/**
	 * Checks if the quest context has network authority (server or client)
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	bool HasAuthority() const
	{
		return GetAuthority() == EQuestNetworkAuthority::HasAuthority;
	}

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	bool ShouldExecuteNetworkAction(EQuestNetworkExecType ExecType) const;

public:

	/**
	 * Initializes context from quest asset.
	 * This should be called before restoring state or starting the quest
	 */
	void InitializeContext(UQuest* Quest);

	/**
	 * Uninitializes the context. Shuts down any services
	 */
	void Uninitialize();

	/**
	 * Return true if this is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	bool IsInitialized() const { return bInitialized; }

	/**
	 * Returns true if this context needs update
	 */
	bool ShouldUpdate() const { return bInitialized && bShouldUpdateContext; }

	/**
	 * Called per frame to tick services
	 */
	void TickActiveServices(float DeltaTime);

	/**
	 * Restores saved state of the quest.
	 * If quest was already started, the context is added to update pipeline
	 */
	void RestoreState(const FGuid& InGuid, int32 SavedNodeIndex, EQuestResult SavedQuestState, const FQuestMemoryBlock& SavedMemoryBlock);

	/**
	 * Starts this quest context if QuestState is "NotStarted".
	 * Adds the context into update pipeline
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	void Start();

	/**
	 * Runs evaluation of the context.
	 * Includes checking objectives, entering into next nodes or ending the quest
	 * Called once per frame automatically when quest is in update pipeline
	 */
	void EvaluateContext();

	/**
	 * Builds replication data of quest memory and objective results on server if the memory has changed
	 */
	void ConditionalReplicate();

	/**
	 * Returns unique identifier of this quest context.
	 * This is not same as quest guid but each context has its unique guid
	 * Saved to save game to guarantee persistent uniquenes
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	FGuid GetGuid() const { return ContextGuid; }

	/**
	 * Returns current state of the quest
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	EQuestResult GetQuestState() const
	{
		bool bRestored;
		return UnpackRepStateValue<EQuestResult>(QuestStatePacked, bRestored);
	}

	/**
	 * Checks if quest have ended (either completed or failed)
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	bool HasEnded() const { return GetQuestState() == EQuestResult::Completed || GetQuestState() == EQuestResult::Failed; }

	/**
	 * Returns quest asset associated with this context
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	UQuest* GetQuestAsset() const { return QuestAsset; }

	/**
	 * Returns memory object of this quest context.
	 * Memory can be used to store values which affect the quest flow and objectives
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	UQuestMemory* GetQuestMemory() const { return ContextMemory; }

	/**
	 * Returns currently active node. Null if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	UQuestNode* GetActiveNode() const;

	/**
	 * Returns currently active node casted to a type. Null if not found
	 */
	template<class TNodeClass>
	TNodeClass* GetActiveNodeUnchecked() const
	{
		return Cast<TNodeClass>(GetActiveNode());
	}

	/**
	 * Returns index of active node
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	int32 GetActiveNodeIndex() const 
	{
		bool bRestored;
		return UnpackRepStateValue<int32>(CurrentNodeIndexPacked, bRestored);
	}

	/**
	 * Returns list of currently active objectives (only active)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Context")
	void GetActiveObjectives(TArray<UQuestObjective*>& Objectives) const;

	/**
	 * Returns list of current objectives (also non-active)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Context")
	void GetCurrentObjectives(TArray<UQuestObjective*>& Objectives) const;

	/**
	 * Checks if given objective is active in this context
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	bool IsObjectiveActive(UQuestObjective* Objective) const;

	/**
	 * Activates objective if context is in stage where the objective is.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	bool TryActivateObjective(UQuestObjective* Objective);

	/**
	 * Deactivates objective if it is active
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	void DeactivateObjective(UQuestObjective* Objective);

	/**
	 * Returns result of given objective.
	 * If bReEvaluate is set to false, last result is used, if true evaluation is done again
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Context")
	EQuestResult GetObjectiveResult(UQuestObjective* Objective, bool bReEvaluate = false) const;

	/**
	 * Attempts to jump into given stage node.
	 * Returns true if the active node was changed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	bool JumpToStageNode(const UQuestNode_Stage* StageNode);

	/**
	 * Attempts to jump into given stage by name.
	 * Returns true if the active node was changed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	bool JumpToStageByName(FName StageName);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Context")
	bool EndQuest(bool Completed);

public:

	/**
	 * Called when quest context starts
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextNotifyDelegate OnQuestStarted;

	/**
	 * Called when quest context has ended.
	 * Result of the quest can be read by using GetQuestState
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextNotifyDelegate OnQuestEnded;


	/**
	 * Called when active node changed to a new one
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextNodeDelegate OnActiveNodeChanged;

	/**
	 * Called when a new quest stage begins
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextStageDelegate OnStageBegin;

	/**
	 * Called when a quest stage ended to either completion or failure
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextStageDelegate OnStageEnded;

	/**
	 * Called when objective comes active
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextObjectiveDelegate OnObjectiveActivated;

	/**
	 * Called when objective comes deactive
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextObjectiveDelegate OnObjectiveDeactivated;

	/**
	 * Called when result of an objective changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Context")
	FQuestContextObjectiveResultDelegate OnObjectiveResultChanged;

public:

	/**
	 * Returns actor that owns this quest context
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	AActor* GetOwnerActor() const;

	/**
	 * Returns controller that own this quest context
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	AController* GetOwnerController() const;

	/**
	 * Returns owning quest component
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context")
	UQuestComponent* GetOwnerQuestComponent() const;

private:

	// Is context initialized?
	bool bInitialized = false;

	// Unique identifier of the context, saved to save state to identify uniqueness
	UPROPERTY(Transient, Replicated)
	FGuid ContextGuid = FGuid();

	// State of the quest.
	UPROPERTY(Transient, ReplicatedUsing="OnRep_QuestState")
	uint64 QuestStatePacked;

	// Node where this context currently is in the quest graph. (index of node array in QuestAsset)
	UPROPERTY(Transient, ReplicatedUsing="OnRep_CurrentNodeIndex")
	uint64 CurrentNodeIndexPacked;

	/**
	 * Quest asset this context represents
	 */
	UPROPERTY(Transient, ReplicatedUsing="OnRep_QuestAsset")
	TObjectPtr<UQuest> QuestAsset;

	/**
	 * Memory for this context.
	 * Used to write/read variables
	 */
	UPROPERTY(Transient)
	TObjectPtr<UQuestMemory> ContextMemory;

	/**
	 * List of currently active objectives
	 */
	UPROPERTY(Transient, ReplicatedUsing="OnRep_ActiveObjectives")
	TArray<TObjectPtr<UQuestObjective>> ActiveObjectives;

	/**
	 * Services that are currently active for stage node
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestService>> ActiveStageServices;

	/**
	 * Services that are currently active for the whole quest context
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestService>> ActiveContextServices;

	/**
	 * results of objectives since last evaluation
	 */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UQuestObjective>, EQuestResult> LastObjectiveResults;

private: // Internal

	bool IsQuestStateRestored() const
	{
		bool bRestored = false;
		UnpackRepStateValue<EQuestResult>(QuestStatePacked, bRestored);
		return bRestored;
	}

	void SetQuestState(EQuestResult InNewState, bool bRestored)
	{
		QuestStatePacked = PackRepStateValue<EQuestResult>(InNewState, bRestored);
	}

	bool IsActiveNodeIndexRestored() const
	{
		bool bRestored = false;
		UnpackRepStateValue<int32>(CurrentNodeIndexPacked, bRestored);
		return bRestored;
	}

	void SetActiveNodeIndex(int32 NewIndex, bool bRestored)
	{
		CurrentNodeIndexPacked = PackRepStateValue<int32>(NewIndex, bRestored);
	}

	int32 InternalEvalNode_Root(const UQuestNode_Root* Node);

	int32 InternalEvalNode_Selector(const UQuestNode_Selector* Node);

	int32 InternalEvalNode_Stage(const UQuestNode_Stage* Node, EQuestResult& OutResult);

	bool InternalEvalNode_End(const UQuestNode_End* Node, EQuestResult& OutResult);

	void SetRequiresUpdatePipeline(bool bShouldUpdate);
	bool bShouldUpdateContext = false;

	void InternalStartQuest(bool bSilent = false);

	void InternalEndQuest(EQuestResult Result);

	void InternalOnQuestNodeChange(int32 OldNodeIndex, int32 NewNodeIndex, EQuestResult OldResult, bool bWasStateRestored);

	void InternalActivateObjectives(UQuestNode_Stage* Node);

	void InternalDeactivateObjectives(UQuestNode_Stage* Node);

	bool InternalTryActivateObjective(UQuestObjective* Objective, UQuestNode_Stage* Stage);

	void InternalDeactivateObjective(UQuestObjective* Objective);

	void InternalActivateStageServices(UQuestNode_Stage* Node);

	void InternalDeactivateStageServices(UQuestNode_Stage* Node);

	void InternalActivateContextServices();
	void InternalDeactivateContextServices();
	bool bContextQuestServicesActive = false;

public: // View models

	/**
	 * Returns view model of the quest context to represents UI data of it.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context | View Model")
	UQuestContextViewModel* GetViewModel() const { return ViewModel; }

	/**
	 * Returns view model of an objective.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Context | View Model")
	UQuestObjectiveViewModel* GetObjectiveViewModel(UQuestObjective* Objective) const;

	// Refreshes view model of the quest context
	UFUNCTION(BlueprintCallable, Category = "Quest Context | View Model")
	void RefreshViewModel(bool bUpdateQuestData = true, bool bUpdateObjectiveList = true, bool bRefreshObjectiveViewModels = false);

	// Refreshes view model of given objective
	UFUNCTION(BlueprintCallable, Category = "Quest Context | View Model")
	void RefreshObjectiveViewModel(UQuestObjective* Objective);

private:

	/**
	 * View model that represents this quest context in UI
	 */
	UPROPERTY(Transient)
	TObjectPtr<UQuestContextViewModel> ViewModel;

	/**
	 * View models of objectives to represent them in UI
	 */
	UPROPERTY(Transient)
	TMap<const TObjectPtr<UQuestObjective>, TObjectPtr<UQuestObjectiveViewModel>> ObjectiveViewModels;

	// Are view models initialized or not?
	bool bViewModelsInitialized = false;

	// Initializes view models, so UI widgets can bind to them
	void InitializeViewModels();

	// Called per tick to refresh view models of active objectives
	void RefreshActiveObjectiveViewModels();

private: // Replication

	// Replication of quest memory block
	UPROPERTY(Transient, ReplicatedUsing="OnRep_MemoryReplicator")
	FQuestMemoryBlockReplicator MemoryReplicator;

	UPROPERTY(Transient, ReplicatedUsing = "OnRep_ObjectiveResultReplicator")
	FQuestObjectiveResultReplicator ObjectiveResultReplicator;

	// Values of packed state and nodes since last replication
	uint64 LastRepQuestStatePacked = 0;
	uint64 LastRepCurrentNodeIndexPacked = 0;

	UFUNCTION()
	void OnRep_QuestAsset();

	UFUNCTION()
	void OnRep_QuestState();

	UFUNCTION()
	void OnRep_CurrentNodeIndex();

	UFUNCTION()
	void OnRep_ActiveObjectives(TArray<UQuestObjective*>& OldObjectives);

	UFUNCTION()
	void OnRep_MemoryReplicator();

	UFUNCTION()
	void OnRep_ObjectiveResultReplicator();

	struct FLatentRepState
	{
		uint64 OldState;
		uint64 NewState;
		FLatentRepState()
		{
			OldState = 0;
			NewState = 0;
		}
		FLatentRepState(const uint64 InOld, const uint64 InNew)
		{
			OldState = InOld;
			NewState = InNew;
		}
	};

	bool bClientContextStarted = false;
	TOptional<FLatentRepState> LatentQuestState;
	TOptional<FLatentRepState> LatentNodeIndex;

	void ClientHandleReceiveQuestAsset();
	void ClientHandleReceiveQuestState(EQuestResult OldState, EQuestResult NewState, bool bRestoredState);
	void ClientHandleNodeIndexChanged(int32 OldIndex, int32 NewIndex, bool bRestoredState);
	void ClientHandleMemoryReplication();

public:

	/**
	* Called on clients just after the context has been replicated to set initial state and node index, so the events will fire
	*/
	void ClientStartContext();

	//UFUNCTION(BlueprintCallable, Category = "Quest List View Model")
	void Clear();

};

// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "Components/ActorComponent.h"

#include "QuestMemory.h"
#include "Quest.h"
#include "QuestContext.h"
#include "QuestSaveState.h"

#include "QuestComponent.generated.h"

class UQuestComponent;

// Event called when quest context is created or suspended
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuestComponentContextDelegate, UQuestComponent*, QuestComponent, UQuestContext*, Context);

/**
 * Component that hosts active quest contexts of player(s)
 * This component is added either to player controller, game state or player pawn.
 * 
 * Config A (Player Controller):
 * Each player has their own quests, only server and owning client can access the quest contexts.
 * 
 * Config B (Game State):
 * Quests are shared with the players, server and all connected clients can access the quest contexts.
 * 
 * Config C (Player Pawn or Player State):
 * Each player has their own quests, server and all clients can access the quest contexts of any player.
 * 
 * Config D (Game Mode):
 * Quests are shared with the players, only server can access quest contexts.
 * 
 */
UCLASS( ClassGroup=("Quest Framework"), meta=(BlueprintSpawnableComponent, DisplayName = "Quest Component"))
class QUESTFRAMEWORK_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UQuestComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

	void InternalEvaluate();
	void InternalTickServices(float DeltaTime);
	void InternalReplicateContexts();

public:

	/**
	 * Saves state of the quest contexts into save state object.
	 * Outputs struct of saved data. Store the struct into save game object of your game.
	 * Quest states can be restored by providing the data for load function
	 * Returns true if save data was written
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Framework")
	bool SaveQuestStates(FQuestSaveStateData& SaveData);

	/**
	 * Restores state of quests where they were when they were saved
	 * WARNING! This operation resets current state of the quest system before restore
	 * Returns true if state was restored
	 * If bForceRestore is set to true, state is restored desipite of save state being incomplete.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Framework")
	bool LoadQuestStates(const FQuestSaveStateData& SaveData, bool bForceRestore);

	/**
	 * Creates a new quest context from quest asset
	 * Returns quest context that holds runtime information of the quest
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Framework")
	UQuestContext* CreateQuestContext(UQuest* Quest, bool bStart = true);

	/**
	 * Suspends given quest context meaning that it will be removed from the manager.
	 * NOTE: Usually you want to suspend only ended quests.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Framework")
	void SuspendQuest(UQuestContext* QuestContext);

	/**
	 * Suspends all active quest contexts
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest Framework")
	void SuspendAllQuests();

	/**
	 * Returns list of all active quest contexts.
	 * NOTE! This is list of ALL started contexts (ended quest that are not yet suspended are in this list).
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Framework")
	void GetAllActiveQuestContexts(TArray<UQuestContext*>& OutContexts) const;

	/**
	 * Finds all active contexts that match given quest asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Framework")
	void FindAllActiveQuestContexts(UQuest* Quest, TArray<UQuestContext*>& OutContexts) const;

	/**
	 * Finds first quest context that match given quest asset.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Framework")
	UQuestContext* FindFirstActiveQuestContext(UQuest* Quest) const;

	/**
	 * Returns true if the component has quest context for given quest asset
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Framework")
	bool HasActiveQuestContext(UQuest* Quest) const;

public: // Events

	/**
	 * Called right after a new quest context is created.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Framework")
	FQuestComponentContextDelegate OnQuestContextCreated;

	/**
	 * Called right before quest context is suspended
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Framework")
	FQuestComponentContextDelegate OnQuestContextSuspend;

	UFUNCTION(BlueprintCallable, Category = "Quest Framework")
	void Clear();

private:

	// List of active quest contexts owned by this component
	UPROPERTY(Transient, ReplicatedUsing="OnRep_ActiveQuestContexts")
	TArray<TObjectPtr<UQuestContext>> ActiveQuestContexts;

	// List of contexts that are being waited to be initialized on client
	TArray<TWeakObjectPtr<UQuestContext>, TInlineAllocator<8>> ClientLatentContexts;

	UFUNCTION()
	void OnRep_ActiveQuestContexts(const TArray<UQuestContext*>& OldActiveContexts);

	// Calls OnQuestContextCreated on clients for contexts in ClientLatentContexts when they are initialized
	void ClientStartLatentContexts();
};

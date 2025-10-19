// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestTrackerComponent.generated.h"

class UQuestComponent;
class UQuestContext;
class UQuestTrackerViewModel;
class UQuestListViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestContextTrackDelegate, UQuestContext*, Context);

/**
* Component to track quests of a player.
* Add this to player controller to enable the tracking feature.
*/
UCLASS( ClassGroup=("Quest Framework"), meta=(BlueprintSpawnableComponent), Config = Game)
class QUESTFRAMEWORK_API UQuestTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UQuestTrackerComponent();

	virtual void InitializeComponent() override;

	virtual void UninitializeComponent() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/**
	 * Should the owning player quest component added automatically as quest source on component initialization?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Tracker")
	bool bAddPlayerAsSourceOnInit = true;

	/**
	 * When enabled, only one quest can be tracked at time.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Tracker")
	bool bTrackOnlySingle = true;

	/**
	 * Should newly added quests automatically tracked?
	 * If bTrackOnlySingle is set to true, new quest is only tracked if no quest is being tracked currently.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Tracker")
	bool bAutoTrackNewQuests = true;

public:

	/**
	 * Adds a new quest component as source to track quest contexts from
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void AddSourceQuestComponent(UQuestComponent* QuestComponent);

	/**
	 * Removes quest component source.
	 * Stops tracking any quest contexts of the source
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void RemoveSourceQuestComponent(UQuestComponent* QuestComponent);

	/**
	 * Starts to track given quest context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void TrackQuest(UQuestContext* QuestContext);

	/**
	 * Stops tracking of given quest context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void StopTrackQuest(UQuestContext* QuestContext);

	/**
	 * Returns all active quest contexts from source quest components
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void GetAllQuestContexts(TArray<UQuestContext*>& OutContexts) const;

	/**
	 * Returns list of currently tracked quest contexts
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void GetTrackedQuestContexts(TArray<UQuestContext*>& OutContexts) const;

	/**
	 * Returns view model to represent tracked quests in UI
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Tracker", BlueprintCosmetic)
	UQuestTrackerViewModel* GetViewModel() const { return ViewModel; }

	/**
	 * Returns view model to represent all quests the player has in UI
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Tracker", BlueprintCosmetic)
	UQuestListViewModel* GetListViewModel() const { return ListViewModel; }

	/*
	* Refreshes tracking state of quests to view models.
	* Called automatically OnBeginTrackQuest and OnStopTrackQuest
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest Tracker", BlueprintCosmetic)
	void RefreshViewModel(bool bTrackerViewModel = true, bool bQuestListViewModel = true);

public:

	/**
	 * Called when a certain quest is started to be tracked
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Tracker")
	FQuestContextTrackDelegate OnBeginTrackQuest;

	/**
	 * Called when a certain quest is no longer tracked
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Tracker")
	FQuestContextTrackDelegate OnStopTrackQuest;

private:

	// View model to represent tracked quests in UI
	UPROPERTY(Transient)
	TObjectPtr<UQuestTrackerViewModel> ViewModel;

	// View model to represent list of all quests player has
	UPROPERTY(Transient)
	TObjectPtr<UQuestListViewModel> ListViewModel;

	// Quest components that acts as source of quest contexts that can be tracked by the player
	TArray<TWeakObjectPtr<UQuestComponent>> SourceQuestComponents;

	// List of quests contexts that are being tracked currently
	TArray<TWeakObjectPtr<UQuestContext>> TrackedQuestContexts;

	void OnQuestComponentAdded(UQuestComponent* Comp);
	void OnQuestComponentRemoved(UQuestComponent* Comp);

	void AutoTrackNextQuest(UQuestContext* IgnoreContext = nullptr);

	// Checks if the component is owned by locally controlled player controller
	bool IsLocallyControlled() const;

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void Clear();

protected: // Delegate binds

	UFUNCTION()
	void HandleOnQuestContextCreated(UQuestComponent* QuestComponent, UQuestContext* Context);

	UFUNCTION()
	void HandleOnQuestContextSuspended(UQuestComponent* QuestComponent, UQuestContext* Context);

	UFUNCTION()
	void HandleOnQuestStarted(UQuestContext* Context);

	UFUNCTION()
	void HandleOnQuestEnded(UQuestContext* Context);
};

// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Styling/SlateColor.h"

#include "GameFramework/PlayerController.h"

#include "Quest.h"
#include "QuestContext.h"
#include "QuestMessageInterface.h"
#include "QuestComponentInterface.h"
#include "QuestSaveState.h"

#include "QuestUtilityLibrary.generated.h"

class UQuestComponent;
class UQuestTrackerComponent;

class UQuestContextViewModel;
class UQuestObjectiveViewModel;
class UQuestTrackerViewModel;
class UQuestListViewModel;

/**
 * Blueprint utility library for quest framework
 */
UCLASS()
class QUESTFRAMEWORK_API UQuestUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/**
	 * Returns quest component of given actor.
	 * If provided actor is pawn, possessing controller is used to look for component.
	 * If bLookForComponent is true, component search is performed, otherwise only component interface is used
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestComponent* GetQuestComponentFromActor(AActor* Actor, bool bLookForComponent = true);

	/**
	 * Searches quest component that is relevant for given actor.
	 * If pawn is passed, tries to find from controller
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestComponent* FindRelevantQuestComponentFromActor(AActor* Actor);

	/**
	 * Return quest tracker component of given actor.
	 * If provided actor is pawn, possessing controller is used to look for component.
	 * If bLookForComponent is true, component search is performed, otherwise only component interface is used
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestTrackerComponent* GetQuestTrackerComponentFromActor(AActor* Actor, bool bLookForComponent = true);

	/**
	 * Seeks quest component from game mode
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility", meta = (WorldContext = "WorldContext"))
	static UQuestComponent* GetGameModeQuestComponent(const UObject* WorldContext);

	/**
	 * Seeks quest component from game state
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility", meta = (WorldContext = "WorldContext"))
	static UQuestComponent* GetGameStateQuestComponent(const UObject* WorldContext);

	/**
	 * Seeks for quest component of given player
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestComponent* GetPlayerQuestComponent(APlayerController* PlayerController);

	/**
	 * Seeks for quest tracker component of given player
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestTrackerComponent* GetPlayerQuestTrackerComponent(APlayerController* PlayerController);

	/**
	 * Finds view model to represent UI data of tracked quests of the player
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestTrackerViewModel* GetPlayerQuestTrackerViewModel(APlayerController* PlayerController);

	/**
	 * Finds view model to represent UI data of quests of the player
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static UQuestListViewModel* GetPlayerQuestListViewModel(APlayerController* PlayerController);

	/**
	 * Seeks quest component that is shared between all players.
	 * Searches first from game state and then from game mode
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility", meta = (WorldContext = "WorldContext"))
	static UQuestComponent* GetSharedQuestComponent(const UObject* WorldContext);

	/**
	 * Sends quest message to all listeners in all quest contexts
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility", meta = (WorldContext = "WorldContext", GameplayTagFilter = "QuestMessage"))
	static void BroadcastQuestMessage(const UObject* WorldContext, const FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Sender = nullptr);

	/**
	 * Sends quest message to all listeners owned by Recipient actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility", meta = (WorldContext = "WorldContext", GameplayTagFilter = "QuestMessage"))
	static void SendQuestMessage(const UObject* WorldContext, const FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Recipient, AActor* Sender = nullptr);

public: // Variable access

	/**
	 * Returns value of variable from given quest context as integer
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static int32 GetQuestVariableValueAsInteger(UQuestContext* Context, const FQuestVariable& Variable);

	/**
	 * Returns value of variable from given quest context as integer
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static float GetQuestVariableValueAsFloat(UQuestContext* Context, const FQuestVariable& Variable);

	/**
	 * Returns value of variable from given quest context as integer
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static bool GetQuestVariableValueAsBool(UQuestContext* Context, const FQuestVariable& Variable);

	/**
	 * Sets value of quest variable in given quest context
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static void SetQuestVariableAsInteger(UQuestContext* Context, const FQuestVariable& Variable, int32 NewValue);

	/**
	 * Sets value of quest variable in given quest context
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static void SetQuestVariableAsFloat(UQuestContext* Context, const FQuestVariable& Variable, float NewValue);

	/**
	 * Sets value of quest variable in given quest context
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static void SetQuestVariableAsBool(UQuestContext* Context, const FQuestVariable& Variable, bool NewValue);

	/**
	 * Converts quest text to display text format.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static FText FormatQuestText(const FQuestText& Text, UQuestContext* Context);

public: // Widget helpers

	/**
	 * Helper to convert quest result enum to color in view model bindings.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static FLinearColor ConvertQuestResultToColor(EQuestResult InResult, FLinearColor NotStarted, FLinearColor InProgress, FLinearColor Completed, FLinearColor Failed);

	/**
	 * Helper to convert quest result enum to boolean in view model bindings.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static bool ConvertQuestResultToBool(EQuestResult InResult, bool NotStarted, bool InProgress, bool Completed, bool Failed);

	/**
	 * Helper to convert quest result enum to text in view model bindings.
	 */
	UFUNCTION(BlueprintPure, Category = "Quest Utility")
	static FText ConvertQuestResultToText(EQuestResult InResult, FText NotStarted, FText InProgress, FText Completed, FText Failed);

	/**
	 * Helper to filter objective view models based on old and new array.
	 * All entries that exists in both input arrays are skipped.
	 * All entries that exists in old list but not in new list are added to OutRemoved.
	 * All entries that exists in new array but not in old array are added to OutAdded.
	 * Check FilterAddedRemoved-template function in QuestUtilityLibrary.h
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static void FilterChangedObjectiveViewModels(const TArray<UQuestObjectiveViewModel*>& OldList, const TArray<UQuestObjectiveViewModel*>& NewList, TArray<UQuestObjectiveViewModel*>& OutAdded, TArray<UQuestObjectiveViewModel*>& OutRemoved);

	/**
	 * Helper to filter quest context view models based on old and new array.
	 * All entries that exists in both input arrays are skipped.
	 * All entries that exists in old list but not in new list are added to OutRemoved.
	 * All entries that exists in new array but not in old array are added to OutAdded.
	 * Check FilterAddedRemoved-template function in QuestUtilityLibrary.h
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Utility")
	static void FilterChangedQuestContextViewModels(const TArray<UQuestContextViewModel*>& OldList, const TArray<UQuestContextViewModel*>& NewList, TArray<UQuestContextViewModel*>& OutAdded, TArray<UQuestContextViewModel*>& OutRemoved);

private:

	/*
	* Helper template to filter entries that were added and entries that were removed due array update by providing old array and new array.
	* All entries that exists in both input arrays are skipped.
	* All entries that exists in old list but not in new list are added to OutRemoved.
	* All entries that exists in new array but not in old array are added to OutAdded.
	*/
	template<typename TElementType>
	static void FilterAddedRemoved(const TArray<TElementType>& OldList, const TArray<TElementType>& NewList, TArray<TElementType>& OutAdded, TArray<TElementType>& OutRemoved)
	{
		// Entries that are in New list but not in old list are added
		for (const TElementType& Element : NewList)
		{
			if (!OldList.Contains(Element))
			{
				OutAdded.Add(Element);
			}
		}

		// Entries that are in Old list but not in the New list are removed
		for (const TElementType& Element : OldList)
		{
			if (!NewList.Contains(Element))
			{
				OutRemoved.Add(Element);
			}
		}
	}

	template<typename TReturnType>
	static void ConvertQuestResultToType(EQuestResult InResult, TReturnType NotStarted, TReturnType InProgress, TReturnType Completed, TReturnType Failed, TReturnType& OutResult)
	{
		switch (InResult)
		{
		case EQuestResult::NotStarted:
			OutResult = NotStarted;
			break;
		case EQuestResult::InProgress:
			OutResult = InProgress;
			break;
		case EQuestResult::Completed:
			OutResult = Completed;
			break;
		case EQuestResult::Failed:
			OutResult = Failed;
			break;
		}
	}
};

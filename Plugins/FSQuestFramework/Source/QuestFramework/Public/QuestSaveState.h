// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Misc/DateTime.h"

#include "QuestData.h"
#include "QuestMemory.h"

#include "QuestSaveState.generated.h"

class UQuest;
class UQuestContext;

/**
 * Represents state of single quest context
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestContextSaveStateData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	EQuestResult ResultState;

	UPROPERTY()
	FGuid NodeGuid;

	UPROPERTY()
	FQuestMemoryBlock MemoryBlock;

	FQuestContextSaveStateData()
	{
		Guid = FGuid();
		ResultState = EQuestResult::NotStarted;
		NodeGuid = FGuid();
	}

	FQuestContextSaveStateData(const UQuestContext* Context)
	{
		Guid = FGuid();
		ResultState = EQuestResult::NotStarted;
		NodeGuid = FGuid();
		StoreQuestContext(Context);
	}

	// Store/resores quest context from this save state
	bool StoreQuestContext(const UQuestContext* Context);
	void RestoreQuestContext(UQuestContext* Context) const;
};

/**
 * Represents saved contexts under quest
 */
USTRUCT()
struct QUESTFRAMEWORK_API FQuestContextSaveState
{
	GENERATED_BODY()

public:

	/**
	 * Guid of a quest to search for quest asset
	 */
	UPROPERTY()
	FGuid QuestGuid;

	/**
	 * Saved states of quest contexts associated with the quest
	 */
	UPROPERTY()
	TArray<FQuestContextSaveStateData> ContextStates;

	FQuestContextSaveState()
	{
		QuestGuid = FGuid();
		ContextStates.Empty();
	}

	// Prepares writing operation
	void PrepareWrite(const UQuest* QuestAsset);

	// Adds a new entry and writes data of given quest context to ContextStates
	void WriteContext(const UQuestContext* Context);
};

/**
 * Represents full snapshot of quest states
 * Can be saved/loaded via quest manager
 * Store save data in this object to your save game file
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestSaveStateData
{
	GENERATED_BODY()

public:

	/**
	 * Time when this save data was created
	 */
	UPROPERTY()
	FDateTime Timestamp;

	/**
	 * Actual data of quest context that can be used to restore their states
	 */
	UPROPERTY()
	TArray<FQuestContextSaveState> Data;

	FQuestSaveStateData()
	{
		Reset();
	}

	void Reset()
	{
		Timestamp = FDateTime();
		Data.Empty();
	}
};

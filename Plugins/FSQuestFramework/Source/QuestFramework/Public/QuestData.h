// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Object.h"
#include "UObject/UObjectIterator.h"
#include "Templates/SubclassOf.h"

#include "Math/NumericLimits.h"
#include "GameplayTagContainer.h"

#include "QuestData.generated.h"

#define QUEST_INDEX_NONE MAX_int32
#define QUEST_MIN_FLT (-(MAX_flt))
#define QUEST_MAX_FLT (MAX_flt)
#define QUEST_MIN_INT (MIN_int32)
#define QUEST_MAX_INT (MAX_int32)

class UQuestContext;

// Results for objectives and quest stages
UENUM(BlueprintType)
enum class EQuestResult : uint8
{
	// Objective, stage or quest has not been started yet
	NotStarted = 0,

	// Objective, stage or quest is currently in progress
	InProgress,

	// Objective, stage or quest completed succesfully
	Completed,

	// Objective, stage or quest ended in failure
	Failed,
};

/**
 * Generic comparison operator enum used in various features in quest framework
 */
UENUM(BlueprintType)
enum class EQuestComparisonOperator : uint8
{
	Equal = 0,
	NotEqual,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
};

/**
 * Generic logical operator enum used in various features in quest framework
 */
UENUM(BlueprintType)
enum class EQuestLogicalOperator : uint8
{
	And = 0,
	Or
};

// Enum to determine authority level in networked environment
UENUM(BlueprintType)
enum class EQuestNetworkAuthority : uint8
{
	// Authority cannot be determined
	None = 0,

	// has server authority
	HasAuthority,

	// is remote proxy
	Remote,
};

/**
 * Determines execution method for quest related objects in networked environment
 */
UENUM(BlueprintType)
enum class EQuestNetworkExecType : uint8 
{
	// runs only on server
	OnlyServer = 0,

	// runs only on owning client
	OnlyClient,

	// runs on both server and client
	Both
};

/**
 * Payload to send with quest events
 */
USTRUCT(BlueprintType)
struct QUESTFRAMEWORK_API FQuestEventPayload
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	int32 IntegerParameter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	float FloatParameter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	bool bBooleanParameter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	TSubclassOf<UObject> ClassParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	TObjectPtr<UObject> ObjectParameter;

	FQuestEventPayload();
};

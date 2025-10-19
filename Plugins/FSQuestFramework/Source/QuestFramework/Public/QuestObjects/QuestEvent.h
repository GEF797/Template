// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestData.h"
#include "QuestMemory.h"
#include "QuestEvent.generated.h"

class UQuestContext;

/**
 * Event that can be triggered upon entry to a quest node or via quest services
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestEvent : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	void NotifyExecuteEvent(class UQuestContext* Context, const FQuestEventPayload& InPayload);

	bool ShouldTriggerOnStateRestore() const { return bTriggerOnStateRestore; }

protected:

	/**
	 * Determines the machines that are allowed to run the event in networked environments
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Event")
	EQuestNetworkExecType NetworkExecutionType = EQuestNetworkExecType::Both;

	/**
	 * Is this event triggered again when quest state is restored (eg. load of savegame)?
	 * This works only for entry events of stage nodes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Event")
	bool bTriggerOnStateRestore = false;

	/**
	 * Function to override in derived class (C++ or Blueprint) to implement event
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Event")
	void ExecuteEvent(UQuestContext* Context, const FQuestEventPayload& Payload);
	virtual void ExecuteEvent_Implementation(UQuestContext* Context, const FQuestEventPayload& InPayload) {}
};

// ---------------------------------------------------------------- //

/**
 * Event that sends event name and payload to quest event listeners
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, meta = (DisplayName = "Send Message To Listeners"))
class QUESTFRAMEWORK_API UQuestMessageEvent : public UQuestEvent
{
	GENERATED_BODY()

protected:

	virtual void ExecuteEvent_Implementation(UQuestContext* Context, const FQuestEventPayload& InPayload) override;

protected:

	/**
	 * Name of the message to send to listeners
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Event", meta = (Categories = "QuestMessage"))
	FGameplayTag MessageTag;

	/**
	 * Payload to send with the message
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Event")
	FQuestEventPayload Payload;
};

// ---------------------------------------------------------------- //

/**
 * Event that sends event name and payload to quest event listeners
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, meta = (DisplayName="Set Variable"))
class QUESTFRAMEWORK_API UQuestEvent_SetVariable : public UQuestEvent
{
	GENERATED_BODY()

protected:

	virtual void ExecuteEvent_Implementation(UQuestContext* Context, const FQuestEventPayload& InPayload) override;

protected:

	/**
	 * Initializer for quest variable. Sets variable into desired value when the event triggers
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Event")
	FQuestVariableInitializer VariableSetter;
};

// ---------------------------------------------------------------- //
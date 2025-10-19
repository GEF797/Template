// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "QuestObjectBase.h"
#include "QuestMemory.h"
#include "QuestMessageInterface.h"
#include "QuestService.generated.h"

class UQuestContext;

/**
 * Service that runs when quest stage is active.
 * Services can be used to update quest variables or do any ticking actions
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType, EditInlineNew)
class QUESTFRAMEWORK_API UQuestService : public UQuestObjectBase
{
	GENERATED_BODY()
	
public:

	/**
	 * Override to specify if the service should be instanced or if the default instance is shared
	 * Any changes to the object requires it to be instanced such as when modifying blueprint variables etc.
	 * When the service is not instanced, it should be treated as read-only
	 */
	virtual bool IsInstanced() const { return false; }

	/**
	 * Override to provide execution method for networked environment
	 */
	virtual EQuestNetworkExecType GetNetworkExecutionType() const { return NetworkExecutionType; }

protected:

	/**
	 * Determines the machines that are suppose to run the service in networked environments
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service")
	EQuestNetworkExecType NetworkExecutionType = EQuestNetworkExecType::OnlyServer;

	/**
	 * Called when the service comes active (owner node comes active)
	 */
	virtual void OnServiceActivated(UQuestContext* Context) {}

	/**
	 * Called when the service comes deactive (owner node comes deactive)
	 */
	virtual void OnServiceDeactivated(UQuestContext* Context) {}

	/**
	 * Called each frame when the service is active
	 */
	virtual void TickService(UQuestContext* Context, float DeltaTime) {}

public:

	/**
	 * Internal notifiers from quest context
	 */
	virtual void NotifyServiceBecomeActive(UQuestContext* Context);
	virtual void NotifyServiceBecomeDeactive(UQuestContext* Context);
	virtual void NotifyTickService(UQuestContext* Context, float DeltaTime);
};


// -------------------------------------- //

/**
 * Service that runs when quest stage is active.
 * Services can be used to update quest variables, listen messages or do any ticking actions.
 * Blueprint-derived services are always instanced per node, so it is safe to modify variables in them.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestService_BlueprintBase : public UQuestService
{
	GENERATED_BODY()

public:

	virtual bool IsInstanced() const { return true; }

protected:

	/**
	 * Called when the service comes active (owner node comes active)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Service", meta = (DisplayName="OnServiceActivated"))
	void OnServiceActivatedBP(UQuestContext* Context);

	/**
	 * Called when the service comes deactive (owner node comes deactive)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Service", meta = (DisplayName = "OnServiceDeactivated"))
	void OnServiceDeactivatedBP(UQuestContext* Context);

	/**
	 * Called each frame when the service is active
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Service", meta = (DisplayName = "TickService"))
	void TickServiceBP(UQuestContext* Context, float DeltaTime);

public:

	virtual void NotifyServiceBecomeActive(UQuestContext* Context);
	virtual void NotifyServiceBecomeDeactive(UQuestContext* Context);
	virtual void NotifyTickService(UQuestContext* Context, float DeltaTime);
};

/**
 * Service to act as timer.
 * Updates quest variable per update
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, meta = (DisplayName="Timer"))
class QUESTFRAMEWORK_API UQuestService_Timer : public UQuestService
{
	GENERATED_BODY()

public:

	UQuestService_Timer();

protected:

	/*
	* Initial value to set into quest variable when the service is activated
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service")
	float InitialTime = 0.0f;

	/**
	 * How much is added to the quest variable per second? (Use negative value to count down)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service")
	float Speed = 1.0f;

	/**
	 * Variable to use as a timer
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service")
	FQuestVariable Variable;

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override;
	virtual void TickService(UQuestContext* Context, float DeltaTime) override;

	float GetTimeVariableValue(UQuestContext* Context) const;
	void SetTimeVariableValue(UQuestContext* Context, float Value);

};

/**
 * Service to trigger quest events
 */
UCLASS(Abstract, EditInlineNew)
class QUESTFRAMEWORK_API UQuestService_EventTriggerBase : public UQuestService
{
	GENERATED_BODY()

public:

	UQuestService_EventTriggerBase() {}

protected:

	/**
	 * Events that are executed when the service triggers
	 */
	UPROPERTY(Instanced, EditAnywhere, Category = "Service")
	TArray<TObjectPtr<class UQuestEvent>> Events;

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override 
	{
		Super::OnServiceActivated(Context);
	}
	virtual void OnServiceDeactivated(UQuestContext* Context) override 
	{
		Super::OnServiceDeactivated(Context);
	}
	virtual void TickService(UQuestContext* Context, float DeltaTime) override 
	{
		Super::TickService(Context, DeltaTime);
	}

	void TriggerEvents(UQuestContext* Context, const FQuestEventPayload& Payload);
};

/**
 * Service to trigger quest events on regular basis
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, meta = (DisplayName="Trigger Events On Regular Basis"))
class QUESTFRAMEWORK_API UQuestService_EventTrigger_Interval : public UQuestService_EventTriggerBase
{
	GENERATED_BODY()

public:

	UQuestService_EventTrigger_Interval() {}

	virtual bool IsInstanced() const { return true; }

protected:

	/**
	 * How often in seconds the events are triggered?
	 */
	UPROPERTY(EditAnywhere, Category = "Service")
	float TriggerInterval = 1.0f;

	/**
	 * Should the events triggered when the service activates?
	 */
	UPROPERTY(EditAnywhere, Category = "Service")
	bool bTriggerOnActivation = false;

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override 
	{
		TriggerTimer = 0.0f;
		if (bTriggerOnActivation)
		{
			TriggerEvents(Context, FQuestEventPayload());
		}
	}

	virtual void TickService(UQuestContext* Context, float DeltaTime) override
	{
		Super::TickService(Context, DeltaTime);

		TriggerTimer += DeltaTime;
		if (TriggerTimer >= TriggerInterval)
		{
			TriggerTimer = 0.0f;
			TriggerEvents(Context, FQuestEventPayload());
		}
	}

private:

	float TriggerTimer = 0.0f;
};


/**
 * Service that can listen to messages coming from message component
 */
UCLASS(Abstract, EditInlineNew, meta = (DisplayName = "Listen Message"))
class QUESTFRAMEWORK_API UQuestService_ListenMessage : public UQuestService_BlueprintBase,
	public IQuestMessageInterface
{
	GENERATED_BODY()

public:

	UQuestService_ListenMessage() {}

protected:

	/**
	 * Called when the service recives a message. Requires service being active
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Service")
	void OnMessageReceived(UQuestContext* Context, const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Sender);

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override;

	virtual void OnServiceDeactivated(UQuestContext* Context) override;

	virtual void ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender) override;
};

/**
 * Service that listens to messages and triggers events when it receives one
 */
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, meta = (DisplayName="Trigger Events On Message"))
class QUESTFRAMEWORK_API UQuestService_TriggerEventsOnMessage : public UQuestService_EventTriggerBase,
	public IQuestMessageInterface
{
	GENERATED_BODY()

public:

	UQuestService_TriggerEventsOnMessage() {}

	virtual bool IsInstanced() const { return true; }

protected:

	/**
	* List of message tags to listen to for event trigger
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service", meta = (Categories = "QuestMessage"))
	FGameplayTagContainer MessagesToListen;

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override;

	virtual void OnServiceDeactivated(UQuestContext* Context) override;

	virtual void ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender) override;
};

/**
 * Service that implements custom logic to keep variable synced in tick
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, meta = (DisplayName = "Sync Variable"))
class QUESTFRAMEWORK_API UQuestService_SyncVariable : public UQuestService_BlueprintBase
{
	GENERATED_BODY()

public:

	UQuestService_SyncVariable() {}

protected:

	/**
	 * Variable to keep sync
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service")
	FQuestVariable Variable;

	/**
	 * Override to return integer value to set variable per tick
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Service")
	int32 GetIntegerSyncValue(UQuestContext* Context) const;
	int32 GetIntegerSyncValue_Implementation(UQuestContext* Context) const { return 0; }

	/**
	 * Override to return float value to set variable per tick
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Service")
	float GetFloatSyncValue(UQuestContext* Context) const;
	float GetFloatSyncValue_Implementation(UQuestContext* Context) const { return 0.0f; }

	/**
	 * Override to return bool value to set variable per tick
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Service")
	bool GetBoolSyncValue(UQuestContext* Context) const;
	bool GetBoolSyncValue_Implementation(UQuestContext* Context) const { return false; }

protected:

	virtual void OnServiceActivated(UQuestContext* Context) override;
	virtual void TickService(UQuestContext* Context, float DeltaTime) override;

private:

	void Sync(UQuestContext* Context);
};
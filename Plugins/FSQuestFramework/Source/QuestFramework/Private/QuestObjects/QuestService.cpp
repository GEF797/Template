// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestService.h"
#include "QuestContext.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestManager.h"

void UQuestService::NotifyServiceBecomeActive(UQuestContext* Context)
{
	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	OnServiceActivated(Context);
}

void UQuestService::NotifyServiceBecomeDeactive(UQuestContext* Context)
{
	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	OnServiceDeactivated(Context);
}

void UQuestService::NotifyTickService(UQuestContext* Context, float DeltaTime)
{
	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	TickService(Context, DeltaTime);
}

// -------------------------------------- //

void UQuestService_BlueprintBase::NotifyServiceBecomeActive(UQuestContext* Context)
{
	Super::NotifyServiceBecomeActive(Context);
	OnServiceActivatedBP(Context);
}

void UQuestService_BlueprintBase::NotifyServiceBecomeDeactive(UQuestContext* Context)
{
	OnServiceDeactivatedBP(Context);
	Super::NotifyServiceBecomeDeactive(Context);
}

void UQuestService_BlueprintBase::NotifyTickService(UQuestContext* Context, float DeltaTime)
{
	Super::NotifyTickService(Context, DeltaTime);
	TickServiceBP(Context, DeltaTime);
}

// -------------------------------------- //

UQuestService_Timer::UQuestService_Timer()
{
	Variable.Type = EQuestVariableType::Float;
	NetworkExecutionType = EQuestNetworkExecType::Both;
}

void UQuestService_Timer::OnServiceActivated(UQuestContext* Context)
{
	Super::OnServiceActivated(Context);
	SetTimeVariableValue(Context, InitialTime);
}


void UQuestService_Timer::TickService(UQuestContext* Context, float DeltaTime)
{
	Super::TickService(Context, DeltaTime);

	const float NewTime = GetTimeVariableValue(Context) + (DeltaTime * Speed);
	SetTimeVariableValue(Context, NewTime);
}

float UQuestService_Timer::GetTimeVariableValue(UQuestContext* Context) const
{
	if (!Context)
	{
		return 0.0f;
	}

	const UQuestMemory* Memory = Context->GetQuestMemory();
	check(Memory);
	return Variable.GetFromMemoryBlock<float>(Memory->GetMemoryBlock());
}

void UQuestService_Timer::SetTimeVariableValue(UQuestContext* Context, float Value)
{
	if (!Context)
	{
		return;
	}

	UQuestMemory* Memory = Context->GetQuestMemory();
	check(Memory);
	Variable.SetInMemoryBlock<float>(Memory->GetMemoryBlock(), Value);
}

// -------------------------------------- //

void UQuestService_EventTriggerBase::TriggerEvents(UQuestContext* Context, const FQuestEventPayload& Payload)
{
	for (UQuestEvent* Event : Events)
	{
		if (IsValid(Event))
		{
			Event->NotifyExecuteEvent(Context, Payload);
		}
	}
}

// -------------------------------------- //

void UQuestService_ListenMessage::OnServiceActivated(UQuestContext* Context)
{
	Super::OnServiceActivated(Context);
	if (UQuestManager* Manager = Context->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->RegisterQuestContextMessageListener(this, Context);
	}
}

void UQuestService_ListenMessage::OnServiceDeactivated(UQuestContext* Context)
{
	if (UQuestManager* Manager =Context->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->UnregisterQuestContextMessageListener(this, Context);
	}
	Super::OnServiceDeactivated(Context);
}

void UQuestService_ListenMessage::ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender)
{
	if (UQuestContext* Context = GetTypedOuter<UQuestContext>())
	{
		OnMessageReceived(Context, MessageTag, Payload, Sender);
	}
}

// ------------------------------------------------------------- //

void UQuestService_TriggerEventsOnMessage::OnServiceActivated(UQuestContext* Context)
{
	Super::OnServiceActivated(Context);
	if (UQuestManager* Manager = Context->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->RegisterQuestContextMessageListener(this, Context);
	}
}

void UQuestService_TriggerEventsOnMessage::OnServiceDeactivated(UQuestContext* Context)
{
	if (UQuestManager* Manager = Context->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->UnregisterQuestContextMessageListener(this, Context);
	}
	Super::OnServiceDeactivated(Context);
}

void UQuestService_TriggerEventsOnMessage::ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient, AActor* Sender)
{
	if (MessagesToListen.HasTagExact(MessageTag))
	{
		if (UQuestContext* Context = GetTypedOuter<UQuestContext>())
		{
			TriggerEvents(Context, Payload);
		}
	}
}

// ------------------------------------------------------------- //

void UQuestService_SyncVariable::OnServiceActivated(UQuestContext* Context)
{
	Sync(Context);
}

void UQuestService_SyncVariable::TickService(UQuestContext* Context, float DeltaTime)
{
	Sync(Context);
}

void UQuestService_SyncVariable::Sync(UQuestContext* Context)
{
	if (Context)
	{
		UQuestMemory* Memory = Context->GetQuestMemory();
		check(Memory);

		FQuestMemoryBlock& MemoryBlock = Memory->GetMemoryBlock();
		if (Variable.Type == EQuestVariableType::Bool)
		{
			Variable.SetInMemoryBlock<bool>(MemoryBlock, GetBoolSyncValue(Context));
		}
		else if (Variable.Type == EQuestVariableType::Float)
		{
			Variable.SetInMemoryBlock<float>(MemoryBlock, GetFloatSyncValue(Context));
		}
		else if (Variable.Type == EQuestVariableType::Integer)
		{
			Variable.SetInMemoryBlock<int32>(MemoryBlock, GetIntegerSyncValue(Context));
		}
	}
}
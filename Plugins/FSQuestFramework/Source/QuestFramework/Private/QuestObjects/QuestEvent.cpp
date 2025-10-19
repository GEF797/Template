// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestEvent.h"
#include "QuestFrameworkModule.h"
#include "QuestContext.h"
#include "QuestManager.h"
#include "Components/QuestMessageComponent.h"

// ---------------------------------------------------------------- //

void UQuestEvent::NotifyExecuteEvent(UQuestContext* Context, const FQuestEventPayload& InPayload)
{
	if (!IsValid(Context))
	{
		UE_LOG(LogQuestFramework, Error, TEXT("Can't execute quest event (%s) because quest context was invalid!"), *GetFName().ToString());
		return;
	}

	if (!Context->ShouldExecuteNetworkAction(NetworkExecutionType))
	{
		return;
	}

	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());

	ExecuteEvent(Context, InPayload);
}

// ---------------------------------------------------------------- //

void UQuestMessageEvent::ExecuteEvent_Implementation(UQuestContext* Context, const FQuestEventPayload& InPayload)
{
	if (!ensure(Context))
	{
		return;
	}

	// Get all listeners and notify them about this event trigger
	if (UQuestManager* Manager = Context->GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		const TSet<TObjectPtr<UQuestMessageComponent>>& Listeners = Manager->GetMessageListeners();
		for (UQuestMessageComponent* Listener : Listeners)
		{
			if (!ensure(Listener))
			{
				continue;
			}
			Listener->ReceiveMessageFromQuestContext(Context, MessageTag, Payload);
		}
	}
}

// ---------------------------------------------------------------- //

void UQuestEvent_SetVariable::ExecuteEvent_Implementation(UQuestContext* Context, const FQuestEventPayload& InPayload)
{
	if (!ensure(Context))
	{
		return;
	}

	UQuestMemory* MemoryObject = Context->GetQuestMemory();
	check(MemoryObject);
	VariableSetter.SetValue(MemoryObject->GetMemoryBlock());
}
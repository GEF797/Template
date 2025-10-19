// Copyright Sami Kangasmaa, 2022


#include "Components/QuestMessageComponent.h"
#include "QuestContext.h"
#include "QuestManager.h"
#include "QuestUtilityLibrary.h"

UQuestMessageComponent::UQuestMessageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestMessageComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UQuestManager* Manager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->RegisterMessageListener(this);
	}
}

void UQuestMessageComponent::EndPlay(EEndPlayReason::Type Reason)
{
	if (UQuestManager* Manager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
	{
		Manager->UnregisterMessageListener(this);
	}
	Super::EndPlay(Reason);
}

void UQuestMessageComponent::ReceiveMessageFromQuestContext(UQuestContext* Context, const FGameplayTag& MessageTag, const FQuestEventPayload& Payload)
{
	if (!ensure(Context))
	{
		return;
	}

	if (!OnQuestMessage.IsBound())
	{
		return;
	}

	if (!MessagesToListen.IsEmpty())
	{
		if (!MessagesToListen.HasTagExact(MessageTag))
		{
			return;
		}
	}

	check(OnQuestMessage.IsBound());
	OnQuestMessage.Broadcast(Context, MessageTag, Payload);
}

void UQuestMessageComponent::SendQuestMessage(FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Recipient)
{
	if (IsValid(Recipient))
	{
		UQuestUtilityLibrary::SendQuestMessage(this, MessageTag, Payload, Recipient, GetOwner());
	}
}

void UQuestMessageComponent::BroadcastQuestMessage(FGameplayTag MessageTag, FQuestEventPayload Payload)
{
	UQuestUtilityLibrary::BroadcastQuestMessage(this, MessageTag, Payload, GetOwner());
}
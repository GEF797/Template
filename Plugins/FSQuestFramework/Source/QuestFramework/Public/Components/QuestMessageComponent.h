// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestMessageInterface.h"
#include "QuestMessageComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuestEventListenerDelegate, class UQuestContext*, Context, const FGameplayTag&, MessageTag, const FQuestEventPayload&, Payload);

/**
* Component that can listen to and send quest events
* Add this to any actor blueprint that want event notify from a quest
*/
UCLASS( ClassGroup=("Quest"), meta=(BlueprintSpawnableComponent, DisplayName = "QuestMessageListener") )
class QUESTFRAMEWORK_API UQuestMessageComponent : public UActorComponent,
	public IQuestMessageInterface
{
	GENERATED_BODY()

public:

	UQuestMessageComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

	// Notifies the listener about a quest message event
	virtual void ReceiveMessageFromQuestContext(class UQuestContext* Context, const FGameplayTag& MessageTag, const FQuestEventPayload& Payload) override;

	/**
	 * Called on quest event.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Quest Message")
	FQuestEventListenerDelegate OnQuestMessage;

	/**
	 * Sends a message to quest contexts that are owned by Recipient actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Message", meta = (GameplayTagFilter = "QuestMessage"))
	void SendQuestMessage(FGameplayTag MessageTag, FQuestEventPayload Payload, AActor* Recipient);

	/**
	 * Sends a message to all quest contexts
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest Message", meta = (GameplayTagFilter = "QuestMessage"))
	void BroadcastQuestMessage(FGameplayTag MessageTag, FQuestEventPayload Payload);

protected:

	/**
	 * List of message tags to listen to. When left empty, all message tags trigger OnQuestMessage delegate
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Message", meta = (Categories = "QuestMessage"))
	FGameplayTagContainer MessagesToListen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Message", meta = (Categories = "QuestMessage"))
	FGameplayTag MessageToSend;
};

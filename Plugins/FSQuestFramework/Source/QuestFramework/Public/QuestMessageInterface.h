// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuestData.h"
#include "QuestMessageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UQuestMessageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface to send messages between quest objects
 */
class QUESTFRAMEWORK_API IQuestMessageInterface
{
	GENERATED_BODY()

public:

	// Message from quest to message components
	virtual void ReceiveMessageFromQuestContext(class UQuestContext* Context, const FGameplayTag& MessageTag, const FQuestEventPayload& Payload) {}

	// Message from message components to quest
	virtual void ReceiveMessageFromListener(const FGameplayTag& MessageTag, const FQuestEventPayload& Payload, AActor* Recipient = nullptr, AActor* Sender = nullptr) {}
};

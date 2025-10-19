// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "QuestManagerAccess.h"
#include "QuestManager.h"

#include "QuestGameInstance.generated.h"

/**
 * Game instance that implements QuestManager access by default
 */
UCLASS()
class QUESTFRAMEWORKEXTENSION_API UQuestGameInstance : public UGameInstance//,
	//public IQuestManagerAccess
{
	GENERATED_BODY()

public:

	UQuestGameInstance();
	
	//** ----- Implementation of IQuestManagerAccess -----

	// Stores given manager instance to QuestManagerInstance-property
	virtual void RetainQuestManager_Implementation(UQuestManager* Manager);

	// Returns QuestManagerInstance
	virtual UQuestManager* GetQuestManagerInstance_Implementation() const;

private:

	// Global instance of the quest manager
	UPROPERTY(Transient)
	TObjectPtr<UQuestManager> QuestManagerInstance;

	//** --------------------------------------------------
};

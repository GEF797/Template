// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuestComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UQuestComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class UQuestComponent;
class UQuestTrackerComponent;

/**
 * Interface for faster access to quest component in actors.
 * Used by QuestUtilityLibrary to find quest components via GetQuestComponentFromActor
 */
class QUESTFRAMEWORK_API IQuestComponentInterface
{
	GENERATED_BODY()

public:

	/**
	 * Override to return quest component of an actor
	 */
	virtual UQuestComponent* GetQuestComponent() const = 0;

	/**
	 * Override to return quest tracker component.
	 */
	virtual UQuestTrackerComponent* GetQuestTrackerComponent() const { return nullptr; }
};

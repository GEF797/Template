// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "QuestFramework.h"
#include "QuestComponentInterface.h"

#include "QuestPlayerController.generated.h"

/**
 * Player controller that implements quest component by default
 */
UCLASS()
class QUESTFRAMEWORKEXTENSION_API AQuestPlayerController : public APlayerController,
	public IQuestComponentInterface
{
	GENERATED_BODY()
	
public:

	AQuestPlayerController();

public:

	/**
	 * Returns quest component of the player.
	 * Prevents component lookup to speed up fetching the quest component of the players
	 */
	virtual UQuestComponent* GetQuestComponent() const override 
	{
		return QuestFrameworkComponent;
	}

	/**
	 * Returns quest tracker component of the player
	 */
	virtual UQuestTrackerComponent* GetQuestTrackerComponent() const override 
	{
		return QuestTrackerComponent;
	}

protected:

	/**
	* Quest component to manage quest contexts of the player
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Framework")
	TObjectPtr<UQuestComponent> QuestFrameworkComponent;

	/**
	 * Component to manage tracking of quests
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Framework")
	TObjectPtr<UQuestTrackerComponent> QuestTrackerComponent;
};

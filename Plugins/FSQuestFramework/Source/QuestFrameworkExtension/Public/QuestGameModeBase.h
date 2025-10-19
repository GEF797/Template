// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QuestGameModeBase.generated.h"

/**
 * Game mode that uses QuestPlayerController as default controller
 */
UCLASS()
class QUESTFRAMEWORKEXTENSION_API AQuestGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	AQuestGameModeBase();
};

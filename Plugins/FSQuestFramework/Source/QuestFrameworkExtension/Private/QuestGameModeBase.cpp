// Copyright Sami Kangasmaa, 2022


#include "QuestGameModeBase.h"
#include "QuestPlayerController.h"

AQuestGameModeBase::AQuestGameModeBase()
{
	PlayerControllerClass = AQuestPlayerController::StaticClass();
}
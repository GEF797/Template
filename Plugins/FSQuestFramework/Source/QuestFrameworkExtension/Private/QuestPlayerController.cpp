// Copyright Sami Kangasmaa, 2022


#include "QuestPlayerController.h"

AQuestPlayerController::AQuestPlayerController()
{
	QuestFrameworkComponent = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));
	QuestTrackerComponent = CreateDefaultSubobject<UQuestTrackerComponent>(TEXT("QuestTracker"));
}
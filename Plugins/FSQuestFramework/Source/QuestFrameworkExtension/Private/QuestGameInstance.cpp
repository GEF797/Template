// Copyright Sami Kangasmaa, 2022


#include "QuestGameInstance.h"

UQuestGameInstance::UQuestGameInstance() {}

void UQuestGameInstance::RetainQuestManager_Implementation(UQuestManager* Manager)
{
	ensure(Manager);
	QuestManagerInstance = Manager;
}

UQuestManager* UQuestGameInstance::GetQuestManagerInstance_Implementation() const
{
	return IsValid(QuestManagerInstance) ? QuestManagerInstance : nullptr;
}
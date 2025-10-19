// Copyright G.E.FC 2023.


#include "System/RPGSaveGame.h"

void URPGSaveGame::SaveWorld(AActor* Context)
{
	if (Context->GetWorld())
	{
		Level = MakeSoftObjectPtr<UWorld>(Context->GetWorld());
	}
	

}

//void URPGSaveGame::SaveQuest(const FQuestContextSaveStateData& Quest)
//{
//	QuestStates= Quest;
//}
//
//FQuestContextSaveStateData URPGSaveGame::GetQuest()
//{
//	return QuestStates;
//}

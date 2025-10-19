// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestCustomData.h"
#include "Quest.h"

UQuest* UQuestCustomData::GetQuestAsset() const
{
	return GetTypedOuter<UQuest>();
}
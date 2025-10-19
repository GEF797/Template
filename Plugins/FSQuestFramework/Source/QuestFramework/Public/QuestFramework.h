// Copyright Sami Kangasmaa, 2022

#pragma once

/**
* Common headers of quest framework to include in main game module
*/

#include "QuestData.h"
#include "QuestMemory.h"
#include "Quest.h"
#include "QuestContext.h"
#include "QuestManager.h"
#include "QuestSaveState.h"
#include "QuestTextTypes.h"
#include "QuestUtilityLibrary.h"

#include "QuestObjects/QuestCondition.h"
#include "QuestObjects/QuestEvent.h"
#include "QuestObjects/QuestObjective.h"
#include "QuestObjects/QuestObjectiveObserver.h"
#include "QuestObjects/QuestService.h"
#include "QuestObjects/QuestTextArgument.h"
#include "QuestObjects/QuestCustomData.h"

#include "Components/QuestComponent.h"
#include "Components/QuestMessageComponent.h"
#include "Components/QuestTrackerComponent.h"

#include "Nodes/QuestNode_End.h"
#include "Nodes/QuestNode_Root.h"
#include "Nodes/QuestNode_Stage.h"
#include "Nodes/QuestNode_Selector.h"
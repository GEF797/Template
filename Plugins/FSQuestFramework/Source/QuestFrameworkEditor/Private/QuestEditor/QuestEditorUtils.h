// Copyright, Sami Kangasmaa 2022

#pragma once

#include "CoreMinimal.h"

#include "Quest.h"
#include "QuestMemory.h"
#include "UObject/UnrealType.h"


/**
 * Utility library for quest editor
 */
class FQuestEditorUtils
{
private:

	FQuestEditorUtils() {}

public:

	~FQuestEditorUtils() {}

public:

	/**
	 * Finds all used quest variables in given quest
	 */
	static TArray<FName> FindQuestVariableNames(UQuest* QuestAsset, EQuestVariableType Type);

	/**
	 * Fetches quest variable info from given quest
	 */
	static TArray<FQuestVariableInfo> CreateQuestVariableInfoData(UQuest* QuestAsset);

private:

	// Internal functions to fetch quest variable names in chain of properties (recursive)
	static void AppendQuestVariableNames_Object(const UObject* Container, EQuestVariableType Type, TSet<FName>& Names, TSet<const UObject*>& VisitedObjects, bool bRecursiveObjects = true);
	static void AppendQuestVariableNames_Struct(const void* Container, const FStructProperty* InStructProperty, EQuestVariableType Type, TSet<FName>& Names, TSet<const UObject*>& VisitedObjects, bool bRecursiveObjects = true);
};

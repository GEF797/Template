// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestNode_Selector.h"
#include "QuestContext.h"

int32 UQuestNode_Selector::EvaluateChildrenConditions(UQuestContext* Context) const
{
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++)
	{
		const int32* ConditionIndex = ChildToConditionIndexTable.Find(ChildIndex);
		if (!ConditionIndex)
		{
			return ChildIndex; // No condition -> pass
		}
		else
		{
			if (!ensure(Conditions.IsValidIndex(*ConditionIndex)))
			{
				return QUEST_INDEX_NONE;
			}

			if (Conditions[*ConditionIndex].EvaluateConditions(Context))
			{
				return ChildIndex;
			}
		}
	}
	return QUEST_INDEX_NONE;
}

const FQuestConditions* UQuestNode_Selector::GetConditionsForChild(int32 ChildIndex) const
{
	const int32* ConditionIndex = ChildToConditionIndexTable.Find(ChildIndex);
	if (!ConditionIndex)
	{
		return nullptr;
	}
	else
	{
		if (!ensure(Conditions.IsValidIndex(*ConditionIndex)))
		{
			return nullptr;
		}
		return &(Conditions[*ConditionIndex]);
	}

	
}
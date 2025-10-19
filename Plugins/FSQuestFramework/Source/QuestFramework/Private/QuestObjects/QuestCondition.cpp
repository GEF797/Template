// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestCondition.h"
#include "QuestContext.h"

bool UQuestCondition::ExecuteCheckCondition(UQuestContext* Context) const
{
	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	return CheckCondition(Context);
}


bool UQuestCondition_VariableQuery::CheckCondition_Implementation(UQuestContext* Context) const
{
	if (!ensure(Context && Context->GetQuestMemory()))
	{
		return false;
	}
	return Query.RunQuery(Context->GetQuestMemory()->GetMemoryBlock());
}

bool FQuestConditions::EvaluateConditions(UQuestContext* Context) const
{
	if (!ensure(Context))
	{
		return false;
	}

	// No conditions -> pass
	if (Conditions.Num() <= 0)
	{
		return true;
	}

	if (ComparisonMethod == EQuestConditionCompareMethod::All)
	{
		for (const UQuestCondition* Condition : Conditions)
		{
			if (Condition)
			{
				if (!Condition->ExecuteCheckCondition(Context))
				{
					return false; // One failed -> fail test as expected result was all to pass
				}
			}
		}

		return true; // If any condition failed, the function already returned false, all passed at this point
	}
	else // Any
	{
		for (const UQuestCondition* Condition : Conditions)
		{
			if (Condition)
			{
				if (Condition->ExecuteCheckCondition(Context))
				{
					return true; // One passed -> test passes
				}
			}
		}

		return false; // If any passed, the function should have returned true already
	}

	//checkNoEntry();
	//return false;
}

FQuestConditions FQuestConditions::MakeNewInstance(UObject* Outer, const FQuestConditions& Template)
{
	check(Outer);

	FQuestConditions NewConditions;
	NewConditions.ComparisonMethod = Template.ComparisonMethod;
	for (UQuestCondition* Condition : Template.Conditions)
	{
		if (Condition)
		{
			UQuestCondition* NewCondition = NewObject<UQuestCondition>(Outer, Condition->GetClass(), NAME_None, RF_NoFlags, Condition);
			check(NewCondition);
			NewConditions.Conditions.Add(NewCondition);
		}
	}

	return NewConditions;
}
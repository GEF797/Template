// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestObjectiveObserver.h"
#include "QuestContext.h"

EQuestResult UQuestObjectiveObserver::NotifyEvaluateResult(const UQuestContext* Context) const
{
	FQuestObjectWorldScope WorldScope(this, Context->GetWorld());
	return Evaluate(const_cast<UQuestContext*>(Context));
}

EQuestResult UQuestObjectiveObserver::Evaluate_Implementation(UQuestContext* Context) const
{
	return EQuestResult::Completed;
}

// -------------------------------- //

EQuestResult UObjectiveObserver_VariableQuery::Evaluate_Implementation(UQuestContext* Context) const
{
	if (!ensure(Context))
	{
		return EQuestResult::Failed;
	}

	const UQuestMemory* Memory = Context->GetQuestMemory();
	if (!ensure(Memory))
	{
		return EQuestResult::Failed;
	}

	const FQuestMemoryBlock& MemoryBlock = Memory->GetMemoryBlock();
	if (FailQuery.HasComparisonOperators())
	{
		if (FailQuery.RunQuery(MemoryBlock))
		{
			return EQuestResult::Failed;
		}
	}

	if (CompleteQuery.HasComparisonOperators())
	{
		if (CompleteQuery.RunQuery(MemoryBlock))
		{
			return EQuestResult::Completed;
		}
	}

	return EQuestResult::InProgress;
}
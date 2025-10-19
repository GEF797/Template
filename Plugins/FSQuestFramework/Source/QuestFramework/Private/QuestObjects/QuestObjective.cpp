// Copyright Sami Kangasmaa, 2022


#include "QuestObjects/QuestObjective.h"

#include "QuestObjects/QuestObjectiveObserver.h"
#include "QuestContext.h"

UQuestObjective::UQuestObjective()
{
	Observer = nullptr;
}

EQuestResult UQuestObjective::EvaluateObserver(const UQuestContext* Context) const
{
	if (!ensure(Context))
	{
		return EQuestResult::Failed;
	}

	// Objective passes automatically if there isn't observer
	if (!IsValid(Observer))
	{
		return EQuestResult::Completed;
	}

	const EQuestResult ObserverResult = Observer->NotifyEvaluateResult(Context);

	// Special case. This functions should have not called if the objective is not yet started.
	if (ObserverResult == EQuestResult::NotStarted)
	{
		return EQuestResult::InProgress;
	}

	return ObserverResult;
}

bool UQuestObjective::IsActive(const UQuestContext* Context) const
{
	if (Context)
	{
		return Context->IsObjectiveActive(const_cast<UQuestObjective*>(this));
	}
	return false;
}

bool UQuestObjective::HasTextArguments() const
{
	return ShortDescription.TextArguments.Num() > 0 || LongDescription.TextArguments.Num() > 0;
}

#if WITH_EDITOR

bool UQuestObjective::SuggestLogicalName(const FName& InName)
{
	if (LogicalName.IsNone())
	{
		LogicalName = InName;
		return true;
	}
	return false;
}

void UQuestObjective::PostLoad()
{
	Super::PostLoad();

	if (!IsValid(Observer))
	{
		Observer = NewObject<UObjectiveObserver_VariableQuery>(this);
	}
}

void UQuestObjective::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{

	FProperty* Property = PropertyChangedEvent.PropertyChain.GetActiveNode()->GetValue();
	FProperty* MemberProperty = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue();
	if (Property && MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		static FName NAME_ShortDescriptionProperty = GET_MEMBER_NAME_CHECKED(UQuestObjective, ShortDescription);
		static FName NAME_LongDescriptionProperty = GET_MEMBER_NAME_CHECKED(UQuestObjective, LongDescription);

		if (MemberProperty->GetFName() == NAME_ShortDescriptionProperty && Property->GetFName() == FQuestText::GetTextPropertyName())
		{
			ShortDescription.NotifyEditText();
		}
		else if (MemberProperty->GetFName() == NAME_LongDescriptionProperty && Property->GetFName() == FQuestText::GetTextPropertyName())
		{
			LongDescription.NotifyEditText();
		}

		if (!bUseCustomObserver && Observer && !Observer->IsA<UObjectiveObserver_VariableQuery>())
		{
			Observer = nullptr;
		}

		if (!IsValid(Observer))
		{
			Observer = NewObject<UObjectiveObserver_VariableQuery>(this);
		}
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif
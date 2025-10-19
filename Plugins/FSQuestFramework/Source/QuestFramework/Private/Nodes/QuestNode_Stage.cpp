// Copyright Sami Kangasmaa, 2022


#include "Nodes/QuestNode_Stage.h"
#include "QuestContext.h"

TArray<UQuestObjective*> UQuestNode_Stage::GetActiveObjectives(UQuestContext* Context) const
{
	TArray<UQuestObjective*> Output;
	if (Context)
	{
		for (UQuestObjective* Objective : Objectives.GetObjectives())
		{
			if (Objective && Context->IsObjectiveActive(Objective))
			{
				Output.Add(Objective);
			}
		}
	}
	return Output;
}

void UQuestNode_Stage::GetAllObjectiveNames(TArray<FName>& OutNames) const
{
	for (const UQuestObjective* Objective : Objectives.GetObjectives())
	{
		if (Objective)
		{
			OutNames.Add(Objective->LogicalName);
		}
	}
}

UQuestObjective* UQuestNode_Stage::FindObjectiveByName(const FName& Name) const
{
	for (UQuestObjective* Objective : Objectives.GetObjectives())
	{
		if (Objective && Objective->LogicalName == Name)
		{
			return Objective;
		}
	}
	return nullptr;
}

TArray<UQuestService*> UQuestNode_Stage::GetInstancedServices() const
{
	TArray<UQuestService*> OutServices;
	for (UQuestService* Service : Services)
	{
		if (Service && Service->IsInstanced())
		{
			OutServices.Add(Service);
		}
	}
	return OutServices;
}

TArray<UQuestService*> UQuestNode_Stage::GetNonInstancedServices() const
{
	TArray<UQuestService*> OutServices;
	for (UQuestService* Service : Services)
	{
		if (Service && !Service->IsInstanced())
		{
			OutServices.Add(Service);
		}
	}
	return OutServices;
}

int32 UQuestNode_Stage::EvaluateObjectives(UQuestContext* Context, EQuestResult& OutResult, TMap<UQuestObjective*, EQuestResult>& OutObjectiveResults) const
{
	OutResult = EQuestResult::InProgress;

	if (!ensure(Context))
	{
		return QUEST_INDEX_NONE;
	}

	// Stage should never have more or less than 2 children entries
	if (!ensure(GetChildren().Num() == 2))
	{
		return QUEST_INDEX_NONE;
	}
	
	// Distribute objectives into temporal arrays based on their result
	TArray<const UQuestObjective*, TInlineAllocator<8>> Completed;
	TArray<const UQuestObjective*, TInlineAllocator<8>> Failed;
	TArray<const UQuestObjective*, TInlineAllocator<8>> InProgress;

	const TArray<UQuestObjective*>& ObjectiveArray = Objectives.GetObjectives();

	// Determine completed and failed objectives first
	for (int32 Idx = 0; Idx < ObjectiveArray.Num(); Idx++)
	{
		const UQuestObjective* Objective = ObjectiveArray[Idx];
		if (!Objective)
		{
			continue;
		}

		if (!Objective->IsActive(Context))
		{
			continue;
		}

		const EQuestResult ObjectiveResult = Objective->EvaluateObserver(Context);
		if (ObjectiveResult == EQuestResult::Completed)
		{
			Completed.Add(Objective);
		}
		else if (ObjectiveResult == EQuestResult::Failed)
		{
			Failed.Add(Objective);
		}
		else
		{
			InProgress.Add(Objective);
		}
		OutObjectiveResults.Add(const_cast<UQuestObjective*>(Objective), ObjectiveResult);
	}

	// Helper lambda to conver result into node index of corresponding child node
	auto GetChildNodeIndexForResult = [this](EQuestResult Res) -> int32
	{
		check(Res != EQuestResult::NotStarted);
		const int32 ChildIndex = Res == EQuestResult::InProgress ? QUEST_INDEX_NONE : (Res == EQuestResult::Completed ? 0 : 1); // 0 = completed child, 1 = failed child

		check(GetChildren().Num() == 2);
		return GetChildren()[ChildIndex];
	};

	// Check first if there's force fail
	for (const UQuestObjective* Objective : Failed)
	{
		if (Objective->Significancy == EQuestObjectiveSignificancy::ForceFailure ||
			Objective->Significancy == EQuestObjectiveSignificancy::ForceCompleteAndFailure ||
			Objective->Significancy == EQuestObjectiveSignificancy::RequiredToComplete)
		{
			OutResult = EQuestResult::Failed;
			return GetChildNodeIndexForResult(EQuestResult::Failed);
		}
	}

	// Check if we are required to complete objectives still
	for (const UQuestObjective* Objective : InProgress)
	{
		if (Objective->Significancy == EQuestObjectiveSignificancy::RequiredToComplete)
		{
			return QUEST_INDEX_NONE; // There're still objective in progress that need to be completed
		}
	}

	// Check then for force completes
	for (const UQuestObjective* Objective : Completed)
	{
		if (Objective->Significancy == EQuestObjectiveSignificancy::ForceComplete ||
			Objective->Significancy == EQuestObjectiveSignificancy::ForceCompleteAndFailure)
		{
			OutResult = EQuestResult::Completed;
			return GetChildNodeIndexForResult(EQuestResult::Completed);
		}
	}


	bool bNonOptionalFailed = false;
	for (const UQuestObjective* Objective : Failed)
	{
		if (Objective->Significancy != EQuestObjectiveSignificancy::Optional)
		{
			bNonOptionalFailed = true; // There's objective failed which is non optional
			break;
		}
	}

	bool InProgressEmpty = true;
	for (const UQuestObjective* Objective : InProgress)
	{
		if (Objective->Significancy != EQuestObjectiveSignificancy::Optional)
		{
			InProgressEmpty = false; // There are non optionals still in progress
			break;
		}
	}

	// There are no more objectives in progress
	if (InProgressEmpty)
	{
		// Completed result if not any objective has failed, failed if one of the objective was failed
		OutResult = bNonOptionalFailed ? EQuestResult::Failed : EQuestResult::Completed;
		return GetChildNodeIndexForResult(OutResult);
	}

	OutResult = EQuestResult::InProgress;
	return QUEST_INDEX_NONE;
}

#if WITH_EDITOR

void UQuestNode_Stage::SuggestLogicalName(const FName& InName)
{
	if (LogicalName.IsNone())
	{
		LogicalName = InName;
	}
}

void UQuestNode_Stage::CreateObjectiveObjects()
{
	TArray<TObjectPtr<UQuestObjective>>& ObjectiveArr = Objectives.ObjectiveArray;
	for (int32 Idx = 0; Idx < ObjectiveArr.Num(); Idx++)
	{
		if (!IsValid(ObjectiveArr[Idx]))
		{
			ObjectiveArr[Idx] = NewObject<UQuestObjective>(this);
		}
	}
}

void UQuestNode_Stage::GenerateMissingObjectiveNames()
{
	for (UQuestObjective* Objective : Objectives.GetObjectives())
	{
		if (!Objective)
		{
			continue;
		}
		const FString NextNameStr = FString(TEXT("Objective")) + FString::FromInt(NextObjectiveNameIndex);
		if (Objective->SuggestLogicalName(FName(*NextNameStr)))
		{
			NextObjectiveNameIndex++;
		}
	}
}

bool UQuestNode_Stage::HasUniqueObjectiveNames() const
{
	TSet<FName> Names;
	for (const UQuestObjective* Objective : Objectives.GetObjectives())
	{
		if (Objective->LogicalName.IsNone())
		{
			return false;
		}
		if (Names.Contains(Objective->LogicalName))
		{
			return false;
		}
		Names.Add(Objective->LogicalName);
	}
	return true;
}

void UQuestNode_Stage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	CreateObjectiveObjects();
	GenerateMissingObjectiveNames();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UQuestNode_Stage::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	CreateObjectiveObjects();
	GenerateMissingObjectiveNames();

	FProperty* Property = PropertyChangedEvent.PropertyChain.GetActiveNode()->GetValue();
	FProperty* MemberProperty = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue();
	if (Property && MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		static FName NAME_StageNameProperty = GET_MEMBER_NAME_CHECKED(UQuestNode_Stage, StageName);
		static FName NAME_StageDescProperty = GET_MEMBER_NAME_CHECKED(UQuestNode_Stage, StageDescription);

		if (MemberProperty->GetFName() == NAME_StageNameProperty && Property->GetFName() == FQuestText::GetTextPropertyName())
		{
			StageName.NotifyEditText();
		}
		else if (MemberProperty->GetFName() == NAME_StageDescProperty && Property->GetFName() == FQuestText::GetTextPropertyName())
		{
			StageDescription.NotifyEditText();
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif
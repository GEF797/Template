// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogBlueprintFunctionLibrary.h"
#include "DialogTaskBlueprintBase.h"





FName UDialogBlueprintFunctionLibrary::GetStateName(FStateTreeStateHandle StateHandle, UStateTree* StateTree)
{
	FName Name;
	if (StateTree)
	{
		if (const FCompactStateTreeState* State = StateTree->GetStateFromHandle(StateHandle))
		{
			Name = State->Name;

			uint16 i = State->TasksBegin;

			while (i != 0 && i < State->TasksNum)
			{
				
			


				//i = ChildrenState.GetNextSibling();
			}

		}
	}


	return Name;
}

bool UDialogBlueprintFunctionLibrary::GetChildrenStateNames(FStateTreeStateHandle StateHandle, UStateTree* StateTree, TArray<FName>& Names, TArray<FStateTreeStateHandle>& TreeStates)
{
	if (StateTree)
	{

		if (const FCompactStateTreeState* State = StateTree->GetStateFromHandle(StateHandle))
		{
			if (State->HasChildren())
			{
				uint16 i = State->ChildrenBegin;

				while (i != 0 && i < State->ChildrenEnd && StateTree->GetStates().IsValidIndex(i))
				{
					FCompactStateTreeState ChildrenState = StateTree->GetStates()[i];
					TreeStates.Add(ChildrenState.ParameterDataHandle.GetState());
					Names.Add(ChildrenState.Name);

					
					i = ChildrenState.GetNextSibling();
				}

				return true;
			}



		}
	}

	return false;
}

bool UDialogBlueprintFunctionLibrary::StateHaschildren(FStateTreeStateHandle StateHandle, UStateTree* StateTree)
{
	if (StateTree)
	{

		if (const FCompactStateTreeState* State = StateTree->GetStateFromHandle(StateHandle))
		{
			if (State->HasChildren())
			{
				return true;
			}
		}
	}
	return false;
}

bool UDialogBlueprintFunctionLibrary::GetGetNextSibling(FStateTreeStateHandle StateHandle, UStateTree* StateTree, FStateTreeStateHandle& NextStateHandle)
{
	if (StateTree)
	{
		
		if (const FCompactStateTreeState* State = StateTree->GetStateFromHandle(StateHandle))
		{
			int Index = State->GetNextSibling();
			int Max = 0;
			if (const FCompactStateTreeState* ParentState = StateTree->GetStateFromHandle(State->Parent))
			{
				Max=ParentState->ChildrenEnd;
			
			if (Index<Max)
			{
				FCompactStateTreeState NextState = StateTree->GetStates()[Index];
				//NextStateHandle=NextState.ParameterDataHandle.GetState();
				const FStateTreeStateHandle CompactStateHandle(Index);
				NextStateHandle = CompactStateHandle;
				return true;
			}
			}
		}
	}
	return false;
}

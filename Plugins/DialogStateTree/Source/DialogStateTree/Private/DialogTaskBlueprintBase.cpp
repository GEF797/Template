// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogTaskBlueprintBase.h"

void UDialogTaskBlueprintBase::DialogRequestTransition(const FStateTreeStateHandle& TargetState, const EStateTreeTransitionPriority Priority, EStateTreeSelectionFallback Fallback)
{
	

	if (!GetWeakExecutionContext().RequestTransition(TargetState, Priority, Fallback))
	{
		UE_VLOG_UELOG(this, LogStateTree, Error, TEXT("Failed to request a transition. The instance probably stopped."));
	}
}

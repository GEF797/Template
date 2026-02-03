// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "DialogTaskBlueprintBase.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGSTATETREE_API UDialogTaskBlueprintBase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintCallable)
	void DialogRequestTransition(const FStateTreeStateHandle& TargetState, const EStateTreeTransitionPriority Priority, EStateTreeSelectionFallback Fallback);



};

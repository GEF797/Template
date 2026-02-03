// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/StateTreeComponent.h"
#include "StateTree.h"
#include "DialogBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGSTATETREE_API UDialogBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	static FName GetStateName(FStateTreeStateHandle StateHandle, UStateTree* StateTree);

	UFUNCTION(BlueprintCallable)
	static bool GetChildrenStateNames(FStateTreeStateHandle StateHandle, UStateTree* StateTree, TArray<FName>& Names, TArray<FStateTreeStateHandle>& TreeStates);


	UFUNCTION(BlueprintCallable)
	static bool StateHaschildren(FStateTreeStateHandle StateHandle, UStateTree* StateTree);

	UFUNCTION(BlueprintCallable)
	static bool GetGetNextSibling(FStateTreeStateHandle StateHandle, UStateTree* StateTree, FStateTreeStateHandle& NextStateHandle);
	
};

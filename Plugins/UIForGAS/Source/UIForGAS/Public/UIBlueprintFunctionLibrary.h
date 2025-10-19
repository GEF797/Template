// Copyright G.E.FC 2023

#pragma once


#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class  UUIBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

	/** Returns all defined Gameplay Attributes for the provided AttributeSet class */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UIBlueprintFunctionLibrary")
		static void GetAllAttributes(TSubclassOf<UAttributeSet> AttributeSetClass, TArray<FGameplayAttribute>& OutAttributes);



};

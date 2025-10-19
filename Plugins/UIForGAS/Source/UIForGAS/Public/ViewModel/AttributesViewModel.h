// Copyright G.E.FC 2023

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "ViewModel/AttributeViewModel.h"
#include "AttributesViewModel.generated.h"



UCLASS(Transient,BlueprintType)
class UIFORGAS_API UAttributesViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UAttributesViewModel(){};

	UFUNCTION(BlueprintCallable, Category = "UIForGAS")
	void SetAbilitySystem(UAbilitySystemComponent* AbilitySystemComponent);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "UIForGAS")
	UAbilitySystemComponent* GetAbilitySystem() const { return AbilitySystem.Get(); };

	UFUNCTION(BlueprintPure, FieldNotify, Category = "UIForGAS")
	TArray<FGameplayAttribute> GetAttributes() const { return Attributes; }

	UFUNCTION()
	void OnRep_AbilitySystem(TWeakObjectPtr<UAbilitySystemComponent> OldAbilitySystem);
	
protected:

	void HandleChanged(const FOnAttributeChangeData& ChangeData);



private:

	UPROPERTY()
	TArray<FGameplayAttribute> Attributes;

	UPROPERTY( ReplicatedUsing = OnRep_AbilitySystem)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
};

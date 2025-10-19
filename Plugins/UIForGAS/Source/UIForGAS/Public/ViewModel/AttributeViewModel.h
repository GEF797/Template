// Copyright G.E.FC 2023

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystemComponent.h"
#include "Logging/LogMacros.h"
#include "AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AttributeViewModel.generated.h"



UCLASS(BlueprintType)
class UIFORGAS_API UAttributeViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UAttributeViewModel() {};


	UFUNCTION(BlueprintPure, FieldNotify, Category = "UIForGAS")
		float GetAttributeValue() const
	{
		if (ASC && Attribute.IsValid())
		{
			
			const UAttributeSet* AttributeSet = ASC->GetAttributeSet(Attribute.GetAttributeSetClass());
			if (AttributeSet)
			{
				return Attribute.GetNumericValue(AttributeSet);
			}
			else
			{
				return 0.0f;
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("AbilitySystem or AttributeSet is nullptr"));
			return 0.0f;
		}
	}

	//	UFUNCTION(BlueprintCallable, Category = "UIForGAS")
	//void MarkAsGarbage(){ this->MarkAsGarbage(); };

	//Set AbilitySystemComponent And Attribute
	UFUNCTION(BlueprintCallable, Category = "UIForGAS")
		void SetAbilitySystem(UAbilitySystemComponent* _ASC)
	{

		if (ASC)
		{
			
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);

		}

		ASC = _ASC;
		if (!ASC)
		{
			UE_LOG(LogTemp, Warning, TEXT("ASC is nullptr"));
		  return;
		}
		
			Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleChanged);

			UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributeValue);

		
	}

	void HandleChanged(const FOnAttributeChangeData& ChangeData);


	virtual void BeginDestroy() override;

	UFUNCTION()
	void OnRep_ASC(UAbilitySystemComponent* OldASC);

protected:

	UPROPERTY(BlueprintReadOnly,Meta = (ExposeOnSpawn = true), Replicated, Category = "UIForGAS")
	FGameplayAttribute Attribute;

private:
	//UPROPERTY(Transient, ReplicatedUsing = OnRep_ASC)
	TObjectPtr <UAbilitySystemComponent> ASC;

	FDelegateHandle  Handle;



};

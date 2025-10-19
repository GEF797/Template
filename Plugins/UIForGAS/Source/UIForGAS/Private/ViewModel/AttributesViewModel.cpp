// Copyright G.E.FC 2023

#include "ViewModel/AttributesViewModel.h"



void UAttributesViewModel::SetAbilitySystem(UAbilitySystemComponent* AbilitySystemComponent)
{
	AbilitySystem = AbilitySystemComponent;

	AbilitySystem->GetAllAttributes(Attributes);

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributes);

	for(FGameplayAttribute Attribute:Attributes)
	{
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleChanged);
	}
}

void UAttributesViewModel::OnRep_AbilitySystem(TWeakObjectPtr<UAbilitySystemComponent> OldAbilitySystem)
{
	//if (OldAbilitySystem)
	//{
	//	for (FGameplayAttribute Attribute : Attributes)
	//	{
	//		OldAbilitySystem->GetGameplayAttributeValueChangeDelegate(Attribute).Remove();
	//	}
	//}
	
	if (AbilitySystem.Get())
	{
		for (FGameplayAttribute Attribute : Attributes)
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleChanged);
		}
	}


	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributes);
}

void UAttributesViewModel::HandleChanged(const FOnAttributeChangeData& ChangeData)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributes);
}


void UAttributesViewModel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AbilitySystem);
}
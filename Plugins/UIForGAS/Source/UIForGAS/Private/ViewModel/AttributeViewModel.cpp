// Copyright G.E.FC 2023


#include "ViewModel/AttributeViewModel.h"


void UAttributeViewModel::HandleChanged(const FOnAttributeChangeData& ChangeData)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributeValue);
}

void UAttributeViewModel::BeginDestroy()
{
   Super::BeginDestroy();

   if (ASC)
   {
	   ASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
	  
   }
   ASC = nullptr;
}

void UAttributeViewModel::OnRep_ASC(UAbilitySystemComponent* OldASC)
{
	if (OldASC)
	{
		OldASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);

	}
	if (ASC)
	{
		Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleChanged);
	}
	

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAttributeValue);
}


void UAttributeViewModel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ASC);
}
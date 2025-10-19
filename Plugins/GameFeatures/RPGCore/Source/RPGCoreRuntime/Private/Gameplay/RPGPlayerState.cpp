// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/RPGPlayerState.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;




ARPGPlayerState::ARPGPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<URPGAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


	SetNetUpdateFrequency(100.0f);


}




ARPGPlayerController* ARPGPlayerState::GetRPGPlayerController() const
{
	return Cast<ARPGPlayerController>(GetOwner());
}

UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return GetRPGAbilitySystemComponent();
}

//void ARPGPlayerState::SetPawnData(const URPGPawnData* InPawnData)
//{
//	check(InPawnData);
//
//	if (GetLocalRole() != ROLE_Authority)
//	{
//		return;
//	}
//
//	if (PawnData)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
//		return;
//	}
//
//	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
//	PawnData = InPawnData;
//
//
//
//	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_RPGAbilityReady);
//
//	ForceNetUpdate();
//}

void ARPGPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ARPGPlayerState::PostInitializeComponents()
{
	Super::Super::PostInitializeComponents();

	check(GetAbilitySystemComponent());
	GetAbilitySystemComponent()->InitAbilityActorInfo(this, GetPawn());


	K2_PostInitializeComponents();
}



void ARPGPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}


}

void ARPGPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
}







FGenericTeamId ARPGPlayerState::GetGenericTeamId() const
{
	if (const IGenericTeamAgentInterface* Agent = Cast< IGenericTeamAgentInterface>(GetPawn()))
	{
		return Agent->GetGenericTeamId();
	}
	return FGenericTeamId();
}









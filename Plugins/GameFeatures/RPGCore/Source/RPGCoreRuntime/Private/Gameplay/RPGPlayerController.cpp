// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/RPGPlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	if (const IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(GetPawn()))
	{
		return TeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

void ARPGPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (GetPawn())
	{
		if (IAbilitySystemInterface* IASInterface = Cast<IAbilitySystemInterface>(GetPawn()))
		{
			if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(IASInterface->GetAbilitySystemComponent()))
			{
				ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
		}
	}


	Super::PostProcessInput(DeltaTime, bGamePaused);
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "RPGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RPGCORERUNTIME_API ARPGPlayerController : public ACommonPlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	virtual FGenericTeamId GetGenericTeamId() const override;
};

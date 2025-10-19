// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "Gameplay/RPGPlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "RPGPlayerState.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class RPGCORERUNTIME_API ARPGPlayerState : public AModularPlayerState,  public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	ARPGPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	


	UFUNCTION(BlueprintCallable, Category = "RPG|PlayerState")
	ARPGPlayerController* GetRPGPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|PlayerState")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//template <class T>
	//const T* GetPawnData() const { return Cast<T>(PawnData); }

	//void SetPawnData(const URPGPawnData* InPawnData);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface


	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	//~End of APlayerState interface


	virtual FGenericTeamId GetGenericTeamId() const override;

protected:


	UFUNCTION(BlueprintImplementableEvent, Category = "AdvancedGASSystem")
	void K2_PostInitializeComponents();


private:
	//void OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);

//protected:
//	UFUNCTION()
//	void OnRep_PawnData();
//
//protected:
//
//	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
//	TObjectPtr<const URPGPawnData> PawnData;
//
//private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "RPG|PlayerState")
	TObjectPtr<URPGAbilitySystemComponent> AbilitySystemComponent;





	
};

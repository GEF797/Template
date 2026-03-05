// Copyright G.E.FC 2023.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/RPGInventoryManager.h"
#include "Equipment/RPGEquipmentManager.h"
#include "System/GeneralSettingSubsystem.h"
#include "TimerManager.h"
#include "AbilitySystem/AbilityDefinition.h"
#include "Engine/DataTable.h"
#include "GenericTeamAgentInterface.h"
#include "RPGSaveGame.generated.h"

class URPGPawnData;




USTRUCT(BlueprintType)
struct FInitializeData : public FTableRowBase
{
	GENERATED_BODY()


public:
	FInitializeData() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MaxHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Health = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MaxMana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Mana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MaxEP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float EP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float UpgradeExp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Exp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Attack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Defense = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MagicAttack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MagicDefense = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Critical = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float CriticalRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Penetrated = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float MagicPenetrated = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RPG|DataAsset")
	float Speed = 0.0f;


};


USTRUCT(Blueprintable)
struct FAbility
{
	GENERATED_BODY()

public:
	FAbility() {  }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	FPrimaryAssetId Ability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	int32 Level = 0;
};






/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType)
class  URPGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame, Category = Save)
	TSoftObjectPtr<UWorld> Level;



		UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame, Category = Save)
			FDateTime Time;

		UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame, Category = Save)
			float GameTime;

			UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame, Category = Save)
			TArray<FSaveData> SaveData;

			//Save level soft references
		UFUNCTION(BlueprintCallable, BlueprintPure = false, BlueprintAuthorityOnly, Category = Save)
			void SaveWorld(AActor* Context);


};

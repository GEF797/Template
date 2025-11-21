// Copyright G.E.FC 2023.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/RPGInventoryManager.h"
#include "Equipment/RPGEquipmentManager.h"
#include "System/GeneralSettingSubsystem.h"
#include "TimerManager.h"
#include "AbilitySystem/AbilityDefinition.h"
#include "GenericTeamAgentInterface.h"
#include "RPGSaveGame.generated.h"

class URPGPawnData;


USTRUCT(Blueprintable)
struct FAbility
{
	GENERATED_BODY()

public:
	FAbility() {  }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	TSoftObjectPtr<UAbilityDefinition> Ability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	int32 Level = -1;
};


USTRUCT(Blueprintable)
struct FInventoryItem
{
	GENERATED_BODY()

public:
	FInventoryItem() {  }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	FPrimaryAssetId Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	int32 Count=0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	int32 Index=-1;
};

USTRUCT(Blueprintable)
struct FEquipment
{
	GENERATED_BODY()

public:
	FEquipment() {  }

	UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame, Category = Save)
	FPrimaryAssetId Equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save, Meta = (Categories = "Gameplay.EquipmentSlot"))
	FGameplayTag SlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	FGameplayTagContainer AvailableTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save, Meta = (Categories = "Gameplay.EquipmentSlot.Weapon"))
	bool Activate=false;
};



/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType)
class  URPGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
	TSoftObjectPtr<UWorld> Level;



		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
			FDateTime Time;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
			float GameTime;

			UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
			TArray<FSaveData> SaveData;

			//Save level soft references
		UFUNCTION(BlueprintCallable, BlueprintPure = false, BlueprintAuthorityOnly, Category = Save)
			void SaveWorld(AActor* Context);


};

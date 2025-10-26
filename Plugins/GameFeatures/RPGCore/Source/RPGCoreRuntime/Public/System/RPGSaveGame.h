// Copyright G.E.FC 2023.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/RPGInventoryManager.h"
#include "Equipment/RPGEquipmentManager.h"
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
	TSoftObjectPtr<URPGInventoryItem> Item;

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
	TSoftObjectPtr<URPGInventoryItem> Equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save, Meta = (Categories = "Gameplay.EquipmentSlot"))
	FGameplayTag SlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save)
	FGameplayTagContainer AvailableTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Save, Meta = (Categories = "Gameplay.EquipmentSlot.Weapon"))
	bool Activate=false;
};

//USTRUCT(Blueprintable)
//struct FHeroData
//{
//
//	GENERATED_BODY()
//
//public:
//	FHeroData() { TeamId = FGenericTeamId(); }
//
//
//
//	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Save)
//	TObjectPtr<URPGPawnData> Definition;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	int32 Level=1;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	  FTransform Position;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		TArray<FEquipment> EquipmentEntries;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	FGameplayTag CurrentSlot;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	float Health=0.0f;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	float Mana = 0.0f;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//	float EP = 0.0f;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		float EXP = 0.0f;
//
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		float AbilityPoints = 0.0f;
//
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		float CostAbilityPoints = 0.0f;
//
//		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		TArray<FAbility> Abilities ;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
//		FGenericTeamId TeamId;
//};

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

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
		//	TArray<FInventoryItem> InventoryItemList;

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
		//	int32 Money;

	 //   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
		//TArray<FHeroData> Heros;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
			FDateTime Time;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
			float GameTime;

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
		//	FGenericTeamId TeamId;

			//Save level soft references
		UFUNCTION(BlueprintCallable, BlueprintPure = false, BlueprintAuthorityOnly, Category = Save)
			void SaveWorld(AActor* Context);


};

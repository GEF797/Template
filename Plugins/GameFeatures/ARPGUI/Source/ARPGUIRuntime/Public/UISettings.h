// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UISettings.generated.h"

/**
 * 
 */
UCLASS(config = "UISettings", defaultconfig, meta = (DisplayName = "UISettings"))
class  UUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UUISettings() {};

	UFUNCTION(BlueprintPure, Category = "UI|Config")
	static const UUISettings* GetUISettingsInstance();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "UI|Config|InventoryGrid")
	int32 GridSize = 64;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "UI|Config|InventoryGrid")
	FLinearColor EnableColor = FLinearColor(0,1,0,0.4f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "UI|Config|InventoryGrid")
	FLinearColor DisableColor = FLinearColor(1, 0, 0, 0.4f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "UI|Config|InventoryGrid")
	FLinearColor BGColor = FLinearColor(0, 0, 0, 0.4f);
	
};

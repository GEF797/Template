// Copyright G.E.FC 2023.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

//#include "GameUIPolicy.h"

#include "RPGCoreSettings.generated.h"



class URPGSaveGame;
class URPGPawnData;

/**
 * 
 */
UCLASS(config = "RPGSettings", defaultconfig, meta = (DisplayName = "RPGDefault Settings"))
class  URPGCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	URPGCoreSettings(){};

	UFUNCTION(BlueprintPure, Category = "RPG|Config")
	static const URPGCoreSettings* GetRPGCoreSettingsInstance();


};

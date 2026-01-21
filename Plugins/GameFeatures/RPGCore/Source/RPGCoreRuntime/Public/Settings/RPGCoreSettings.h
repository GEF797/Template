// Copyright G.E.FC 2023.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

//#include "GameUIPolicy.h"

#include "RPGCoreSettings.generated.h"



class URPGSaveGame;
class URPGPawnData;

 
USTRUCT(BlueprintType)
struct FLevelData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Displayname;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UTexture2D> Image;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> GameFeatures;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowedClasses = "/Script/Engine.World"))
	FPrimaryAssetId Map;



};


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

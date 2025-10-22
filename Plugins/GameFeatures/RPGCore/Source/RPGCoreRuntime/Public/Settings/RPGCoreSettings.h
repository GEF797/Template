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


	//UPROPERTY(config, EditAnywhere ,Category = "UI")
	//TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;

		//UFUNCTION(BlueprintCallable, Category = "Game")
		//TSoftObjectPtr<URPGGameData> GetGameData() const{
		//	if (GetWorld())
		//	{
		//	if (GetWorld()->GetGameState())
		//	{
		//	if(ULyraExperienceManagerComponent* LyraExperienceManagerComponent = GetWorld()->GetGameState()->GetComponentByClass<ULyraExperienceManagerComponent>())
		//	{
		//		if (const ULyraExperienceDefinition* LyraExperienceDefinition =LyraExperienceManagerComponent->GetCurrentExperienceChecked())
		//		{
		//			if (const URPGExperienceDefinition* RPGExperienceDefinition=Cast<URPGExperienceDefinition>(LyraExperienceDefinition))
		//			{
		//				if (RPGExperienceDefinition->GameData.IsValid())
		//				{
		//					return RPGExperienceDefinition->GameData;
		//				}
		//			}

		//			//if( TObjectPtr<const URPGPawnData> PawnData=Cast<const URPGPawnData>(LyraExperienceDefinition->DefaultPawnData))
		//			//{

		//			//	//return PawnData.Get()->GetSoft();
		//			//}
		//		}
		//		}
		//	}
		//	}
		//	
		//
		//		return DefaultGameData;
		//	
		//};

protected:
		/*UPROPERTY(EditDefaultsOnly, config, Category = "Game")
		TSoftObjectPtr<URPGGameData> DefaultGameData;*/
};

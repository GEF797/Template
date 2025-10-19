// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Engine/DeveloperSettings.h"

#include "QuestFrameworkSettings.generated.h"

/**
 * Settings for Quest Framework editable in project settings menu
 */
UCLASS(Config = Game, DefaultConfig)
class QUESTFRAMEWORK_API UQuestFrameworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:

	virtual FName GetContainerName() const override 
	{ 
		return TEXT("Project"); 
	}
	
	virtual FName GetCategoryName() const override 
	{ 
		return TEXT("Plugins"); 
	}

	virtual FName GetSectionName() const override
	{
		return TEXT("Quest Framework");
	}

#if WITH_EDITOR

	virtual FText GetSectionText() const override
	{
		static FText QuestFrameworkSettingsName = FText::FromString(TEXT("Quest Framework"));
		return QuestFrameworkSettingsName;
	}

	virtual bool SupportsAutoRegistration() const override { return true; }
#endif

public:
	
	/**
	 * Paths to search for quest assets
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Assets")
	TArray<FString> QuestAssetPaths;

	/**
	 * View model class to use with quest contexts.
	 * If not set, UQuestContextViewModel is used by default
	 */
	UPROPERTY(Config, EditAnywhere, Category = "View Models")
	TSubclassOf<class UQuestContextViewModel> QuestContextViewModelClass;

	/**
	 * View model class to use with objectives.
	 * If not set, UQuestObjectiveViewModel is used by default
	 */
	UPROPERTY(Config, EditAnywhere, Category = "View Models")
	TSubclassOf<class UQuestObjectiveViewModel> QuestObjectiveViewModelClass;

	/**
	* View model class to use with quest tracker.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "View Models")
	TSubclassOf<class UQuestTrackerViewModel> QuestTrackerViewModelClass;

	/**
	* View model class to use with quest list.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "View Models")
	TSubclassOf<class UQuestListViewModel> QuestListViewModelClass;
};

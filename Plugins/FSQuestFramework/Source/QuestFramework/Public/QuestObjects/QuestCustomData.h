// Copyright Sami Kangasmaa, 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestCustomData.generated.h"

class UQuest;

/**
 * Custom data to embed into quest assets.
 * Use this to add additional game specific information to the quest. (rewards of the quest etc.)
 */
UCLASS(Const, Abstract, Blueprintable, EditInlineNew)
class QUESTFRAMEWORK_API UQuestCustomData : public UObject
{
	GENERATED_BODY()

public:

	UQuestCustomData() {}

	/**
	* Returns quest outer of this object
	*/
	UFUNCTION(BlueprintPure, Category = "Quest Custom Data")
	UQuest* GetQuestAsset() const;
};

// Copyright G.E.FC 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/MenuAnchor.h"

#include "RPGMenuAnchor.generated.h"

/**
 * 
 */
UCLASS()
class  URPGMenuAnchor : public UMenuAnchor
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "UI|RPGInventorySystem")
	TObjectPtr<UWidget> Widget;

	protected:

		virtual TSharedRef<SWidget> RebuildWidget() override;

		TSharedRef<SWidget> HandleGetMenuContent();

};

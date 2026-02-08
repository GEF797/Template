// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"


#include "Components/SkeletalMeshComponent.h" 

#include "DragonIKTransformReceiverComp.generated.h"





UCLASS( ClassGroup=(DragonIK), meta=(DisplayName = "DragonIK Transform Receiver Component",BlueprintSpawnableComponent) )
class DRAGONIKPLUGIN_API UDragonIKTransformReceiverComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	
	// Sets default values for this component's properties
	UDragonIKTransformReceiverComp();

	TArray<FTransform> Raw_Animation_Transforms;

	UPROPERTY()
		TObjectPtr<USkeletalMeshComponent> owning_skeleton;



	/*
*
* This component lets you receive the pure bone transforms of the character from a specific point in the animation blueprint.
* This relies on the DragonIK Transform Relayer that we connect in the animgraph of your animation blueprint, and the DragonIK Transform Receiver we connect in the pawn/character blueprint.
* The DragonIK Transform Relayer transmits the bone transform of the pose data to this Transform Receiver.
* These bone transforms are unchanged by aspects like physical animations and procedural animations.
*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CoreInput, meta = (DisplayName = "HOVER MOUSE HERE FOR TIPS!", PinHiddenByDefault))
	FName read_this = "A description on how to use the dragonik transform receiver component";




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreOutputs, meta = (DisplayName = "Bone-Transform Map (World Space)"))
		TMap<FName,FTransform> World_Name_Transform_Map;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreOutputs, meta = (DisplayName = "Bone-Transform Map (Component Space)"))
		TMap<FName,FTransform> Component_Name_Transform_Map;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	


		
};

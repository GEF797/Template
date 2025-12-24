/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#pragma once



//#include "DragonIK_Library.h"
#include "CoreMinimal.h"

#include "DragonIKPlugin.h"

#include "Kismet/KismetMathLibrary.h"

#include "CollisionQueryParams.h"
#include "DragonIKTransformReceiverComp.h"

#include "AnimNode_DragonControlBase.h"

#include "Components/SkeletalMeshComponent.h"


#include "AnimNode_DragonTransformRelayer.generated.h"
/**
 * 
 */


//class USkeletalMeshComponent;






USTRUCT(BlueprintType)
struct DRAGONIKPLUGIN_API FAnimNode_DragonTransformRelayer : public FAnimNode_DragonControlBase
{
	//	GENERATED_USTRUCT_BODY()
	GENERATED_BODY()

public:






	FAnimNode_DragonTransformRelayer();


	TArray<FBoneReference> character_bone_references;

	TArray<FTransform> character_bone_transforms;

	virtual int32 GetLODThreshold() const override { return LODThreshold; }

	FBoneContainer* SavedBoneContainer;	

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> owning_skel = nullptr;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Transform Receiver Component tag", PinHiddenByDefault))
	TObjectPtr<UDragonIKTransformReceiverComp> transform_receiver_component = nullptr;




	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Transform Receiver Component ID", PinHiddenByDefault))
	//	int physanim_tag_index = 1;


	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreOutputs, meta = (DisplayName = "World Name Transform Map"))
	TMap<FName,FTransform> World_Name_Transform_Map;

	TMap<FName,FTransform> Component_Name_Transform_Map;

	

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
//	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)  override;
	//virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;

	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;

	//virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;

	virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;


	virtual void Evaluate_AnyThread(FPoseContext& Output);


	// use this function to evaluate for skeletal control base
//	virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context);
	// Evaluate the new component-space transforms for the affected bones.
	//	virtual void EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases, TArray<FBoneTransform>& OutBoneTransforms);

//	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);




	// return true if it is valid to Evaluate
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones);
	// initialize any bone references you have
	virtual void InitializeBoneReferences(FBoneContainer& RequiredBones) override;



protected:


};


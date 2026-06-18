/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/

#pragma once



#include "DragonIK_Library.h"
#include "CoreMinimal.h"

#include "DragonIKPlugin.h"

#include "Kismet/KismetMathLibrary.h"

#include "Animation/InputScaleBias.h"
#include "AnimNode_DragonControlBase.h"

#include "CommonAnimTypes.h"
#include "Curves/CurveFloat.h"
#include "CollisionQueryParams.h"

#include "Components/SkeletalMeshComponent.h"


#include "AnimNode_DragonFatSolver.generated.h"

/**
 * 
 */

 

class FPrimitiveDrawInterface;
//class USkeletalMeshComponent;





//USTRUCT(BlueprintInternalUseOnly)
USTRUCT(BlueprintType)
struct DRAGONIKPLUGIN_API FAnimNode_DragonFatSolver : public FAnimNode_DragonControlBase
{
	//	GENERATED_USTRUCT_BODY()
	GENERATED_BODY()



public:


	TMap<FName, TArray<int>> BoneHierarchy;

	/*
	* The bones that is going to be influenced by the fat solver
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Custom Bone Chain Input", PinHiddenByDefault ))
		FDragonData_DragonFatBoneInput fat_bone_input;



	/*
	* Set the individual fat strength multiplier for the custom bone chain input array
	* The index of this array is connected to the index of the custom bone chain input array
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Fat Control Strength Array", PinHiddenByDefault ))
		FDragonData_DragonFatControlInput fat_control_input;


	/*
	* Set the fat strength of all the input bones of this solver.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Fat Strength Value", PinShownByDefault))
		float Fat_Strength_Value = 1;


	/*
	* Use this alpha (0-1) to blend between original pose and fat solver output pose
	* The native alpha parameter has an issue where the bone positions might offset when altering the alpha.
	* This alpha parameter doesn't produce such issues.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", DisplayName = "Fat Alpha Value", PinShownByDefault))
		float Fat_Alpha_Value = 1;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Restrict Scaling Within The Input Bone Axis ?", PinHiddenByDefault))
		bool Use_Axis_Limitation = true;




	TArray<FBoneReference> Character_Bone_References = TArray<FBoneReference>();



	virtual int32 GetLODThreshold() const override { return LODThreshold; }


	float component_scale = 1;




	FBoneContainer* SavedBoneContainer = nullptr;


	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> owning_skel = nullptr;

	


//#if WITH_EDITOR
	void ResizeDebugLocations(int32 NewSize);
//#endif


	virtual void ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* PreviewSkelMeshComp) const;


	//constructor
	//UWheeledVehicleMovementComponent4W::UWheeledVehicleMovementComponent4W(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;

//	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)  override;


	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;


//	virtual void EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output) override;


	virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;

	// initialize any bone references you have
	virtual void InitializeBoneReferences(FBoneContainer& RequiredBones) override;



	virtual void Evaluate_AnyThread(FPoseContext& Output);

	FAnimNode_DragonFatSolver();

protected:

//	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


//	void LineTraceControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


	// return true if it is valid to Evaluate
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones);
	


};


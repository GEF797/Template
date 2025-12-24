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

#include "AnimNode_ChineseDragonSolver.generated.h"

/**
 * 
 */

 

class FPrimitiveDrawInterface;
//class USkeletalMeshComponent;





USTRUCT()
struct FChinaDragonChainLink
{
	GENERATED_USTRUCT_BODY()

public:
	/** Position of bone in component space. */
	FTransform Position;

	FTransform OriginalPosition;

	FTransform InterpolatedPosition;

	FTransform Interpolated_Positions_Final;

	FRotator SavedRotation = FRotator::ZeroRotator;
	
	/** Distance to its parent link. */
	double Length;

	/** Bone Index in SkeletalMesh */
	int32 BoneIndex;

	/** Transform Index that this control will output */
	int32 TransformIndex;

	/** Default Direction to Parent */
	FVector DefaultDirToParent;

	/** Child bones which are overlapping this bone.
	* They have a zero length distance, so they will inherit this bone's transformation. */
	TArray<int32> ChildZeroLengthTransformIndices;

	FChinaDragonChainLink()
		: Position(FTransform::Identity)
		, Length(0.0)
		, BoneIndex(INDEX_NONE)
		, TransformIndex(INDEX_NONE)
		, DefaultDirToParent(FVector(-1.0, 0.0, 0.0))
	{
	}

	FChinaDragonChainLink(const FTransform& InPosition, const double InLength, const FCompactPoseBoneIndex& InBoneIndex, const int32& InTransformIndex)
		: Position(InPosition)
		, Length(InLength)
		, BoneIndex(InBoneIndex.GetInt())
		, TransformIndex(InTransformIndex)
		, DefaultDirToParent(FVector(-1.0, 0.0, 0.0))
	{
	}

	FChinaDragonChainLink(const FTransform& InPosition, const double InLength, const FCompactPoseBoneIndex& InBoneIndex, const int32& InTransformIndex, const FVector& InDefaultDirToParent)
		: Position(InPosition)
		, Length(InLength)
		, BoneIndex(InBoneIndex.GetInt())
		, TransformIndex(InTransformIndex)
		, DefaultDirToParent(InDefaultDirToParent)
	{
	}

	FChinaDragonChainLink(const FTransform& InPosition, const double InLength, const int32 InBoneIndex, const int32 InTransformIndex)
		: Position(InPosition)
		, Length(InLength)
		, BoneIndex(InBoneIndex)
		, TransformIndex(InTransformIndex)
		, DefaultDirToParent(FVector(-1.0, 0.0, 0.0))
	{
	}
};




//USTRUCT(BlueprintInternalUseOnly)
USTRUCT(BlueprintType)
struct DRAGONIKPLUGIN_API FAnimNode_ChineseDragonSolver : public FAnimNode_DragonControlBase
{
	//	GENERATED_USTRUCT_BODY()
	GENERATED_BODY()



public:





	virtual int32 GetLODThreshold() const override { return LODThreshold; }


	float component_scale = 1;

	int solver_counter = 0;



	/*
	 * A custom bone chain lets you freely define any kind of bone order to perform the chinese dragon solving.
	 */
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Use Custom Bone Chain ?",PinHiddenByDefault))
	bool Use_Custom_Bone_Chain = true;


	bool ShouldSolverFail = false;


	
	/** Name of tip bone */
//	UPROPERTY(EditAnywhere, Category = Core, meta = (DisplayName = "Tip Bone (Eg:- Head)",PinHiddenByDefault,EditCondition = "!Use_Custom_Bone_Chain"))
	FBoneReference TipBone;

	/** Name of the root bone*/
//	UPROPERTY(EditAnywhere, Category = Core, meta = (DisplayName = "Root Bone (Eg:- Pelvis)",PinHiddenByDefault,EditCondition = "!Use_Custom_Bone_Chain"))
	FBoneReference RootBone;




	TArray<FBoneReference> Custom_BoneList_Reference = TArray<FBoneReference>();



	/*
	 * The custom bone chain input.
	 * The 0th element is the "root" bone and the last element is the "tip" bone.
	 * DO NOT ADD YOUR ACTUAL ROOT BONE HERE.
	 * You can add bones between the pelvis to your head, or even from the tail tip to your head if the tail needs the same effect.
	 * Bone order only matters in a visual sense.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Custom Bone Chain Input",PinHiddenByDefault,EditCondition = "Use_Custom_Bone_Chain"))
		FDragonData_ChineseDragonBoneInput Custom_Bone_Input;







	/*
	 * Only used if using custom bone chains.
	 * Set the correct array index corresponding to your custom bone chain's main "pelvis" bone
	 * Eg:- If pelvis is in array index 3 of the custom bone chain, set it 3 here.
	 * This ensures to make the solver know what is the correct pelvis bone even in a complicated custom bone chain involving head & tail bones.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Core Pelvis Array Index (for stability purposes)", PinHiddenByDefault, EditCondition = "Use_Custom_Bone_Chain"))
		int Stability_Reference_Bone_Index = -1;
	

	/** Coordinates for target location of tip bone
	 *  If using advanced strict logic, then the rotation and scales are used for the chain logic
	 *  If using simple logic, only the location is used for the main chain except for the tip bone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (PinShownByDefault))
		FTransform EffectorTransform = FTransform::Identity;




	/*
	 * This is only used if custom bone chains is enabled.
	 * This provides a rotation offset to specific bone names in the custom bone chain.
	 * The index of the rotation offset correspond to the index of the custom bone chain.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotationOffsets, meta = (DisplayName = "Extra Rotation Offset Per Bone", PinHiddenByDefault, EditCondition = "Use_Custom_Bone_Chain"))
		FDragonData_ChineseDragonBoneRotationOffset Extra_Rotation_Offset_Per_Bone;


	/*
	* This provides a rotation offset to all the bones except for the tip head
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotationOffsets, meta = (DisplayName = "Extra Rotation Offset Overall (except head)", PinHiddenByDefault))
		FRotator Extra_Rotation_Offset_Overall = FRotator(0, 0, 0);


	/*
	 * This provides a rotation offset to the tip head bone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotationOffsets, meta = (DisplayName = "Head Rotation Offset", PinHiddenByDefault))
		FRotator Extra_Rotation_Offset = FRotator(0, 0, 0);








	FTransform Initial_Effector_Reference = FTransform::Identity;

	FTransform Initial_Skel_Reference = FTransform::Identity;



	/*
	* This solver uses two distinct solving types - Strict and Unstrict
	* Strict solving makes the entire bone chain to follow the location,rotation and scale of the effector transform in a strict manner. Like an efficient train. Good for creaturs like chinese dragons and cetipees.
	* Un-Strict solving makes the entire bone chain to somewhat follow the location of the effector, but in a lazier manner. The rotations are calculated based on the location difference. Scale remains unchanged. Good for most quarupeds for a natural effect.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Use Advanced Strict Chinese Dragon Logic ?", PinHiddenByDefault))
		bool Use_Advanced_Strict_Chain_Logic = false;


	int Last_Detected_Bone = -1;


	/*
	 * Remove any vertical movement of the body.
	 * Useful for simple quadruped turning/curving purposes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Limit Z Solving ?",PinHiddenByDefault))
		bool Limit_Z_Solving = true;



	


	/*
	 *	Only usable in simple logic mode.
	 * The root bone can be fixed in place while letting the other bone chain to move around.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Set root bone in place and limit chain extension (only for unstrict logic) ?",PinHiddenByDefault,EditCondition = "!Use_Advanced_Strict_Chain_Logic"))
		bool Fix_Root_in_Place = false;
	


	/*
	* Enable this if you need the creature to always try to go back to it's original pose at all times.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Use Continous Pose Normalization (only for unstrict logic) ?", PinHiddenByDefault, EditCondition = "!Use_Advanced_Strict_Chain_Logic"))
		bool Continous_Normalization = false;



	/*
	* How fast should this creature normalize it's pose from the solving state.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Normalization Speed (only for unstrict logic) ?", PinHiddenByDefault, EditCondition = "!Use_Advanced_Strict_Chain_Logic && Continous_Normalization"))
		float Normalization_Speed = 5;


	/*
	* Use this curve to control the normalization speed with respect to the character's velocity derived from the pawn/character's velocity from it's movement component
	*/
	UPROPERTY(EditAnywhere, Category = SolverTypeAndMainSettings, meta = (DisplayName = "Normalization Speed Multiplier Relative To Velocity", EditCondition = "!Use_Advanced_Strict_Chain_Logic && Continous_Normalization", PinHiddenByDefault))
		FRuntimeFloatCurve Normalization_Multiplier_Rel_Velocity;



	/*
	 * If turned on, the original animation is preserved while the character bends,curves and follows the effector transform.
	 * If turned off, the character bends,curves and follows the effector transform based on the rigged orientations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PosePreservationSettings, meta = (DisplayName = "Should Solving Preserve Original Pose ?", PinHiddenByDefault))
		bool Preserve_Original_Pose = true;





	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PosePreservationSettings, meta = (DisplayName = "Forward Axis", PinHiddenByDefault))
		FVector Test_Ref_Forward_Axis = FVector(0,1,0);



	/*
	* This is only relevant if preserve original pose mode is used.
	* Calibrate this value until the yellow line widget aligns close to the 'Core Pelvis' bone you set previously.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PosePreservationSettings, meta = (DisplayName = "Core Pelvis Positioning Calibration Value", PinHiddenByDefault))
		float Pelvis_Positioning_Calibration_Value = 0;






	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PosePreservationSettings, meta = (DisplayName = "Final Result Offset in Component Space", PinHiddenByDefault))
	//	FVector Complete_Bone_Offset = FVector(0,0,0);


	


	float Initial_Warmup_Distance_Alpha = 0;

	float accumulated_warmup_distance = 0;
	FVector Saved_Current_Owning_Skel_Trans = FVector(0,0,0);



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Preview in Animbp Viewport for debugging ? (If using unpreserved pose mode)", PinHiddenByDefault))
		bool Enable_AnimBP_Viewport_Output = true;



	/*
	 * Whether the effector transform needed to be calibrated at the beginning or use absolute transform values ?  
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Set scale be relative to initial effector transform ?",PinHiddenByDefault))
		bool Use_Scale_Add_Mode = true;



	/*
	 * Whether the effector transform needed to be calibrated at the beginning or use absolute transform values ?  
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Set rotation be relative to initial effector transform ?",PinHiddenByDefault))
		bool Use_Rot_Add_Mode = false;


	
	/*
	 * Whether to make the rotations be adjusted to the current transform of the owning mesh ?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Set rotation be relative to owning mesh transform ?",PinHiddenByDefault))
		bool rotation_relative_to_mesh = true;



	/*
	 * The solver initially appears turned off and only blends into the solving state when the character moves and crosses a distance treshold.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Use Initial Warmup Logic ?", PinHiddenByDefault))
		bool Use_Warmup_Logic = true;

	/*
	 * The solver initially appears turned off in the beginning. We blend into the solving state while crossing this distance treshold.
	 * The warming up logic resets if the 'Reset To Normal?' parameter is turned on and off.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MinorSettings, meta = (DisplayName = "Initial Warmup Distance", PinHiddenByDefault, EditCondition = "Use_Warmup_Logic"))
		float Initial_Warmup_Distance = 700;


	
	/*
	 * Reset the solver completely, make all bones go back to their original animated positions,rotations and scale in a smooth manner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "Reset To Normal ?", PinShownByDefault))
		bool Reset_To_Normal = false;

	float reset_to_normal_alpha = 0;

	bool Reset_Commiting = false;

	int Alpha_Reset_Rest_Count = 0;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "Use General Interpolation ?", PinHiddenByDefault))
		bool Use_Interpolation = true;

	/*
	 * Whether interpolation is to be in absolute world space or in component space
	 */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "Interpolation Is In World Space ?",PinHiddenByDefault, EditCondition = "Use_Interpolation"))
		bool Interpolation_Is_World = true;


	/*
	* General solving interpolation speed.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "General Interpolation Speed",PinHiddenByDefault, EditCondition = "Use_Interpolation"))
		float Interpolation_Speed = 25;

	


	
	/*
	 * The blending speed from going from solving state to original state during a reset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "RESET ON Blending Speed", PinHiddenByDefault))
	float Reset_ON_Interpolation_Speed = 15;


	
	/*
	 * The blending speed from going from going from original state to solving state after a reset is turned off again.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationSettings, meta = (DisplayName = "RESET OFF Blending Speed", PinHiddenByDefault))
		float Reset_OFF_Interpolation_Speed = 5;


	

	//UPROPERTY(EditAnywhere, Category = TracePerformance, meta = (DisplayName = "Initial Interpolation Warmup Curve", PinHiddenByDefault))
	//	FRuntimeFloatCurve Warmup_Curve;



	/** The gap of each track point when using the advanced logic mode
	 *  Use larger values if each track point needed to be bigger, but 1 is good enough for most purposes.
	 */
	UPROPERTY(EditAnywhere, Category = MinorSettings)
		float Precision = 1;

	float Maximum_Bone_Extension = 100000000;


	

	
	/** Reference frame of Effector Transform. */
	//UPROPERTY(EditAnywhere, Category = EndEffector)
	//TEnumAsByte<enum EBoneControlSpace> EffectorTransformSpace;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debugging, meta = (DisplayName = "Debugging Effector (only for animbp viewport purpose)",PinHiddenByDefault))
		FTransform CachedEffectorCSTransform = FTransform::Identity;


	TArray<int> IgnoredSolvedBones = TArray<int>();


	FBoneContainer* SavedBoneContainer;


	UPROPERTY()
	TObjectPtr < USkeletalMeshComponent> owning_skel = nullptr;

	FTransform owner_skel_w_transform = FTransform::Identity;

	TArray<FChinaDragonChainLink> Chain;

	TArray<FChinaDragonChainLink> Test_Chain;

	//bool ChineseDragonChainSolving(TArray<FChinaDragonChainLink>& InOutChain, const FVector& TargetPosition, double MaximumReach);
	bool ChineseDragonChainSolving_Strict(FComponentSpacePoseContext& Output,TArray<FChinaDragonChainLink>& InOutChain, const FTransform& TargetPosition, TArray<FTransform>& RecordedPath);

	bool ChineseDragonChainSolving_Unstrict(TArray<FChinaDragonChainLink>& InOutChain, const FTransform& TargetPosition, bool Fix_Root);


	TArray<FTransform> SolverProcessing(FComponentSpacePoseContext& Output,TArray<FChinaDragonChainLink>& ChainTemp, FTransform InputTransform, bool Fix_Root, bool IsTestChain);

	void Solver_PreProcessing(TArray<FChinaDragonChainLink>& ChainTemp,TArray<FBoneTransform>& OutTransforms,TArray<FCompactPoseBoneIndex> BoneIndices,FComponentSpacePoseContext& Output);


	
	void SmoothTransformArray(TArray<FTransform>& InOutTransforms, float Alpha = 0.5f);

	
	TArray<FTransform> path_record;


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

	FAnimNode_ChineseDragonSolver();

protected:

//	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


//	void LineTraceControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


	// return true if it is valid to Evaluate
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones);
	


};


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


#include "AnimNode_TailSolver.generated.h"

/**
 * 
 */

 

class FPrimitiveDrawInterface;
//class USkeletalMeshComponent;





USTRUCT()
struct FDragonTailChainLink
{
	GENERATED_USTRUCT_BODY()

public:
	/** Position of bone in component space. */
	FTransform Position;

	FTransform OriginalPosition;

	FTransform InterpolatedPosition;

	FHitResult chain_trace_hit;

	FVector Last_Trace_Hit_Position = FVector(0,0,0);

	float HitAlpha = 0;

	float Bone_CS_Height = 0;

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

	FDragonTailChainLink()
		: Position(FTransform::Identity)
		, Length(0.0)
		, BoneIndex(INDEX_NONE)
		, TransformIndex(INDEX_NONE)
		, DefaultDirToParent(FVector(-1.0, 0.0, 0.0))
	{
	}

	FDragonTailChainLink(const FTransform& InPosition, const double InLength, const FCompactPoseBoneIndex& InBoneIndex, const int32& InTransformIndex)
		: Position(InPosition)
		, Length(InLength)
		, BoneIndex(InBoneIndex.GetInt())
		, TransformIndex(InTransformIndex)
		, DefaultDirToParent(FVector(-1.0, 0.0, 0.0))
	{
	}

	FDragonTailChainLink(const FTransform& InPosition, const double InLength, const FCompactPoseBoneIndex& InBoneIndex, const int32& InTransformIndex, const FVector& InDefaultDirToParent)
		: Position(InPosition)
		, Length(InLength)
		, BoneIndex(InBoneIndex.GetInt())
		, TransformIndex(InTransformIndex)
		, DefaultDirToParent(InDefaultDirToParent)
	{
	}

	FDragonTailChainLink(const FTransform& InPosition, const double InLength, const int32 InBoneIndex, const int32 InTransformIndex)
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
struct DRAGONIKPLUGIN_API FAnimNode_TailSolver : public FAnimNode_DragonControlBase
{
	//	GENERATED_USTRUCT_BODY()
	GENERATED_BODY()



public:





	virtual int32 GetLODThreshold() const override { return LODThreshold; }


	float component_scale = 1;

	int solver_counter = 0;




	/*
	* - Add the tail bone names in the custom bone input. The first element should be the tail root and the last element should be the tail tip.
	* - Set an appropriate bone height.
	* - Tweak the trace height upwards and trace height downwards value according to your needs. The trace height downwards can be kept 0 in most cases as the tail only
	* needs to react if it's going through any object, but be left normal otherwise.
	* Tweak the interpolation and transition speeds to your needs.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Hover here for usage tooltips", PinHiddenByDefault ))
		FName HowToUse = "How to use this solver ?";
	

	
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Core, meta = (DisplayName = "Use Custom Bone Chain ?",PinHiddenByDefault))
	bool Use_Custom_Bone_Chain = true;

	
	/** Name of tip bone */
//	UPROPERTY(EditAnywhere, Category = Core, meta = (DisplayName = "Tip Bone (Eg:- Head)",PinHiddenByDefault,EditCondition = "!Use_Custom_Bone_Chain"))
	FBoneReference TipBone;

	/** Name of the root bone*/
//	UPROPERTY(EditAnywhere, Category = Core, meta = (DisplayName = "Root Bone (Eg:- Pelvis)",PinHiddenByDefault,EditCondition = "!Use_Custom_Bone_Chain"))
	FBoneReference RootBone;


	/*
	 * The custom bone chain input.
	 * The 0th element is the tail "root" bone and the last element is the tail "tip" bone.
	 * Bone order only matters in a visual sense.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Custom Bone Chain Input",PinHiddenByDefault,EditCondition = "Use_Custom_Bone_Chain"))
		FDragonData_TailBoneInput Custom_Bone_Input;


	// Gather all bone indices between root and tip.
	//	TArray<FCompactPoseBoneIndex> BoneIndices;


	bool ShouldSolverFail = false;


	TArray<FBoneReference> Custom_BoneList_Reference = TArray<FBoneReference>();




	/*
	* The bone height is the floating gap between the tail bone and the trace hit impact location.
	* If custom heights is off, all the tail bones can use the fixed Tail Bone Height.
	* If custom heights is enabled, you can set individual heights to each tail bone using the custom bone heights.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Use Custom Bone Height Array ?", PinHiddenByDefault))
		bool use_custom_heights = false;




	/*
	* The bone height is the floating gap between the tail bone and the trace hit impact location.
	* If custom heights is off, all the tail bones can use the fixed Tail Bone Height.
	* If custom heights is enabled, you can set individual heights to each tail bone using the custom bone heights.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Tail Bone Height", PinHiddenByDefault, EditCondition = "!use_custom_heights"))
		float Tail_Bone_Height = 100;





	/*
	* The bone height is the floating gap between the tail bone and the trace hit impact location.
	* If custom heights is off, all the tail bones can use the fixed Tail Bone Height.
	* If custom heights is enabled, you can set individual heights to each tail bone using the custom bone heights.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Custom Bone Heights", PinHiddenByDefault, EditCondition = "use_custom_heights"))
		FDragonData_TailHeightInput Custom_Bone_Heights;


	



	/*
	*The up direction vector of the character in component space.
	* 99% cases, this should not be altered.
	* Only needed to alter on characters that do not follow the standard unreal character orientations.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "Up Direction Vector", PinHiddenByDefault))
		FVector character_direction_vector_CS = FVector(0, 0, 1);

	TArray<FDragonTailChainLink> Chain;







	/*
	* The general bone interpolation speed.
	* Smaller values gives slowe smoother results, larger values gives faster but potentially rough results.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CoreInput, meta = (DisplayName = "General Animation Interpolation Speed", PinHiddenByDefault))
		float Animation_Interpolation_Speed = 25;







	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings)
		TEnumAsByte<ETraceTypeQuery> Trace_Channel = ETraceTypeQuery::TraceTypeQuery1;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Trace Hit Complex ? (True for complex, False for simple)"))
		bool line_trace_hit_complex = true;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Use Sphere Trace ?"))
		bool Use_Sphere_Trace = false;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Sphere Trace Radius", EditCondition = "Use_Sphere_Trace"))
		float Sphere_Trace_Radius = 25;




	FTransform Initial_Effector_Reference = FTransform::Identity;

	FTransform Initial_Skel_Reference = FTransform::Identity;



	/*
	* The upward height of the trace from each tail bone location.
	* 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Trace Up Height", PinHiddenByDefault))

			float Trace_Up_Height = 1000;



	/*
	* The downward height of the trace from each tail bone location.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Trace Down Height", PinHiddenByDefault))
			float Trace_Down_Height = 0;



		


		


	/*
	* When the trace of a tail bone hits something in the level, the tail bone shifts and adjusts it's transform from it's original bone transform to the hit location adjusted transform.
	* This parameter is the interpolation speed going from OFF to ON state.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Trace Block Transistion Speed", PinHiddenByDefault))
			float TraceBlock_Interpolation_Speed = 5;



	/*
	* When the trace of a tail bone stops hitting anything in the level, the tail bone goes towards it's original transform.
	* This parameter is the interpolation speed going from ON to OFF state.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicTraceSettings, meta = (DisplayName = "Trace Unblock Transition Speed", PinHiddenByDefault))
			float TraceUnBlock_Interpolation_Speed = 5;


	//




	float trace_timer_count = 1000;

	bool trace_distance_legal = false;


	float current_trace_interval = 0.1f;

	TArray<FColor> TraceLinearColor = TArray<FColor>();

	TArray<FVector> TraceStartList = TArray<FVector>();
	TArray<FVector> TraceEndList = TArray<FVector>();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "Trace Firing Interval Constant (Performance)", EditCondition = "!Use_LOD_Specific_Intervals", PinHiddenByDefault))
		float trace_interval_duration = 0.1f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "Use LOD specific intervals ?", PinHiddenByDefault))
		bool Use_LOD_Specific_Intervals = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "LOD0 Interval", EditCondition = "Use_LOD_Specific_Intervals", PinHiddenByDefault))
		float LOD0_Interval = 0.1f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "LOD1 Interval", EditCondition = "Use_LOD_Specific_Intervals", PinHiddenByDefault))
		float LOD1_Interval = 0.2f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "LOD2 Interval", EditCondition = "Use_LOD_Specific_Intervals", PinHiddenByDefault))
		float LOD2_Interval = 0.5f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "Turn off traces after a distance from the camera ?", PinHiddenByDefault))
		bool Use_Trace_Distance_Adapting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "Max distance until traces are turned off", EditCondition = "Use_Trace_Distance_Adapting", PinHiddenByDefault))
		float Trace_Max_Distance = 5000;

			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TracePerformance, meta = (DisplayName = "Use trace interval-velocity multiplier curve function ?", PinHiddenByDefault))
			//	bool Use_Interval_Velocity_Curve = true;




	//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debugging, meta = (DisplayName = "Debugging Effector (only for animbp viewport purpose)",PinHiddenByDefault))
		FTransform CachedEffectorCSTransform = FTransform::Identity;


	


	FBoneContainer* SavedBoneContainer;


	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> owning_skel = nullptr;

	FTransform owner_skel_w_transform = FTransform::Identity;



	void line_trace_func(USkeletalMeshComponent& skelmesh, FVector startpoint, FVector endpoint, FHitResult RV_Ragdoll_Hit, FName bone_text, FName trace_tag, FHitResult& Output, FLinearColor debug_color, bool debug_mode = false, bool ducking_mode = false);

	void Solver_PreProcessing(TArray<FDragonTailChainLink>& ChainTemp, TArray<FBoneTransform>& OutTransforms, TArray<FCompactPoseBoneIndex> BoneIndices, FComponentSpacePoseContext& Output);


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

	FAnimNode_TailSolver();

protected:

//	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


//	void LineTraceControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);


	// return true if it is valid to Evaluate
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones);
	


};


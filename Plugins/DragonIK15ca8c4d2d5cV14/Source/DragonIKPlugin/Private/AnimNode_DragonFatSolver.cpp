/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/


#include "AnimNode_DragonFatSolver.h"
#include "Animation/AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"






// Initialize the component pose as well as defining the owning skeleton
void FAnimNode_DragonFatSolver::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_DragonControlBase::Initialize_AnyThread(Context);
	//	ComponentPose.Initialize(Context);
	owning_skel = Context.AnimInstanceProxy->GetSkelMeshComponent();




	//	dragon_bone_data.Start_Spine = FBoneReference(dragon_input_data.Start_Spine);
}




void FAnimNode_DragonFatSolver::Evaluate_AnyThread(FPoseContext& Output)
{
}





void FAnimNode_DragonFatSolver::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* PreviewSkelMeshComp) const
{



}

//Perform update operations inside this
void FAnimNode_DragonFatSolver::UpdateInternal(const FAnimationUpdateContext& Context)
{
	FAnimNode_DragonControlBase::UpdateInternal(Context);
	
	
}







//Nothing would be needed here
void FAnimNode_DragonFatSolver::EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context)
{
}






void FAnimNode_DragonFatSolver::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{


	if (Character_Bone_References.IsEmpty())
	{

		

		//Initialize all character bone references once
		for (int bone_i = 0; bone_i < owning_skel->GetNumBones(); bone_i++)
		{

			FBoneReference Bone_Ref = FBoneReference(owning_skel->GetBoneName(bone_i));
			Bone_Ref.Initialize(*SavedBoneContainer);
			Character_Bone_References.Add(Bone_Ref);

		}
	}


	TArray<FName> Input_Bone_Name_Array = TArray<FName>();

	for (int bone_struct_i=0;bone_struct_i < fat_bone_input.Custom_Bone_Structure.Num(); bone_struct_i++)
	{
		Input_Bone_Name_Array.Add(fat_bone_input.Custom_Bone_Structure[bone_struct_i].Bone_Name);
	}


	for (int bone_i = 0; bone_i < Character_Bone_References.Num(); bone_i++)
	{

		


		const FCompactPoseBoneIndex BoneCompactIndex = FCompactPoseBoneIndex(Character_Bone_References[bone_i].BoneIndex);
		
		if (BoneCompactIndex.IsValid())
		{

			if (Input_Bone_Name_Array.Contains(Character_Bone_References[bone_i].BoneName))
			{

					
					float Bone_Strength_Proportion = 1;

					int Fat_Bone_Index = Input_Bone_Name_Array.IndexOfByKey(Character_Bone_References[bone_i].BoneName);

					if (fat_control_input.FatControlStrengthArray.Num() > Fat_Bone_Index)
					{

						Bone_Strength_Proportion = fat_control_input.FatControlStrengthArray[Fat_Bone_Index];
					}
				
					FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(BoneCompactIndex);

					

			//		FTransform BoneTransform = BoneCSTransform;
					FTransform BoneTransform = BoneCSTransform * owning_skel->GetComponentTransform();


					// Construct a scaling matrix along the world direction
					float k = Bone_Strength_Proportion * Fat_Strength_Value;

					float clamped_fat_alpha = FMath::Clamp<float>(Fat_Alpha_Value,0,1);
					
					k = FMath::Lerp(1,k, clamped_fat_alpha);

					if(fat_bone_input.Custom_Bone_Structure[Fat_Bone_Index].Bone_Axis == EInput_FatBoneFAxis::ENUM_XAxis)
						BoneTransform.SetScale3D(FVector(1,k,k));


					if (fat_bone_input.Custom_Bone_Structure[Fat_Bone_Index].Bone_Axis == EInput_FatBoneFAxis::ENUM_YAxis)
						BoneTransform.SetScale3D(FVector(k, 1, k));


					if (fat_bone_input.Custom_Bone_Structure[Fat_Bone_Index].Bone_Axis == EInput_FatBoneFAxis::ENUM_ZAxis)
						BoneTransform.SetScale3D(FVector(k, k, 1));



					if(Use_Axis_Limitation == false || fat_bone_input.Custom_Bone_Structure[Fat_Bone_Index].Bone_Axis == EInput_FatBoneFAxis::ENUM_XYZAxis)
						BoneTransform.SetScale3D(FVector(k, k, k));


					BoneTransform.SetLocation(BoneCSTransform.GetLocation());
					BoneTransform.SetRotation(BoneCSTransform.GetRotation());
					
					
					FBoneTransform BoneFinalTransform = FBoneTransform(BoneCompactIndex, BoneTransform);
					OutBoneTransforms.Add(BoneFinalTransform);
				
			}
			else
			{

				FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(BoneCompactIndex);

				BoneCSTransform.SetScale3D(FVector(1,1,1));
				FBoneTransform BoneFinalTransform = FBoneTransform(BoneCompactIndex, BoneCSTransform);
				OutBoneTransforms.Add(BoneFinalTransform);
			}
		}



	}
	


}






bool FAnimNode_DragonFatSolver::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{


	for (int bone_struct_i = 0; bone_struct_i < fat_bone_input.Custom_Bone_Structure.Num(); bone_struct_i++)
	{
		//Input_Bone_Name_Array.Add(fat_bone_input.Custom_Bone_Structure[bone_struct_i].Bone_Name);
		if (fat_bone_input.Custom_Bone_Structure[bone_struct_i].Bone_Ref.IsValidToEvaluate(RequiredBones) == false)
		{
			return false;
		}

	}



	return true;
	
}




FAnimNode_DragonFatSolver::FAnimNode_DragonFatSolver()
{	
		

}




//#if WITH_EDITOR
void FAnimNode_DragonFatSolver::ResizeDebugLocations(int32 NewSize)
{
	
}
//#endif 


void FAnimNode_DragonFatSolver::InitializeBoneReferences(FBoneContainer& RequiredBones)
{


	SavedBoneContainer = &RequiredBones;



	for (int bone_struct_i = 0; bone_struct_i < fat_bone_input.Custom_Bone_Structure.Num(); bone_struct_i++)
	{

		fat_bone_input.Custom_Bone_Structure[bone_struct_i].Bone_Ref = FBoneReference(fat_bone_input.Custom_Bone_Structure[bone_struct_i].Bone_Name);

		fat_bone_input.Custom_Bone_Structure[ bone_struct_i ].Bone_Ref.Initialize(RequiredBones);

	}

	
}



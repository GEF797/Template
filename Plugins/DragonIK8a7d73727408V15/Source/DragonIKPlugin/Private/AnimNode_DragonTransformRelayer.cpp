/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/


#include "AnimNode_DragonTransformRelayer.h"
#include "Animation/AnimInstanceProxy.h"



#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"
#include "DragonIKTransformReceiverComp.h"



DECLARE_CYCLE_STAT(TEXT("DragonTransformRelayer Eval"), STAT_DragonTransformRelayer_Eval, STATGROUP_Anim);




FAnimNode_DragonTransformRelayer::FAnimNode_DragonTransformRelayer()
{
	

}




void FAnimNode_DragonTransformRelayer::InitializeBoneReferences(FBoneContainer& RequiredBones)
{
	
		character_bone_references.Empty();

		const UWorld* TheWorld = (owning_skel->GetOwner()!=nullptr ? owning_skel->GetOwner()->GetWorld() : nullptr);

		//if (owning_skel->GetOwner()->GetWorld()->IsGameWorld())
		//&& owning_skel->GetOwner()->GetWorld()->IsGameWorld()
		if (TheWorld!=nullptr)
		{
					
			character_bone_transforms.Empty();


			for (int physicshandle_index = 0; physicshandle_index < owning_skel->GetNumBones(); physicshandle_index++)
			{					

				FBoneReference bone_reference = FBoneReference(owning_skel->GetBoneName(physicshandle_index));
				bone_reference.Initialize(RequiredBones);
				character_bone_references.Add(bone_reference);
			}

			character_bone_transforms.AddDefaulted(character_bone_references.Num());

		}


}




// Initialize the component pose as well as defining the owning skeleton
void FAnimNode_DragonTransformRelayer::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{

	FAnimNode_DragonControlBase::Initialize_AnyThread(Context);
	owning_skel = Context.AnimInstanceProxy->GetSkelMeshComponent();

}




void FAnimNode_DragonTransformRelayer::EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output)
{
	Super::EvaluateComponentSpaceInternal(Output);

	if (owning_skel != nullptr)
	{
		

			
			//if (transform_receiver_component != nullptr && transform_receiver_component != NULL)
			if (transform_receiver_component != nullptr)
			{
				if (character_bone_references.Num() > 0 && character_bone_transforms.Num() > 0)
				{

					for (int transform_i = 0; transform_i < character_bone_references.Num(); transform_i++)
					{
						
						if (character_bone_references[transform_i].IsValidToEvaluate())
						{
							character_bone_transforms[transform_i] = Output.Pose.GetComponentSpaceTransform(character_bone_references[transform_i].GetCompactPoseIndex(Output.Pose.GetPose().GetBoneContainer()));


							if (!character_bone_transforms[transform_i].Equals(FTransform::Identity))
								FAnimationRuntime::ConvertCSTransformToBoneSpace(owning_skel->GetComponentTransform(), Output.Pose, character_bone_transforms[transform_i], character_bone_references[transform_i].CachedCompactPoseIndex, EBoneControlSpace::BCS_WorldSpace);


							World_Name_Transform_Map.Add(character_bone_references[transform_i].BoneName,character_bone_transforms[transform_i]*owning_skel->GetComponentTransform());

							Component_Name_Transform_Map.Add(character_bone_references[transform_i].BoneName,character_bone_transforms[transform_i]);

							
							//if (dragon_phys_component->Raw_Animation_Transforms != NULL)
							{
								transform_receiver_component->World_Name_Transform_Map = World_Name_Transform_Map;
								
								transform_receiver_component->Component_Name_Transform_Map = Component_Name_Transform_Map;
							}

						}
					}
				}
			}
	}

	
}


//Perform update operations inside this
void FAnimNode_DragonTransformRelayer::UpdateInternal(const FAnimationUpdateContext& Context)
{

	FAnimNode_DragonControlBase::UpdateInternal(Context);



}



void FAnimNode_DragonTransformRelayer::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{

	//ComponentPose.EvaluateComponentSpace(Output);


}


void FAnimNode_DragonTransformRelayer::Evaluate_AnyThread(FPoseContext& Output)
{

	

}




bool FAnimNode_DragonTransformRelayer::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{

	return true;
}


/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/


#include "AnimNode_TailSolver.h"
#include "Animation/AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"






// Initialize the component pose as well as defining the owning skeleton
void FAnimNode_TailSolver::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_DragonControlBase::Initialize_AnyThread(Context);
	//	ComponentPose.Initialize(Context);
	owning_skel = Context.AnimInstanceProxy->GetSkelMeshComponent();




	//	dragon_bone_data.Start_Spine = FBoneReference(dragon_input_data.Start_Spine);
}




void FAnimNode_TailSolver::Evaluate_AnyThread(FPoseContext& Output)
{
}





void FAnimNode_TailSolver::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* PreviewSkelMeshComp) const
{



}

//Perform update operations inside this
void FAnimNode_TailSolver::UpdateInternal(const FAnimationUpdateContext& Context)
{
	FAnimNode_DragonControlBase::UpdateInternal(Context);


	if (Context.AnimInstanceProxy->GetSkelMeshComponent() != nullptr)
	{
		component_scale = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentScale().Z;

		owner_skel_w_transform = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentTransform();
	}





	current_trace_interval = trace_interval_duration;

	if (Use_LOD_Specific_Intervals)
	{
		int lod_current_index = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetForcedLOD();

		if (lod_current_index == 0)
		{
			current_trace_interval = LOD0_Interval;
		}
		else
			if (lod_current_index == 1)
			{
				current_trace_interval = LOD1_Interval;
			}
			else
			{
				current_trace_interval = LOD2_Interval;
			}



	}




	if (Use_Trace_Distance_Adapting)
	{

		if (Context.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld()->GetFirstPlayerController())
		{
			FVector Camera_Location = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
			float Distance_Bw_Cam_Char = (Context.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentLocation() - Camera_Location).Size();


			trace_distance_legal = Distance_Bw_Cam_Char < Trace_Max_Distance;
		}
		else
		{
			trace_distance_legal = true;
		}

	}
	else
	{
		trace_distance_legal = true;
	}


	trace_timer_count += Context.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld()->DeltaTimeSeconds;





	for (int32 i = 1; i < Chain.Num(); i++)
	{
		FDragonTailChainLink& CurrentLink = Chain[i];
		//FDragonTailChainLink& PreviousLink = Chain[i - 1];

		FHitResult hit_result_info;


		float selected_trace_down_height = Trace_Down_Height;

		/*
		if ( i < Custom_Bone_Heights.Tail_Chain_Heights.Num())
		{

			selected_trace_down_height = Custom_Bone_Heights.Tail_Chain_Heights[i];

		}*/


		FVector Character_UP_WS = UKismetMathLibrary::TransformDirection(owner_skel_w_transform, character_direction_vector_CS);


		line_trace_func(*Context.AnimInstanceProxy->GetSkelMeshComponent()
			, CurrentLink.OriginalPosition.GetLocation() + Character_UP_WS * Trace_Up_Height* component_scale,
			CurrentLink.OriginalPosition.GetLocation() - Character_UP_WS * selected_trace_down_height* component_scale,
			hit_result_info,
			"",
			"",
			hit_result_info, FLinearColor::Red,
			true);


		if (trace_timer_count > current_trace_interval && trace_distance_legal)
		{
			CurrentLink.chain_trace_hit = hit_result_info;
		}
		
		
		//CurrentLink.Position.SetLocation( hit_result_info.ImpactPoint);

	}


	if (solver_counter < 20)
		solver_counter++;


	if (trace_timer_count > current_trace_interval && trace_distance_legal)
		trace_timer_count = 0;
}








void FAnimNode_TailSolver::line_trace_func(USkeletalMeshComponent& skelmesh, FVector startpoint, FVector endpoint, FHitResult RV_Ragdoll_Hit, FName bone_text, FName trace_tag, FHitResult& Output, FLinearColor debug_color, bool debug_mode, bool ducking_mode)
{


	TArray<AActor*> ignoreActors;



	//if (RV_Ragdoll_Hit.ImpactNormal.Equals(FVector::ZeroVector))
	if (owning_skel->GetOwner() && (trace_timer_count > current_trace_interval && trace_distance_legal))
	{
		ignoreActors.Add(owning_skel->GetOwner());


		//UKismetSystemLibrary::LineTraceSingle(owning_skel->GetOwner(), startpoint, endpoint, Trace_Channel, true, ignoreActors, EDrawDebugTrace::None, RV_Ragdoll_Hit, false, debug_color, FLinearColor::Yellow);

		float owner_scale = component_scale;


		{

			
			if (Use_Sphere_Trace)
			{
				UKismetSystemLibrary::SphereTraceSingle(owning_skel->GetOwner(), startpoint, endpoint, Sphere_Trace_Radius* owner_scale, Trace_Channel, line_trace_hit_complex, ignoreActors, EDrawDebugTrace::None, RV_Ragdoll_Hit, true, debug_color);
			}
			else
			{

				UKismetSystemLibrary::LineTraceSingle(owning_skel->GetOwner(), startpoint, endpoint, Trace_Channel, line_trace_hit_complex, ignoreActors, EDrawDebugTrace::None, RV_Ragdoll_Hit, true, debug_color);
			}
		}



	}



	TraceStartList.Add(startpoint);
	TraceEndList.Add(endpoint);
	TraceLinearColor.Add(debug_color.ToFColor(true));






	Output = RV_Ragdoll_Hit;

}




//Nothing would be needed here
void FAnimNode_TailSolver::EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context)
{
}



void FAnimNode_TailSolver::Solver_PreProcessing(TArray<FDragonTailChainLink>& ChainTemp, TArray<FBoneTransform>& OutTransforms, TArray<FCompactPoseBoneIndex> BoneIndices, FComponentSpacePoseContext& Output)
{

	int32 const NumTransforms = BoneIndices.Num();

	if (ChainTemp.Num() == 0)
	{

		ChainTemp.AddDefaulted(NumTransforms);

	}

	if (ChainTemp.Num() > 0)
	{

		// Start with Root Bone
		{
			const FCompactPoseBoneIndex& RootBoneIndex = BoneIndices[0];
			FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(RootBoneIndex);



			BoneCSTransform = BoneCSTransform * owning_skel->GetComponentTransform();

			//BoneCSTransform.SetRotation(FQuat::Identity);
			//BoneCSTransform.SetScale3D(FVector(1, 1, 1));



			OutTransforms[0] = FBoneTransform(RootBoneIndex, BoneCSTransform);
			//	Chain.Add(FChinaDragonChainLink(BoneCSTransform.GetLocation(), 0.f, RootBoneIndex, 0));

			FHitResult temp_hit = ChainTemp[0].chain_trace_hit;
			FTransform temp_interp_trans = ChainTemp[0].InterpolatedPosition;
			float temp_interp_alpha = ChainTemp[0].HitAlpha;
			FVector temp_Last_Trace_Hit_Position = ChainTemp[0].Last_Trace_Hit_Position;



			ChainTemp[0] = FDragonTailChainLink(BoneCSTransform, 0.f, RootBoneIndex, 0);

			ChainTemp[0].OriginalPosition = BoneCSTransform;


			ChainTemp[0].chain_trace_hit = temp_hit;
			ChainTemp[0].InterpolatedPosition = temp_interp_trans;
			ChainTemp[0].HitAlpha = temp_interp_alpha;
			ChainTemp[0].Last_Trace_Hit_Position = temp_Last_Trace_Hit_Position;
			ChainTemp[0].Bone_CS_Height = Output.Pose.GetComponentSpaceTransform(BoneIndices[0]).GetLocation().Z;



			//ChainTemp[0].InterpolatedPosition = BoneCSTransform;
		}

		// Go through remaining transforms
		for (int32 TransformIndex = 1; TransformIndex < NumTransforms; TransformIndex++)
		{
			const FCompactPoseBoneIndex& BoneIndex = BoneIndices[TransformIndex];

			FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(BoneIndex);

			//BoneCSTransform = BoneCSTransform*(owning_skel->GetComponentTransform().Inverse()*owner_skel_w_transform);

			BoneCSTransform = BoneCSTransform * owning_skel->GetComponentTransform();

			//BoneCSTransform.SetRotation(FQuat::Identity);
			//BoneCSTransform.SetScale3D(FVector(1, 1, 1));


			FVector const BoneCSPosition = BoneCSTransform.GetLocation();

			OutTransforms[TransformIndex] = FBoneTransform(BoneIndex, BoneCSTransform);

			// Calculate the combined length of this segment of skeleton
			double const BoneLength = FVector::Dist(BoneCSPosition, OutTransforms[TransformIndex - 1].Transform.GetLocation());

			//	if (!FMath::IsNearlyZero(BoneLength))
			{

				//	Chain.Add(FChinaDragonChainLink(BoneCSPosition, BoneLength, BoneIndex, TransformIndex));

				FHitResult temp_hit = ChainTemp[TransformIndex].chain_trace_hit;
				FTransform temp_interp_trans = ChainTemp[TransformIndex].InterpolatedPosition;
				float temp_interp_alpha = ChainTemp[TransformIndex].HitAlpha;
				FVector temp_Last_Trace_Hit_Position = ChainTemp[TransformIndex].Last_Trace_Hit_Position;



				ChainTemp[TransformIndex] = FDragonTailChainLink(BoneCSTransform, 0, BoneIndex, TransformIndex);

				ChainTemp[TransformIndex].OriginalPosition = BoneCSTransform;

				ChainTemp[TransformIndex - 1].Length = BoneLength;

				ChainTemp[TransformIndex].chain_trace_hit = temp_hit;
				ChainTemp[TransformIndex].InterpolatedPosition = temp_interp_trans;
				ChainTemp[TransformIndex].HitAlpha = temp_interp_alpha;
				ChainTemp[TransformIndex].Last_Trace_Hit_Position = temp_Last_Trace_Hit_Position;

				//ChainTemp[TransformIndex].InterpolatedPosition = BoneCSTransform;



				ChainTemp[TransformIndex].Bone_CS_Height = Output.Pose.GetComponentSpaceTransform(BoneIndex).GetLocation().Z;

			}

		}


		

	}





}


void FAnimNode_TailSolver::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{



	if (owning_skel != nullptr)
	{

		



		


		if (Initial_Skel_Reference.Equals(FTransform::Identity))
		{
			Initial_Skel_Reference = owning_skel->GetComponentTransform();
		}


		



		const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

		
		ShouldSolverFail = false;


		// Gather all bone indices between root and tip.
		TArray<FCompactPoseBoneIndex> BoneIndices;

		//if(Use_Custom_Bone_Chain)
		{

			for (int i = 0; i < Custom_Bone_Input.Custom_Bone_Structure.Num(); i++)
			{

				FBoneReference Bone_Input = FBoneReference(Custom_Bone_Input.Custom_Bone_Structure[i]);
				Bone_Input.Initialize(BoneContainer);

				BoneIndices.Insert(Bone_Input.CachedCompactPoseIndex, i);


				Custom_BoneList_Reference.Insert(Bone_Input, i);


				if (Bone_Input.CachedCompactPoseIndex.GetInt() < 0)
					ShouldSolverFail = true;

				//	GEngine->AddOnScreenDebugMessage(-1, 0.05f, FColor::Red, " Bone_Input.CachedCompactPoseIndex : " + FString::SanitizeFloat(Bone_Input.CachedCompactPoseIndex.GetInt()));


			}

		}
		

		if (ShouldSolverFail == false)
		{

			int32 const NumTransforms = BoneIndices.Num();

			OutBoneTransforms.AddUninitialized(NumTransforms);

			Solver_PreProcessing(Chain, OutBoneTransforms, BoneIndices, Output);



			FVector Character_UP_WS = UKismetMathLibrary::TransformDirection(owner_skel_w_transform, character_direction_vector_CS);



			{

				FDragonTailChainLink& CurrentLink = Chain[0];
				CurrentLink.Position = (CurrentLink.OriginalPosition);

			}



			float tail_tip_offset = 0;
			bool any_trace_hit = false;

			float total_bonechain_length = 0;

			FVector highest_impact_point = FVector(0, 0, 0);

			int closest_hit_bone = -1;



			{



				for (int32 i = 0; i < Chain.Num(); i++)
				{


					float ProjectionA = FVector::DotProduct(Chain[i].chain_trace_hit.ImpactPoint, Character_UP_WS);
					float ProjectionB = FVector::DotProduct(highest_impact_point, Character_UP_WS);

					total_bonechain_length += Chain[i].Length;

					if (Chain[i].chain_trace_hit.bBlockingHit)
					{
						any_trace_hit = true;


						//	if (Chain[i].chain_trace_hit.ImpactPoint.Z > highest_impact_point.Z || highest_impact_point.Equals(FVector(0, 0, 0)))

						if (ProjectionA > ProjectionB || highest_impact_point.Equals(FVector(0, 0, 0)))
						{
							highest_impact_point = Chain[i].chain_trace_hit.ImpactPoint;

							//	closest_hit_bone = i;
						}

						//if(closest_hit_bone == -1)
						closest_hit_bone = i;
					}

				}


				FDragonTailChainLink& CurrentLink = Chain[Chain.Num() - 1];
				//CurrentLink.Position = EffectorTransform;

				FVector TipPostion = CurrentLink.Position.GetLocation();

				//if (CurrentLink.chain_trace_hit.bBlockingHit)
				if (any_trace_hit)
				{
					FVector TracePoint = CurrentLink.OriginalPosition.GetLocation();


					UDragonIK_Library::SetVectorComponentAlongDirection(TracePoint, highest_impact_point, Character_UP_WS);


					if ((Chain.Num() - 1) < Custom_Bone_Heights.Tail_Chain_Heights.Num() && use_custom_heights)
					{

						TracePoint += Character_UP_WS * Custom_Bone_Heights.Tail_Chain_Heights[Chain.Num() - 1];

					}
					else
					{
						TracePoint += Character_UP_WS * Tail_Bone_Height;
					}


					//TracePoint += character_direction_vector_CS * Chain[Chain.Num()-1].Bone_CS_Height;


					float Tip_Hit_Ratio = (float)(closest_hit_bone - 1) / (float)Chain.Num();


					if (closest_hit_bone > -1)
					{
						TracePoint = UKismetMathLibrary::VLerp(CurrentLink.OriginalPosition.GetLocation(), TracePoint, Tip_Hit_Ratio);
					}


					//	TracePoint = CurrentLink.OriginalPosition.GetLocation();


					TipPostion = TracePoint;

					CurrentLink.Last_Trace_Hit_Position = TipPostion;

					CurrentLink.HitAlpha = UKismetMathLibrary::FInterpTo(CurrentLink.HitAlpha, 1, owning_skel->GetWorld()->DeltaTimeSeconds, TraceBlock_Interpolation_Speed);
				}
				else
				{
					CurrentLink.HitAlpha = UKismetMathLibrary::FInterpTo(CurrentLink.HitAlpha, 0, owning_skel->GetWorld()->DeltaTimeSeconds, TraceUnBlock_Interpolation_Speed);
				}




				CurrentLink.Position.SetLocation(UKismetMathLibrary::VLerp(CurrentLink.OriginalPosition.GetLocation(), CurrentLink.Last_Trace_Hit_Position, CurrentLink.HitAlpha));


				/*
				FTransform Inverse_InterpTransform = CurrentLink.InterpolatedPosition * owning_skel->GetComponentTransform().Inverse();
				FTransform Inverse_OriginalTransform = CurrentLink.OriginalPosition * owning_skel->GetComponentTransform().Inverse();


				FVector Inv_InterpPos = Inverse_InterpTransform.GetLocation();

				Inv_InterpPos.X = Inverse_OriginalTransform.GetLocation().X;
				Inv_InterpPos.Y = Inverse_OriginalTransform.GetLocation().Y;

				Inverse_InterpTransform.SetLocation(Inv_InterpPos);

				Inverse_InterpTransform = Inverse_InterpTransform * owning_skel->GetComponentTransform();

				CurrentLink.InterpolatedPosition = Inverse_InterpTransform;
				*/
				//


				FTransform CS_InterpTransform = CurrentLink.InterpolatedPosition * owning_skel->GetComponentTransform().Inverse();
				FTransform CS_OriginalTransform = CurrentLink.OriginalPosition * owning_skel->GetComponentTransform().Inverse();


				//FVector Inv_InterpPos = Inverse_InterpTransform.GetLocation();

				//Inv_InterpPos.X = Inverse_OriginalTransform.GetLocation().X;
				//Inv_InterpPos.Y = Inverse_OriginalTransform.GetLocation().Y;

				float Original_New_Length = (CS_InterpTransform.GetLocation() - CS_OriginalTransform.GetLocation()).Size();


				FVector Inv_InterpPos = CS_OriginalTransform.GetLocation() + character_direction_vector_CS * (Original_New_Length);


				CS_InterpTransform.SetLocation(Inv_InterpPos);

				CS_InterpTransform = CS_InterpTransform * owning_skel->GetComponentTransform();

				CurrentLink.InterpolatedPosition = CS_InterpTransform;


				//	CurrentLink.InterpolatedPosition.SetLocation(Inv_InterpPos);

				CurrentLink.InterpolatedPosition.SetLocation(UKismetMathLibrary::VInterpTo(CurrentLink.InterpolatedPosition.GetLocation(), CurrentLink.Position.GetLocation(), owning_skel->GetWorld()->DeltaTimeSeconds, Animation_Interpolation_Speed));



				FVector Original_Tip_Root_Dir = Chain[0].OriginalPosition.GetLocation() - Chain[Chain.Num() - 1].OriginalPosition.GetLocation();

				CurrentLink.Position.SetLocation(Chain[0].OriginalPosition.GetLocation() + (CurrentLink.InterpolatedPosition.GetLocation() - Chain[0].OriginalPosition.GetLocation()).GetUnsafeNormal() * Original_Tip_Root_Dir.Size());




				FVector DirectionToPrevious_Original = (Chain[Chain.Num() - 2].OriginalPosition.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).GetSafeNormal();
				FVector DirectionToPrevious = (Chain[Chain.Num() - 2].Position.GetLocation() - CurrentLink.InterpolatedPosition.GetLocation()).GetSafeNormal();


				//	CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(DirectionToPrevious_Original, DirectionToPrevious) * CurrentLink.OriginalPosition.GetRotation());


				tail_tip_offset = (CurrentLink.Position.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).Size();

				//	CurrentLink.InterpolatedPosition = CurrentLink.OriginalPosition;
			}



			//for (int32 i = 1; i < Chain.Num(); i++)
			for (int32 i = Chain.Num() - 2; i > 0; i--)
			{

				FDragonTailChainLink& CurrentLink = Chain[i];
				FDragonTailChainLink& PreviousLink = Chain[i + 1];




				//if (CurrentLink.chain_trace_hit.bBlockingHit || (closest_hit_bone > -1 ))
				if (CurrentLink.chain_trace_hit.bBlockingHit || (closest_hit_bone > -1 && i >= closest_hit_bone))
					//if (CurrentLink.chain_trace_hit.bBlockingHit)
				{

					FVector TracePoint = CurrentLink.OriginalPosition.GetLocation();

					UDragonIK_Library::SetVectorComponentAlongDirection(TracePoint, highest_impact_point, Character_UP_WS);


					if (i < Custom_Bone_Heights.Tail_Chain_Heights.Num() && use_custom_heights)
					{

						TracePoint += Character_UP_WS * Custom_Bone_Heights.Tail_Chain_Heights[i];

					}
					else
					{
						TracePoint += Character_UP_WS * Tail_Bone_Height;
					}


					CurrentLink.Last_Trace_Hit_Position = TracePoint;

					CurrentLink.HitAlpha = UKismetMathLibrary::FInterpTo(CurrentLink.HitAlpha, 1, owning_skel->GetWorld()->DeltaTimeSeconds, TraceBlock_Interpolation_Speed);
				}
				else
				{
					CurrentLink.HitAlpha = UKismetMathLibrary::FInterpTo(CurrentLink.HitAlpha, 0, owning_skel->GetWorld()->DeltaTimeSeconds, TraceUnBlock_Interpolation_Speed);
				}


				CurrentLink.Position.SetLocation(UKismetMathLibrary::VLerp(CurrentLink.OriginalPosition.GetLocation(), CurrentLink.Last_Trace_Hit_Position, CurrentLink.HitAlpha));

				//
				/*
				FTransform Inverse_InterpTransform = CurrentLink.InterpolatedPosition * owning_skel->GetComponentTransform().Inverse();
				FTransform Inverse_OriginalTransform = CurrentLink.OriginalPosition * owning_skel->GetComponentTransform().Inverse();


				FVector Inv_InterpPos = Inverse_InterpTransform.GetLocation();

				Inv_InterpPos.X = Inverse_OriginalTransform.GetLocation().X;
				Inv_InterpPos.Y = Inverse_OriginalTransform.GetLocation().Y;

				Inverse_InterpTransform.SetLocation(Inv_InterpPos);

				Inverse_InterpTransform = Inverse_InterpTransform * owning_skel->GetComponentTransform();

				CurrentLink.InterpolatedPosition = Inverse_InterpTransform;
				*/



				FTransform CS_InterpTransform = CurrentLink.InterpolatedPosition * owning_skel->GetComponentTransform().Inverse();
				FTransform CS_OriginalTransform = CurrentLink.OriginalPosition * owning_skel->GetComponentTransform().Inverse();

				float Original_New_Length = (CS_InterpTransform.GetLocation() - CS_OriginalTransform.GetLocation()).Size();


				FVector Inv_InterpPos = CS_OriginalTransform.GetLocation() + character_direction_vector_CS * (Original_New_Length);


				//	Inv_InterpPos.X = Inverse_OriginalTransform.GetLocation().X;
				//	Inv_InterpPos.Y = Inverse_OriginalTransform.GetLocation().Y;

				CS_InterpTransform.SetLocation(Inv_InterpPos);

				CS_InterpTransform = CS_InterpTransform * owning_skel->GetComponentTransform();

				CurrentLink.InterpolatedPosition = CS_InterpTransform;


				//




				CurrentLink.InterpolatedPosition.SetLocation(UKismetMathLibrary::VInterpTo(CurrentLink.InterpolatedPosition.GetLocation(), CurrentLink.Position.GetLocation(), owning_skel->GetWorld()->DeltaTimeSeconds, Animation_Interpolation_Speed));



				FVector DirectionToPrevious_Original = (PreviousLink.OriginalPosition.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).GetSafeNormal();

				FVector DirectionToPrevious = (PreviousLink.Position.GetLocation() - CurrentLink.InterpolatedPosition.GetLocation()).GetSafeNormal();



				FVector TargetLocation = PreviousLink.Position.GetLocation() - DirectionToPrevious * PreviousLink.Length;

				CurrentLink.Position.SetLocation(TargetLocation);


				//	CurrentLink.Position.SetRotation( FQuat::FindBetweenNormals(DirectionToPrevious_Original,DirectionToPrevious) * CurrentLink.OriginalPosition.GetRotation());
				CurrentLink.Position.SetRotation(CurrentLink.OriginalPosition.GetRotation());

				//	CurrentLink.Position.SetRotation(( New_Diff_Rot* Original_Diff_Rot.Inverse())* CurrentLink.OriginalPosition.GetRotation());


				CurrentLink.Position.SetScale3D(CurrentLink.OriginalPosition.GetScale3D());


			}







			for (int32 i = 1; i < Chain.Num(); i++)
			{

				FDragonTailChainLink& CurrentLink = Chain[i];
				FDragonTailChainLink& PreviousLink = Chain[i - 1];


				FVector DirectionToPrevious = (PreviousLink.Position.GetLocation() - CurrentLink.Position.GetLocation()).GetSafeNormal();

				FVector TargetLocation = PreviousLink.Position.GetLocation() - DirectionToPrevious * PreviousLink.Length;


				CurrentLink.Position.SetLocation(TargetLocation);


			}


			{

				FDragonTailChainLink& First_Link = Chain[0];
				FDragonTailChainLink& Second_Link = Chain[1];


				FQuat First_Second_Diff_Rot = FQuat::FindBetweenNormals((Second_Link.OriginalPosition.GetLocation() - First_Link.OriginalPosition.GetLocation()).GetUnsafeNormal(), (Second_Link.Position.GetLocation() - First_Link.Position.GetLocation()).GetUnsafeNormal());

				Chain[0].Position.SetRotation(First_Second_Diff_Rot * Chain[0].OriginalPosition.GetRotation());
			}


			for (int32 i = 1; i < Chain.Num(); i++)
			{


				FDragonTailChainLink& CurrentLink = Chain[i];
				FDragonTailChainLink& PreviousLink = Chain[i - 1];


				FQuat Diff_Rot = FQuat::FindBetweenNormals((PreviousLink.OriginalPosition.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).GetUnsafeNormal(), (PreviousLink.Position.GetLocation() - CurrentLink.Position.GetLocation()).GetUnsafeNormal());
				CurrentLink.Position.SetRotation((Diff_Rot)*CurrentLink.OriginalPosition.GetRotation());

				//CurrentLink.Position.SetRotation(CurrentLink.OriginalPosition.GetRotation());

			}



			////
		//	OutBoneTransforms.Empty();
		//	OutBoneTransforms.AddDefaulted(2);

		//	for (int chain_i=0; chain_i < OutBoneTransforms.Num(); chain_i++)
			for (int chain_i = 0; chain_i < Chain.Num(); chain_i++)
			{

				if (OutBoneTransforms.Num() > chain_i)
				{
					OutBoneTransforms[chain_i].BoneIndex = BoneIndices[chain_i];
				}


				FTransform CST_InterpolatedTransform = Chain[chain_i].Position * owning_skel->GetComponentTransform().Inverse();

				FTransform CST_OriginalTransform = Chain[chain_i].OriginalPosition * owning_skel->GetComponentTransform().Inverse();


				if (OutBoneTransforms.Num() > chain_i)
				{
					OutBoneTransforms[chain_i].Transform = Chain[chain_i].Position * owning_skel->GetComponentTransform().Inverse();
				}



			}


		}

	}


}







bool FAnimNode_TailSolver::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{

	//return true;


	if (Custom_Bone_Input.Custom_Bone_Structure.Num() < 2 && Use_Custom_Bone_Chain)
	{
		return false;
	}




	for (int i = 0; i < Custom_BoneList_Reference.Num(); i++)
	{

		if (Custom_BoneList_Reference[i].IsValidToEvaluate(RequiredBones) == false)
		{

			return false;

		}

	}


	if (ShouldSolverFail == true)
		return false;


	/*
	for (int i = 0; i < Custom_Bone_Input.Custom_Bone_Structure.Num(); i++)
	{

		FBoneReference Bone_Input = FBoneReference(Custom_Bone_Input.Custom_Bone_Structure[i]);
		Bone_Input.Initialize(RequiredBones);

		if (Bone_Input.IsValidToEvaluate() == false)
		{

			return false;

		}

	}
	*/


	return true;

}




FAnimNode_TailSolver::FAnimNode_TailSolver()
{
	ResizeDebugLocations(1);



}




//#if WITH_EDITOR
void FAnimNode_TailSolver::ResizeDebugLocations(int32 NewSize)
{

}
//#endif 


void FAnimNode_TailSolver::InitializeBoneReferences(FBoneContainer& RequiredBones)
{


	SavedBoneContainer = &RequiredBones;
	TipBone.Initialize(RequiredBones);
	RootBone.Initialize(RequiredBones);
	solver_counter = 0;

}



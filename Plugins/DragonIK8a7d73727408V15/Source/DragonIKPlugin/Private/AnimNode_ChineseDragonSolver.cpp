/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/


#include "AnimNode_ChineseDragonSolver.h"
#include "Animation/AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"






// Initialize the component pose as well as defining the owning skeleton
void FAnimNode_ChineseDragonSolver::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_DragonControlBase::Initialize_AnyThread(Context);
	//	ComponentPose.Initialize(Context);
	owning_skel = Context.AnimInstanceProxy->GetSkelMeshComponent();




	//	dragon_bone_data.Start_Spine = FBoneReference(dragon_input_data.Start_Spine);
}




void FAnimNode_ChineseDragonSolver::Evaluate_AnyThread(FPoseContext& Output)
{
}





void FAnimNode_ChineseDragonSolver::ConditionalDebugDraw(FPrimitiveDrawInterface* PDI, USkeletalMeshComponent* PreviewSkelMeshComp) const
{



}

//Perform update operations inside this
void FAnimNode_ChineseDragonSolver::UpdateInternal(const FAnimationUpdateContext& Context)
{
	FAnimNode_DragonControlBase::UpdateInternal(Context);
	

	component_scale = owner_skel_w_transform.GetScale3D().Z;


	if(Reset_To_Normal || Last_Alpha<0.01f || solver_counter < 10)
	{
		reset_to_normal_alpha = FMath::FInterpTo(reset_to_normal_alpha,0,owning_skel->GetWorld()->DeltaTimeSeconds,Reset_ON_Interpolation_Speed);
	}
	else
	{
		reset_to_normal_alpha = FMath::FInterpTo(reset_to_normal_alpha,1,owning_skel->GetWorld()->DeltaTimeSeconds, Reset_OFF_Interpolation_Speed);
	}

	if (Reset_To_Normal)
	{
		
	//	Initial_Warmup_Distance_Alpha = UKismetMathLibrary::FInterpTo(Initial_Warmup_Distance_Alpha,0,owning_skel->GetWorld()->DeltaTimeSeconds,Reset_Interpolation_Speed);
		Initial_Skel_Reference = owning_skel->GetComponentTransform();

	//	accumulated_warmup_distance = UKismetMathLibrary::FInterpTo(accumulated_warmup_distance, 0, owning_skel->GetWorld()->DeltaTimeSeconds, Reset_Interpolation_Speed);
	//	Initial_Warmup_Distance_Alpha = UKismetMathLibrary::FInterpTo(Initial_Warmup_Distance_Alpha, 0, owning_skel->GetWorld()->DeltaTimeSeconds, Reset_Interpolation_Speed);

	//	Initial_Warmup_Distance_Alpha = 0;
	//	accumulated_warmup_distance = 0;

		Reset_Commiting = true;
	}

	if (Reset_Commiting)
	{

		accumulated_warmup_distance = UKismetMathLibrary::FInterpTo(accumulated_warmup_distance, 0, owning_skel->GetWorld()->DeltaTimeSeconds, Reset_ON_Interpolation_Speed);
		Initial_Warmup_Distance_Alpha = UKismetMathLibrary::FInterpTo(Initial_Warmup_Distance_Alpha, 0, owning_skel->GetWorld()->DeltaTimeSeconds, Reset_ON_Interpolation_Speed);

		

	}

	//if (Initial_Warmup_Distance_Alpha <= 0.001f)
	if (Initial_Warmup_Distance_Alpha <= 0.0f)
	{
		Reset_Commiting = false;
	}


	if (Saved_Current_Owning_Skel_Trans == FVector(0, 0, 0))
	{
		Saved_Current_Owning_Skel_Trans = owning_skel->GetComponentLocation();
	}
	
	accumulated_warmup_distance += (Saved_Current_Owning_Skel_Trans - owning_skel->GetComponentLocation()).Size();
	Saved_Current_Owning_Skel_Trans = owning_skel->GetComponentLocation();
	


	if (owning_skel != nullptr && Reset_To_Normal == false && solver_counter > 5 && Reset_Commiting == false)
	{

		Initial_Warmup_Distance_Alpha = FMath::Max(Initial_Warmup_Distance_Alpha, accumulated_warmup_distance / Initial_Warmup_Distance);

	//	Initial_Warmup_Distance_Alpha = FMath::Max(Initial_Warmup_Distance_Alpha,(owning_skel->GetComponentLocation() - Initial_Skel_Reference.GetLocation()).Size()/Initial_Warmup_Distance);

		Initial_Warmup_Distance_Alpha = FMath::Min(Initial_Warmup_Distance_Alpha,1);
	}
	


	if (owning_skel != nullptr)
	{
		if (owning_skel->GetWorld()->WorldType == EWorldType::EditorPreview)
		{
			Initial_Warmup_Distance_Alpha = 1;
		}
	}

	
	


	if(solver_counter < 20)
	 solver_counter++;
	
}







//Nothing would be needed here
void FAnimNode_ChineseDragonSolver::EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Context)
{
}






void FAnimNode_ChineseDragonSolver::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{



	if(solver_counter > 1.f && owning_skel!=nullptr)
	{

		FTransform Debug_Check_Transform;

		if(owning_skel->GetWorld()->WorldType == EWorldType::EditorPreview)
			Debug_Check_Transform = CachedEffectorCSTransform;
		else
			Debug_Check_Transform = EffectorTransform;


		
		if(Initial_Effector_Reference.Equals(FTransform::Identity))
		{
			Initial_Effector_Reference = Debug_Check_Transform;

			//Rotation offset caused by mesh rotation difference
			FQuat Mesh_Rel_Diff = ( owning_skel->GetComponentRotation().Quaternion().Inverse()*owning_skel->GetOwner()->GetActorRotation().Quaternion());
			
			Initial_Effector_Reference.SetRotation(Mesh_Rel_Diff*Initial_Effector_Reference.GetRotation());


			if(owning_skel->GetComponentScale()!=FVector(0,0,0))
			{
			//	Initial_Effector_Reference.SetScale3D(Initial_Effector_Reference.GetScale3D()/owning_skel->GetComponentScale());
			}
			
		}


		if(Initial_Skel_Reference.Equals(FTransform::Identity))
		{
			Initial_Skel_Reference = owning_skel->GetComponentTransform();
		}

	
		FTransform Scaled_EffectorTransform = Debug_Check_Transform;
	

		if(Use_Scale_Add_Mode)
		{
			if(!Initial_Effector_Reference.GetLocation().Equals(FVector::ZeroVector))
			{
				Scaled_EffectorTransform.SetScale3D( Debug_Check_Transform.GetScale3D()/Initial_Effector_Reference.GetScale3D());

				//Scaled_EffectorTransform.SetScale3D( owning_skel->GetComponentScale());
			}
		}

		if(Use_Rot_Add_Mode)
		{
			if(!Initial_Effector_Reference.GetLocation().Equals(FVector::ZeroVector))
			{
				Scaled_EffectorTransform.SetRotation(Debug_Check_Transform.GetRotation()*Initial_Effector_Reference.GetRotation().Inverse());
			}
		}

		

		if (rotation_relative_to_mesh && Use_Advanced_Strict_Chain_Logic == false)
		{
			Scaled_EffectorTransform.SetRotation(Scaled_EffectorTransform.GetRotation() * owning_skel->GetComponentRotation().Quaternion().Inverse());
		}


		Scaled_EffectorTransform.SetRotation(Extra_Rotation_Offset.Quaternion()*Scaled_EffectorTransform.GetRotation());
		
		const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();	

		FTransform WS_EffectorTransform = Scaled_EffectorTransform*Output.AnimInstanceProxy->GetComponentTransform().Inverse();

		// Update EffectorLocation if it is based off a bone position
		FTransform CSEffectorTransform = WS_EffectorTransform;
	
		FVector const CSEffectorLocation = CSEffectorTransform.GetLocation();

		/*
#if WITH_EDITOR
		CachedEffectorCSTransform = CSEffectorTransform;
#endif
*/
		// Gather all bone indices between root and tip.
		TArray<FCompactPoseBoneIndex> BoneIndices;

		ShouldSolverFail = false;

		if(Use_Custom_Bone_Chain)
		{

			//for (int i = 0; i < Custom_Bone_Input.Custom_Bone_Structure.Num(); i++)
			for (int i = 0; i < Custom_BoneList_Reference.Num(); i++)
			{

			//	FBoneReference Bone_Input = FBoneReference(Custom_Bone_Input.Custom_Bone_Structure[i]);
			//	Bone_Input.Initialize(BoneContainer);

				


			//	Custom_BoneList_Reference.Insert(Bone_Input, i);


				BoneIndices.Insert(Custom_BoneList_Reference[i].CachedCompactPoseIndex, i);
		
				if (Custom_BoneList_Reference[i].CachedCompactPoseIndex.GetInt() < 0)
				{
					ShouldSolverFail = true;
				}
			}
		
		}
		
	

		if (ShouldSolverFail == false)
		{

			// Gather transforms
			int32 const NumTransforms = BoneIndices.Num();
			OutBoneTransforms.AddUninitialized(NumTransforms);


			TArray<FBoneTransform> TestBoneTransforms;
			TestBoneTransforms.AddUninitialized(NumTransforms);

			Solver_PreProcessing(Chain, OutBoneTransforms, BoneIndices, Output);
			Solver_PreProcessing(Test_Chain, TestBoneTransforms, BoneIndices, Output);


			FTransform TestTransform = Test_Chain[Test_Chain.Num() - 1].OriginalPosition;

			{

				//	FVector Test_Offset = (Test_Chain[Test_Chain.Num() - 1].OriginalPosition.GetLocation() - Test_Chain[0].OriginalPosition.GetLocation()).GetUnsafeNormal();
				//	Test_Offset.Z = 0;
				//	TestTransform.SetLocation(Test_Chain[0].OriginalPosition.GetLocation() + Test_Offset * 900);

				FVector Solving_W_Direction = UKismetMathLibrary::TransformDirection(owning_skel->GetComponentTransform(), Test_Ref_Forward_Axis);

				TestTransform.SetLocation(Test_Chain[0].OriginalPosition.GetLocation() + Solving_W_Direction * 100000000000000);
				//	TestTransform.SetLocation(Test_Chain[0].OriginalPosition.GetLocation() + owning_skel->GetRightVector() * 100000000000000);

				TestTransform.SetScale3D(Scaled_EffectorTransform.GetScale3D());

			}


			if (Reset_To_Normal && Use_Advanced_Strict_Chain_Logic)
			{
				path_record.Empty();


				FTransform RootTransform = Scaled_EffectorTransform;
				RootTransform.SetLocation(Chain[0].OriginalPosition.GetLocation());
				path_record.Add(RootTransform);

				FTransform HeadTransform = Scaled_EffectorTransform;
				HeadTransform.SetLocation(Chain[Chain.Num() - 1].OriginalPosition.GetLocation());
				path_record.Add(HeadTransform);


			}


			for (int i = 0; i < Chain.Num(); i++)
			{

				//if(Last_Alpha<0.01f)
				if (Reset_To_Normal || Last_Alpha < 0.01f || solver_counter < 10)
				{
					FChinaDragonChainLink& CurrentLink = Chain[i];
					CurrentLink.Position = CurrentLink.OriginalPosition;
					//	CurrentLink.InterpolatedPosition = CurrentLink.OriginalPosition;


					FTransform Path_Transform = CurrentLink.Position;
					Path_Transform.SetLocation(FVector(Path_Transform.GetLocation().X, Path_Transform.GetLocation().Y, Scaled_EffectorTransform.GetLocation().Z));
					Path_Transform.SetRotation(FRotator(0, 0, 0).Quaternion());


					CurrentLink.Position = Path_Transform;


				}
			}






			TArray<FTransform> Test_BoneTransformList = SolverProcessing(Output, Test_Chain, TestTransform, true, true);
			TArray<FTransform> BoneTransformList = SolverProcessing(Output, Chain, Scaled_EffectorTransform, Fix_Root_in_Place, false);


			int Using_StabilityBone = FMath::Min(Stability_Reference_Bone_Index, Test_Chain.Num() - 1);

			if (Using_StabilityBone > -1)
			{



				FVector Normalized_Stable_LocRef = FVector(0, 0, Test_BoneTransformList[Using_StabilityBone].GetLocation().Z) + Test_Ref_Forward_Axis * Pelvis_Positioning_Calibration_Value * -1;


				//	FVector Stability_Offset = FVector(0,0,CSEffectorLocation.Z) - Test_BoneTransformList[Using_StabilityBone].GetLocation();
				FVector Stability_Offset = Normalized_Stable_LocRef - Test_BoneTransformList[Using_StabilityBone].GetLocation();

				//	FVector Stability_Offset = (Test_Chain[Using_StabilityBone].OriginalPosition * owning_skel->GetComponentTransform().Inverse()).GetLocation() - Test_BoneTransformList[Using_StabilityBone].GetLocation();



				for (int32 TransformIndex = 0; TransformIndex < Test_BoneTransformList.Num() - 1; TransformIndex++)
				{

					Test_BoneTransformList[TransformIndex].SetLocation(Test_BoneTransformList[TransformIndex].GetLocation() + Stability_Offset);

				}
			}




			for (int32 LinkIndex = 0; LinkIndex < Chain.Num() - 1; LinkIndex++)
			{
				//if (Use_Advanced_Strict_Chain_Logic == false)
				{
					FVector ModifiedTestBoneValue = Test_BoneTransformList[LinkIndex].GetLocation();
					ModifiedTestBoneValue.Z = (Scaled_EffectorTransform * owning_skel->GetComponentTransform().Inverse()).GetLocation().Z;

					// Might need to uncomment this later..
					Test_BoneTransformList[LinkIndex].SetLocation(ModifiedTestBoneValue);
				}


				if (LinkIndex == 0 && Fix_Root_in_Place)
				{

				}
				else
				{

					BoneTransformList[LinkIndex].SetRotation(Extra_Rotation_Offset_Overall.Quaternion() * BoneTransformList[LinkIndex].GetRotation());


					if (LinkIndex == 0)
					{

					}
					else
					{
						Test_BoneTransformList[LinkIndex].SetRotation(Extra_Rotation_Offset_Overall.Quaternion() * Test_BoneTransformList[LinkIndex].GetRotation());
					}

				}


				if (Extra_Rotation_Offset_Per_Bone.Custom_Bone_Rotation_Offset.Num() > LinkIndex && Use_Custom_Bone_Chain)
				{
					BoneTransformList[LinkIndex].SetRotation(Extra_Rotation_Offset_Per_Bone.Custom_Bone_Rotation_Offset[LinkIndex].Quaternion() * BoneTransformList[LinkIndex].GetRotation());
					Test_BoneTransformList[LinkIndex].SetRotation(Extra_Rotation_Offset_Per_Bone.Custom_Bone_Rotation_Offset[LinkIndex].Quaternion() * Test_BoneTransformList[LinkIndex].GetRotation());

				}



			}



			FTransform Last_Second_Bone_Transform;

			for (int32 LinkIndex = Chain.Num() - 1; LinkIndex > -1; LinkIndex--)
			{

				FChinaDragonChainLink const& ChainLink = Chain[LinkIndex];


				//reset_to_normal_alpha
				FTransform Original_Unchanged_Transform = Chain[LinkIndex].OriginalPosition * owning_skel->GetComponentTransform().Inverse();
				FTransform Changed_Transform = Original_Unchanged_Transform;

				bool CheckLastDetectedBone = true;

				if (Last_Detected_Bone > -1)
				{
					CheckLastDetectedBone = (ChainLink.BoneIndex != Chain[Last_Detected_Bone].BoneIndex);

					if (Last_Detected_Bone == 0)
					{
						CheckLastDetectedBone = true;
					}
				}




				//	if(Preserve_Original_Pose && !IgnoredSolvedBones.Contains(ChainLink.BoneIndex) && CheckLastDetectedBone)
				if (Preserve_Original_Pose)
				{

					//	FTransform Solving_Alpha_Transform = UKismetMathLibrary::TLerp(Test_BoneTransformList[ChainLink.TransformIndex], BoneTransformList[ChainLink.TransformIndex],0.0f);


					Changed_Transform = Changed_Transform * (Test_BoneTransformList[ChainLink.TransformIndex].Inverse() * BoneTransformList[ChainLink.TransformIndex]);
					//	Changed_Transform = Changed_Transform * (Test_BoneTransformList[ChainLink.TransformIndex].Inverse() * Solving_Alpha_Transform);


					if (owning_skel->GetWorld()->WorldType == EWorldType::EditorPreview)
					{
						//Changed_Transform = (Test_BoneTransformList[ChainLink.TransformIndex]);

						Changed_Transform = Original_Unchanged_Transform;
					}
				}
				else
				{
					Changed_Transform = (BoneTransformList[ChainLink.TransformIndex]);
					//	Changed_Transform = (Test_BoneTransformList[ChainLink.TransformIndex]);

				}

				if (Enable_AnimBP_Viewport_Output == false)
				{
					Changed_Transform = Original_Unchanged_Transform;
				}


				if (Last_Detected_Bone == -1 && Use_Advanced_Strict_Chain_Logic)
				{
					Changed_Transform = Original_Unchanged_Transform;
				}


				//Changed_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform,Changed_Transform,1.00f);
			/*	if (Use_Interpolation)
				{
					Chain[LinkIndex].Interpolated_Positions_Final = UKismetMathLibrary::TInterpTo(Chain[LinkIndex].Interpolated_Positions_Final, Changed_Transform, owning_skel->GetWorld()->DeltaTimeSeconds, Interpolation_Speed);
				}
				else
			*/ {
					Chain[LinkIndex].Interpolated_Positions_Final = Changed_Transform;
				}

			/*
						if(LinkIndex == Chain.Num()-1)
						{
						//	FTransform TLerped_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform,Chain[LinkIndex].Interpolated_Positions_Final,reset_to_normal_alpha);
							FTransform TLerped_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform, Chain[LinkIndex].Interpolated_Positions_Final, reset_to_normal_alpha * Initial_Warmup_Distance_Alpha);
							OutBoneTransforms[ChainLink.TransformIndex] = FBoneTransform(FCompactPoseBoneIndex(Chain[LinkIndex].BoneIndex), TLerped_Transform);
						}
						else
						{
							FTransform TLerped_Transform =  UKismetMathLibrary::TLerp(Original_Unchanged_Transform,Chain[LinkIndex].Interpolated_Positions_Final,reset_to_normal_alpha* Initial_Warmup_Distance_Alpha);
						//	FTransform TLerped_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform, Chain[LinkIndex].Interpolated_Positions_Final, reset_to_normal_alpha);

							OutBoneTransforms[ChainLink.TransformIndex] = FBoneTransform(FCompactPoseBoneIndex(Chain[LinkIndex].BoneIndex), TLerped_Transform);
						}
			*/


			FTransform TLerped_Transform;

			if (Use_Warmup_Logic)
			{



				TLerped_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform, Chain[LinkIndex].Interpolated_Positions_Final, reset_to_normal_alpha * Initial_Warmup_Distance_Alpha);
			}
			else
			{
				TLerped_Transform = UKismetMathLibrary::TLerp(Original_Unchanged_Transform, Chain[LinkIndex].Interpolated_Positions_Final, reset_to_normal_alpha);
			}



			OutBoneTransforms[ChainLink.TransformIndex] = FBoneTransform(FCompactPoseBoneIndex(Chain[LinkIndex].BoneIndex), TLerped_Transform);


			if (LinkIndex == Chain.Num() - 2)
			{
				Last_Second_Bone_Transform = OutBoneTransforms[ChainLink.TransformIndex].Transform;

			}

			}



			FTransform Original_Offset_Transform = ((Chain[Chain.Num() - 1].OriginalPosition * owning_skel->GetComponentTransform().Inverse()) * (Chain[Chain.Num() - 2].OriginalPosition * owning_skel->GetComponentTransform().Inverse()).Inverse());
			FTransform Current_Offset_Transform = Original_Offset_Transform * Last_Second_Bone_Transform;

			OutBoneTransforms[Chain[Chain.Num() - 1].TransformIndex].Transform.SetLocation(Current_Offset_Transform.GetLocation());



		}

	}
	

	if(Last_Alpha == 0)
	{
		Alpha_Reset_Rest_Count--;
	}

	if(Alpha_Reset_Rest_Count <= 0)
	{
		//Last_Alpha = FMath::FInterpTo(Last_Alpha,ActualAlpha,owning_skel->GetWorld()->DeltaTimeSeconds,1);

		Last_Alpha = ActualAlpha;
		
		Alpha_Reset_Rest_Count = 60;
	}



	
}




void FAnimNode_ChineseDragonSolver::SmoothTransformArray(TArray<FTransform>& InOutTransforms, float AlphaVal)
{
    if (InOutTransforms.Num() < 3)
    {
        // If there are less than 3 transforms, there's not enough data to smooth.
        return;
    }

    // Create a copy of the original array to store the smoothed results.
    TArray<FTransform> SmoothedTransforms = InOutTransforms;

    // Start smoothing from the second element to the second-to-last element.
    for (int32 i = 1; i < InOutTransforms.Num() - 1; ++i)
    {
        // Get the previous, current, and next transforms
        const FTransform& PrevTransform = InOutTransforms[i - 1];
        const FTransform& CurrTransform = InOutTransforms[i];
        const FTransform& NextTransform = InOutTransforms[i + 1];

        // Smooth the position using linear interpolation
        FVector SmoothedPosition = FMath::Lerp(
            FMath::Lerp(PrevTransform.GetLocation(), CurrTransform.GetLocation(), AlphaVal),
            FMath::Lerp(CurrTransform.GetLocation(), NextTransform.GetLocation(), AlphaVal),
            AlphaVal
        );

        // Smooth the rotation using spherical interpolation (slerp)
        FQuat SmoothedRotation = FQuat::Slerp(
            FQuat::Slerp(PrevTransform.GetRotation(), CurrTransform.GetRotation(), AlphaVal),
            FQuat::Slerp(CurrTransform.GetRotation(), NextTransform.GetRotation(), AlphaVal),
            AlphaVal
        );

        // Smooth the scale using linear interpolation
        FVector SmoothedScale = FMath::Lerp(
            FMath::Lerp(PrevTransform.GetScale3D(), CurrTransform.GetScale3D(), AlphaVal),
            FMath::Lerp(CurrTransform.GetScale3D(), NextTransform.GetScale3D(), AlphaVal),
            AlphaVal
        );

        // Set the smoothed values back into the copied transform
    //    SmoothedTransforms[i].SetLocation(SmoothedPosition);
        SmoothedTransforms[i].SetRotation(SmoothedRotation);
        SmoothedTransforms[i].SetScale3D(SmoothedScale);
    }

    // Copy the smoothed transforms back into the original array
    InOutTransforms = SmoothedTransforms;
}



bool FAnimNode_ChineseDragonSolver::ChineseDragonChainSolving_Unstrict(TArray<FChinaDragonChainLink>& InOutChain, const FTransform& TargetPosition, bool Fix_Root)
{

	if (InOutChain.Num() == 0)
	{
		return false;
	}


	
	// The first bone moves freely toward the target position
	FChinaDragonChainLink& FirstLink = InOutChain[InOutChain.Num()-1];
	FirstLink.Position = (TargetPosition);

	//FirstLink.Position.SetScale3D(TargetPosition.GetScale3D()*owning_skel->GetComponentScale());
	
	//FirstLink.Position.SetScale3D(FVector(1,1,1));

	int i_min = -1;

	if(Fix_Root)
	{
		i_min = 0;
		


		FChinaDragonChainLink& CurrentLink = InOutChain[0];
		//FChinaDragonChainLink& PreviousLink = InOutChain[1];

	
		CurrentLink.Position.SetLocation(CurrentLink.OriginalPosition.GetLocation());
		

		
	}
	
	// Iterate through the rest of the chain
	for (int32 i = InOutChain.Num()-2; i > i_min; i--)
	{
		FChinaDragonChainLink& CurrentLink = InOutChain[i];
		FChinaDragonChainLink& PreviousLink = InOutChain[i + 1];

		if(Limit_Z_Solving)
		{
			FVector Temp_Vect_Current = CurrentLink.Position.GetLocation();
			FVector Temp_Vect_Previous = PreviousLink.Position.GetLocation();
			Temp_Vect_Current.Z = TargetPosition.GetLocation().Z;
			Temp_Vect_Previous.Z = TargetPosition.GetLocation().Z;

			
			CurrentLink.Position.SetLocation(Temp_Vect_Current);
			PreviousLink.Position.SetLocation(Temp_Vect_Previous);

			
		}

		FVector DirectionToPrevious_Original = (PreviousLink.OriginalPosition.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).GetSafeNormal();
		
		FVector DirectionToPrevious = (PreviousLink.Position.GetLocation() - CurrentLink.Position.GetLocation()).GetSafeNormal();

		//DirectionToPrevious = UKismetMathLibrary::VLerp(DirectionToPrevious, owning_skel->GetRightVector(),0.1f);

		//float AngleDifference = FMath::RadiansToDegrees( FVector::DotProduct(DirectionToPrevious, owning_skel->GetForwardVector()));

	//	FVector Solving_W_Direction = FVector::CrossProduct(owning_skel->GetUpVector(), UKismetMathLibrary::TransformDirection(owning_skel->GetComponentTransform(), Test_Ref_Forward_Axis));
		FVector Solving_W_Direction = UKismetMathLibrary::TransformDirection(owning_skel->GetComponentTransform(), Test_Ref_Forward_Axis);


		if (Continous_Normalization)
		{

			float Skel_Speed = 1;

			if (owning_skel->GetOwner())
			{

				
					Skel_Speed = Normalization_Multiplier_Rel_Velocity.GetRichCurve()->Eval(owning_skel->GetOwner()->GetVelocity().Size());

					Skel_Speed = FMath::Clamp<float>(Skel_Speed, 0.01f, 10000000000000);

			}


			DirectionToPrevious = UKismetMathLibrary::VInterpTo(DirectionToPrevious, Solving_W_Direction, owning_skel->GetWorld()->DeltaTimeSeconds, Normalization_Speed*Skel_Speed);

			DirectionToPrevious = DirectionToPrevious.GetSafeNormal();

		}

		//GEngine->AddOnScreenDebugMessage(-1, 0.05f, FColor::Red, " AngleDifference : " + FString::SanitizeFloat(AngleDifference));



		if(Limit_Z_Solving)
		{
			DirectionToPrevious.Z = 0;
		}
		
		
		FVector TargetLocation = PreviousLink.Position.GetLocation() - DirectionToPrevious * CurrentLink.Length;

	//	TargetLocation.Z = InOutChain[0].OriginalPosition.GetLocation().Z;
		
		CurrentLink.Position.SetLocation(TargetLocation);

		//Rotation logic altered here
	//	CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(DirectionToPrevious_Original,DirectionToPrevious));

	//	CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(owning_skel->GetRightVector(),DirectionToPrevious));
		CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(Solving_W_Direction, DirectionToPrevious));


	//	CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(FVector(1,0,0), DirectionToPrevious));



		CurrentLink.Position.SetScale3D(FVector(1,1,1));
		
	//	CurrentLink.Position.SetRotation(FRotator(0,0,0).Quaternion());
		
	}


	if(Fix_Root)
	{
		for (int32 i = 1; i < InOutChain.Num(); i++)
		{
			FChinaDragonChainLink& CurrentLink = InOutChain[i];
			FChinaDragonChainLink& PreviousLink = InOutChain[i - 1];


			//	FVector DirectionToPrevious_Original = (PreviousLink.OriginalPosition.GetLocation() - CurrentLink.OriginalPosition.GetLocation()).GetSafeNormal();
		
			FVector DirectionToPrevious = (PreviousLink.Position.GetLocation() - CurrentLink.Position.GetLocation()).GetSafeNormal();
		
			FVector TargetLocation = PreviousLink.Position.GetLocation() - DirectionToPrevious * PreviousLink.Length;
		
			CurrentLink.Position.SetLocation(TargetLocation);

			//	CurrentLink.Position.SetRotation(FQuat::FindBetweenNormals(DirectionToPrevious_Original,DirectionToPrevious));

		
		}
	}
	
	
	return true;
}



void FAnimNode_ChineseDragonSolver::Solver_PreProcessing(TArray<FChinaDragonChainLink>& ChainTemp,TArray<FBoneTransform>& OutTransforms,TArray<FCompactPoseBoneIndex> BoneIndices,FComponentSpacePoseContext& Output)
{

	int32 const NumTransforms = BoneIndices.Num();

	if(ChainTemp.Num() == 0)
		{
			owner_skel_w_transform = owning_skel->GetComponentTransform();
			
			ChainTemp.AddDefaulted(NumTransforms);
		
	

			// Start with Root Bone
			{
				const FCompactPoseBoneIndex& RootBoneIndex = BoneIndices[0];
				FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(RootBoneIndex);

			

				BoneCSTransform = BoneCSTransform*owning_skel->GetComponentTransform();

				BoneCSTransform.SetRotation(FQuat::Identity);
				BoneCSTransform.SetScale3D(FVector(1,1,1));



				OutTransforms[0] = FBoneTransform(RootBoneIndex, BoneCSTransform);
				//	Chain.Add(FChinaDragonChainLink(BoneCSTransform.GetLocation(), 0.f, RootBoneIndex, 0));

				ChainTemp[0] = FChinaDragonChainLink(BoneCSTransform, 0.f, RootBoneIndex, 0);

				ChainTemp[0].InterpolatedPosition = BoneCSTransform;
			}

			// Go through remaining transforms
			for (int32 TransformIndex = 1; TransformIndex < NumTransforms; TransformIndex++)
			{
				const FCompactPoseBoneIndex& BoneIndex = BoneIndices[TransformIndex];

				FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(BoneIndex);

				//BoneCSTransform = BoneCSTransform*(owning_skel->GetComponentTransform().Inverse()*owner_skel_w_transform);

				BoneCSTransform = BoneCSTransform*owning_skel->GetComponentTransform();

				BoneCSTransform.SetRotation(FQuat::Identity);
				BoneCSTransform.SetScale3D(FVector(1,1,1));

		
				FVector const BoneCSPosition = BoneCSTransform.GetLocation();

				OutTransforms[TransformIndex] = FBoneTransform(BoneIndex, BoneCSTransform);

				// Calculate the combined length of this segment of skeleton
				double const BoneLength = FVector::Dist(BoneCSPosition, OutTransforms[TransformIndex-1].Transform.GetLocation());

			//	if (!FMath::IsNearlyZero(BoneLength))
				{
			
					//	Chain.Add(FChinaDragonChainLink(BoneCSPosition, BoneLength, BoneIndex, TransformIndex));

					ChainTemp[TransformIndex] = FChinaDragonChainLink(BoneCSTransform, 0, BoneIndex, TransformIndex);

					ChainTemp[TransformIndex].InterpolatedPosition = BoneCSTransform;
			

				}
			/*	else
				{
					// Mark this transform as a zero length child of the last link.
					// It will inherit position and delta rotation from parent link.
					FChinaDragonChainLink & ParentLink = ChainTemp[ChainTemp.Num()-1];
					ParentLink.ChildZeroLengthTransformIndices.Add(TransformIndex);
				}
			*/
			}



			for (int32 TransformIndex = ChainTemp.Num()-2; TransformIndex > -1; TransformIndex--)
				//for (int32 TransformIndex = 1; TransformIndex < NumTransforms-1; TransformIndex++)
			{
			
				const FCompactPoseBoneIndex& BoneIndex1 = FCompactPoseBoneIndex(ChainTemp[TransformIndex].BoneIndex);
				const FCompactPoseBoneIndex& BoneIndex2 = FCompactPoseBoneIndex(ChainTemp[TransformIndex+1].BoneIndex);

			
				FTransform BoneCSTransform1 = Output.Pose.GetComponentSpaceTransform(BoneIndex1)*owning_skel->GetComponentTransform();
				FTransform BoneCSTransform2 = Output.Pose.GetComponentSpaceTransform(BoneIndex2)*owning_skel->GetComponentTransform();

			
				double const BoneLength = FVector::Dist(BoneCSTransform1.GetLocation(), BoneCSTransform2.GetLocation());

				ChainTemp[TransformIndex].Length = BoneLength;
			
			
			}
			//Chain[1].Length = 2000;
		}



		for (int32 TransformIndex = 0; TransformIndex < NumTransforms; TransformIndex++)
		{

			const FCompactPoseBoneIndex BoneIndex1 = FCompactPoseBoneIndex(ChainTemp[TransformIndex].BoneIndex);

			if(BoneIndex1.IsValid())
			{
				FTransform BoneCSTransform1 = Output.Pose.GetComponentSpaceTransform(BoneIndex1);
		
				ChainTemp[TransformIndex].OriginalPosition = BoneCSTransform1*owning_skel->GetComponentTransform();

				FTransform Identity_Rotation_Ref = Initial_Skel_Reference;
				Identity_Rotation_Ref.SetRotation(FRotator(0,0,0).Quaternion());

				ChainTemp[TransformIndex].SavedRotation = (BoneCSTransform1 * Identity_Rotation_Ref).Rotator();

			//	ChainTemp[TransformIndex].SavedRotation = (BoneCSTransform1 * owning_skel->GetComponentTransform()).Rotator();

			}
		}
	
}



TArray<FTransform> FAnimNode_ChineseDragonSolver::SolverProcessing(FComponentSpacePoseContext& Output,TArray<FChinaDragonChainLink>& ChainTemp,FTransform InputTransform,bool Fix_Root,bool IsTestChain)
{



	
		//bool bBoneLocationUpdated = true;

		if(Use_Advanced_Strict_Chain_Logic && IsTestChain == false)
		{
			ChineseDragonChainSolving_Strict(Output,ChainTemp, InputTransform, path_record);
		}
		else
		{
			ChineseDragonChainSolving_Unstrict(ChainTemp, InputTransform, Fix_Root);
		}

		//ChineseDragonChainSolving_Unstrict(Chain, Scaled_EffectorTransform);



		//OutBoneTransforms[Chain[Chain.Num()-1].TransformIndex].Transform = Chain[Chain.Num()-1].Position*owning_skel->GetComponentTransform().Inverse();


		TArray<FTransform> BoneTransformList = TArray<FTransform>();
		BoneTransformList.AddDefaulted(ChainTemp.Num());
		
		for (int32 LinkIndex = ChainTemp.Num()-1; LinkIndex > -1; LinkIndex--)
		{
			const FCompactPoseBoneIndex BoneIndex = FCompactPoseBoneIndex(ChainTemp[LinkIndex].BoneIndex);
			FTransform ChainPos;

			FTransform Chain_Transform_Var = ChainTemp[LinkIndex].Position;
		//	Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation()*ChainTemp[LinkIndex].OriginalPosition.GetRotation());

			if (LinkIndex == ChainTemp.Num() - 1)
			{
				Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation() * ChainTemp[LinkIndex].OriginalPosition.GetRotation());
			}
			else
			{
				Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation() * ChainTemp[LinkIndex].SavedRotation.Quaternion());
			}


			if (Use_Advanced_Strict_Chain_Logic == false || IsTestChain)
			{
				Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation() * ChainTemp[LinkIndex].OriginalPosition.GetRotation());
			}



			Chain_Transform_Var.SetScale3D(ChainTemp[LinkIndex].Position.GetScale3D()*ChainTemp[LinkIndex].OriginalPosition.GetScale3D());

			
			
		//	Chain_Transform_Var.AddToTranslation( (Complete_Bone_Offset));




			if (IsTestChain == false && Use_Advanced_Strict_Chain_Logic == true)
			{

				Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation() * ChainTemp[LinkIndex].SavedRotation.Quaternion());

			//	Chain_Transform_Var.SetRotation((owning_skel->GetComponentRotation().Quaternion().Inverse()) * Chain_Transform_Var.GetRotation());
			}

			if (IsTestChain)
			{
				if (LinkIndex == ChainTemp.Num() - 1)
				{

					Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].OriginalPosition.GetRotation());


					//	Chain_Transform_Var.AddToTranslation(Chain[LinkIndex].OriginalPosition.GetLocation()-Chain[LinkIndex-1].OriginalPosition.GetLocation());
				}


				if (LinkIndex == 0)
				{
					Chain_Transform_Var.SetRotation(ChainTemp[LinkIndex].Position.GetRotation() * ChainTemp[LinkIndex].OriginalPosition.GetRotation());
				}

			}
	
		
		//	if(Interpolation_Is_World)
			{

			//	float Selected_Interpolation_Speed = Interpolation_Speed;

				float Normalized_Distance_Alpha = FMath::Clamp<float>( UKismetMathLibrary::NormalizeToRange(Initial_Warmup_Distance_Alpha,0.5f,1.0f),0,1);

				float Selected_Interpolation_Speed = UKismetMathLibrary::Lerp( Reset_OFF_Interpolation_Speed,Interpolation_Speed, Normalized_Distance_Alpha);
				
				/*
				if(Initial_Warmup_Distance_Alpha < 0.99f)
				{
					Selected_Interpolation_Speed = Reset_OFF_Interpolation_Speed;
					//Selected_Interpolation_Speed = UKismetMathLibrary::Lerp( Reset_OFF_Interpolation_Speed,Selected_Interpolation_Speed, Initial_Warmup_Distance_Alpha);
				}*/


				
				if (Reset_To_Normal == true || Reset_Commiting == true || solver_counter < 10)
				{
					Selected_Interpolation_Speed = Reset_ON_Interpolation_Speed;
				}
				//else if(Initial_Warmup_Distance_Alpha < 1)

				


				if(ChainTemp[LinkIndex].InterpolatedPosition.ContainsNaN() || Use_Interpolation == false || IsTestChain || Initial_Warmup_Distance_Alpha <= 0.001f)
					ChainTemp[LinkIndex].InterpolatedPosition = Chain_Transform_Var ;
				else
					ChainTemp[LinkIndex].InterpolatedPosition = UKismetMathLibrary::TInterpTo(ChainTemp[LinkIndex].InterpolatedPosition,Chain_Transform_Var,owning_skel->GetWorld()->DeltaTimeSeconds, Selected_Interpolation_Speed);

				if(solver_counter < 10)
					ChainTemp[LinkIndex].InterpolatedPosition = Chain_Transform_Var;

				//

				if (Limit_Z_Solving)
				{
					FVector Chain_Transform_Vect = ChainTemp[LinkIndex].InterpolatedPosition.GetLocation();
					Chain_Transform_Vect.Z = Chain_Transform_Var.GetLocation().Z;
					ChainTemp[LinkIndex].InterpolatedPosition.SetLocation(Chain_Transform_Vect);
				}


			//
			// 
			// 
			//	ChainPos = (ChainTemp[LinkIndex].InterpolatedPosition)*owning_skel->GetComponentTransform().Inverse();

				ChainPos = (ChainTemp[LinkIndex].InterpolatedPosition * owning_skel->GetComponentTransform().Inverse());


			}
			/*
			else
			{


				if(ChainTemp[LinkIndex].InterpolatedPosition.ContainsNaN() || Use_Interpolation == false || IsTestChain)
					ChainTemp[LinkIndex].InterpolatedPosition = Chain_Transform_Var*owning_skel->GetComponentTransform().Inverse();
				else
					ChainTemp[LinkIndex].InterpolatedPosition = UKismetMathLibrary::TInterpTo(ChainTemp[LinkIndex].InterpolatedPosition,Chain_Transform_Var*owning_skel->GetComponentTransform().Inverse(),owning_skel->GetWorld()->DeltaTimeSeconds,Interpolation_Speed*Initial_Warmup_Distance_Alpha);

				if(solver_counter < 10)
					ChainTemp[LinkIndex].InterpolatedPosition = Chain_Transform_Var*owning_skel->GetComponentTransform().Inverse();
			
				ChainPos = (ChainTemp[LinkIndex].InterpolatedPosition);
			
			}
			*/
			FChinaDragonChainLink const & ChainLink = ChainTemp[LinkIndex];

			BoneTransformList[ChainLink.TransformIndex] = (ChainPos);
			
		}
		

		return BoneTransformList;
	
}



bool FAnimNode_ChineseDragonSolver::ChineseDragonChainSolving_Strict(FComponentSpacePoseContext& Output,TArray<FChinaDragonChainLink>& InOutChain, const FTransform& TargetPosition, TArray<FTransform>& RecordedPath)
{


	if (InOutChain.Num() == 0)
	{
		return false;
	}

	
	
	 // The first bone moves freely toward the target position
	FChinaDragonChainLink& FirstLink = InOutChain[InOutChain.Num()-1];
//	FirstLink.Position.SetLocation(TargetPosition);


	float DesiredDistance = 0;

	//DesiredDistance += InOutChain[0].Length;


	if(RecordedPath.IsEmpty())
	{
		
		
	}
	else if(solver_counter > 5)
	{
		FirstLink.Position = (TargetPosition);

		
	}
	

	if(solver_counter > 5)
	{

		FVector RecordedPath_Last = TargetPosition.GetLocation();

		if (RecordedPath.Num() > 0)
		{
			RecordedPath_Last = RecordedPath.Last().GetLocation();
		}

		FVector TargetPos_Temp = TargetPosition.GetLocation();

		if (Limit_Z_Solving)
		{
			RecordedPath_Last.Z = 0;
			TargetPos_Temp.Z = 0;
		}


		// Record the movement of the first bone (acting like a train track)
	//	if (RecordedPath.Num() == 0 || ( (RecordedPath.Last().GetLocation() - TargetPosition.GetLocation()).Size() > Precision))
		if (RecordedPath.Num() == 0 || ( (RecordedPath_Last - TargetPos_Temp).Size() > Precision))
		{


			
			RecordedPath.Add(TargetPosition);
			
			
			// Keep the recorded path within a reasonable limit to avoid memory overload
			if (RecordedPath.Num() > Maximum_Bone_Extension)
			{
				RecordedPath.RemoveAt(0);
			}

			SmoothTransformArray(RecordedPath, 0.5f);
		}


	//	GEngine->AddOnScreenDebugMessage(-1, 0.05f, FColor::Red, " Maximum_Bone_Extension : "+ FString::SanitizeFloat(Maximum_Bone_Extension) );

		if (RecordedPath.Num() == 1)
		{
			RecordedPath.Empty();

			FTransform Placeholder_Transform = InOutChain[0].OriginalPosition;
			Placeholder_Transform.SetRotation(TargetPosition.GetRotation());
			Placeholder_Transform.SetScale3D(TargetPosition.GetScale3D());

		//	float SectionSize = (InOutChain[0].OriginalPosition.GetLocation() - InOutChain[InOutChain.Num()-1].OriginalPosition.GetLocation()).Size() / (float)InOutChain.Num();
			float ChunkSize = (InOutChain[0].OriginalPosition.GetLocation() - InOutChain[InOutChain.Num() - 1].OriginalPosition.GetLocation()).Size() / Precision;


		//	for (int p_i = 0; p_i < ChunkSize; p_i++)
			{
				RecordedPath.Add(Placeholder_Transform);

				
			}



			


			for (int c_i = 0; c_i < InOutChain.Num(); c_i++)
		//	for (int c_i = InOutChain.Num() - 1; c_i > -1; c_i--)
			{

				
				{
					FChinaDragonChainLink& CurrentLink = InOutChain[c_i];

				//	UKismetSystemLibrary::DrawDebugSphere(owning_skel->GetWorld(), CurrentLink.OriginalPosition.GetLocation(), 25, 12, FLinearColor::Red, 50.1f);

					CurrentLink.Position = CurrentLink.OriginalPosition;

					CurrentLink.Position.SetLocation(FVector(CurrentLink.Position.GetLocation().X, CurrentLink.Position.GetLocation().Y, TargetPosition.GetLocation().Z));

				//	CurrentLink.Position.SetLocation(RecordedPath[c_i+1].GetLocation());
					CurrentLink.Position.SetRotation(TargetPosition.GetRotation());
				//	CurrentLink.Position.SetScale3D(RecordedPath[c_i+1].GetScale3D());
				}

			}

		}



		if (Limit_Z_Solving)
		{
			for (int r_i = 0; r_i < RecordedPath.Num(); r_i++)
			{
				FVector recorded_loc = RecordedPath[r_i].GetLocation();

				recorded_loc.Z = TargetPosition.GetLocation().Z;

				RecordedPath[r_i].SetLocation(recorded_loc);

			}
		}

		
		

		
	}


	Maximum_Bone_Extension = 0;
	
	//GEngine->AddOnScreenDebugMessage(-1, 0.05f, FColor::Red, " RecordedPath : " + FString::SanitizeFloat(Initial_Warmup_Distance_Alpha));

	Last_Detected_Bone = -1;
	
	// Now move the rest of the bones along the recorded path
	for (int32 i = InOutChain.Num()-2; i > -1; i--)
	{
		FChinaDragonChainLink& CurrentLink = InOutChain[i];

		DesiredDistance += InOutChain[i].Length;

		Maximum_Bone_Extension += InOutChain[i].Length;

		IgnoredSolvedBones.AddUnique(InOutChain[i].BoneIndex);

		
		// Traverse the recorded path to find the point that maintains the distance from the previous bone
		FTransform TargetPositionForCurrentBone = FTransform::Identity;
	//	TargetPositionForCurrentBone = InOutChain[i].OriginalPosition;
	//	TargetPositionForCurrentBone.SetRotation(TargetPosition.GetRotation());

		if (Reset_To_Normal || Reset_Commiting)
		{
			if (i == 0)
			{
				CurrentLink.Position.SetRotation(TargetPosition.GetRotation());
				CurrentLink.InterpolatedPosition.SetRotation(TargetPosition.GetRotation());
			}

		}

	//	TargetPositionForCurrentBone.SetLocation(FVector(TargetPositionForCurrentBone.GetLocation().X, TargetPositionForCurrentBone.GetLocation().Y, TargetPosition.GetLocation().Z));

		float AccumulatedDistance = 0.01f;
		

		
		
		for (int32 PathIndex = RecordedPath.Num() - 2; PathIndex > -1; PathIndex--)
		{

			

			
			FTransform PathPoint = RecordedPath[PathIndex];
			AccumulatedDistance += (RecordedPath[PathIndex+1].GetLocation() - PathPoint.GetLocation()).Size();
			
			FVector Chain_Distance_Diff = ((InOutChain[i].OriginalPosition).GetLocation() - (InOutChain[i+1].OriginalPosition).GetLocation());

			
			// If the accumulated distance reaches the desired length, place the bone at this position
			if (AccumulatedDistance >= DesiredDistance)
			{

				float lerp_segment = FMath::Clamp( 1 - ((RecordedPath.Last().GetLocation() - TargetPosition.GetLocation()).Size()/Precision) ,0,1);
				

				if((PathIndex-1) > -1)
				{
						TargetPositionForCurrentBone.SetLocation(InOutChain[i+1].Position.GetLocation() - (InOutChain[i+1].Position.GetLocation() - UKismetMathLibrary::VLerp(RecordedPath[PathIndex].GetLocation(),RecordedPath[PathIndex-1].GetLocation(),lerp_segment)).GetUnsafeNormal()*Chain_Distance_Diff.Size());
						TargetPositionForCurrentBone.SetRotation(UKismetMathLibrary::Quat_Slerp(RecordedPath[PathIndex].GetRotation(),RecordedPath[PathIndex-1].GetRotation(),lerp_segment));
						TargetPositionForCurrentBone.SetScale3D(UKismetMathLibrary::VLerp(RecordedPath[PathIndex].GetScale3D(),RecordedPath[PathIndex-1].GetScale3D(),lerp_segment));
				}
				else
				{
						TargetPositionForCurrentBone.SetLocation( InOutChain[i+1].Position.GetLocation() - (InOutChain[i+1].Position.GetLocation() - RecordedPath[PathIndex].GetLocation()).GetUnsafeNormal()*Chain_Distance_Diff.Size() );
						TargetPositionForCurrentBone.SetRotation(RecordedPath[PathIndex].GetRotation());
						TargetPositionForCurrentBone.SetScale3D(RecordedPath[PathIndex].GetScale3D());
						
				}

				//Last_Detected_Bone = InOutChain[i].BoneIndex;
				Last_Detected_Bone = i;
				
				IgnoredSolvedBones.Remove(InOutChain[i].BoneIndex);

			//	TargetPositionForCurrentBone = RecordedPath[PathIndex];
				
				//GEngine->AddOnScreenDebugMessage(-1, 0.05f, FColor::Red, " BoneIndex : "+ FString::SanitizeFloat(InOutChain[i].BoneIndex) );

				
				break;
			}
		}

		if (!TargetPositionForCurrentBone.Equals(FTransform::Identity))
		{
			CurrentLink.Position = (TargetPositionForCurrentBone);
		}
		

	}
	
	



	
	
	 return true;
}


bool FAnimNode_ChineseDragonSolver::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{

	if (ShouldSolverFail)
		return false;

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

	

	
	if(Use_Custom_Bone_Chain == false)
	{
		return
			(
			TipBone.IsValidToEvaluate(RequiredBones)
			&& RootBone.IsValidToEvaluate(RequiredBones)
			&& Precision > 0
			&& RequiredBones.BoneIsChildOf(TipBone.BoneIndex, RootBone.BoneIndex)
			);
	}

	return true;
	
}




FAnimNode_ChineseDragonSolver::FAnimNode_ChineseDragonSolver()
{	
		ResizeDebugLocations(1);

		FRichCurve* Normalization_Multiplier_Rel_Data = Normalization_Multiplier_Rel_Velocity.GetRichCurve();
		Normalization_Multiplier_Rel_Data->AddKey(0.f, 1.f);
		Normalization_Multiplier_Rel_Data->AddKey(100.f, 0.5f);

}




//#if WITH_EDITOR
void FAnimNode_ChineseDragonSolver::ResizeDebugLocations(int32 NewSize)
{
	
}
//#endif 


void FAnimNode_ChineseDragonSolver::InitializeBoneReferences(FBoneContainer& RequiredBones)
{


	SavedBoneContainer = &RequiredBones;
	TipBone.Initialize(RequiredBones);
	RootBone.Initialize(RequiredBones);
	solver_counter = 0;

	Custom_BoneList_Reference.Empty();

	if (Use_Custom_Bone_Chain)
	{

		for (int i = 0; i < Custom_Bone_Input.Custom_Bone_Structure.Num(); i++)
		{

			FBoneReference Bone_Input = FBoneReference(Custom_Bone_Input.Custom_Bone_Structure[i]);
			Bone_Input.Initialize(RequiredBones);

			Custom_BoneList_Reference.Insert(Bone_Input, i);

		}

	}

}



/* Copyright (C) Eternal Monke Games - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Mansoor Pathiyanthra <codehawk64@gmail.com , mansoor@eternalmonke.com>, 2021
*/


#include "DragonIKWeaponPhysicsComponent.h"

#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UDragonIKWeaponPhysicsComponent::UDragonIKWeaponPhysicsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	

	// ...
}


// Called when the game starts
void UDragonIKWeaponPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();


}


void UDragonIKWeaponPhysicsComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	//UPhysicsHandleComponent* test;
	//test->DestroyComponent

	if(weapon_physics_handle != nullptr)
	weapon_physics_handle->DestroyComponent();

	if (weapon_physics_constraint != nullptr)
	weapon_physics_constraint->DestroyComponent();

}


// Called every frame
void UDragonIKWeaponPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!dominant_hand_transform.Equals(FTransform::Identity))
	 Temp_Dom_Transform = dominant_hand_transform;
	
	if (owning_skeleton != nullptr)
	{
		
		ModifyWeaponTransform();
		
		if (perform_grab_weapon == true)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, " zweapon_handler_component....="+dominant_hand_transform.ToString());
			//UKismetSystemLibrary::PrintText(owning_skeleton->GetOwner()->GetWorld(),FText::AsCultureInvariant(" zweapon_handler_component TEST!"+FString::SanitizeFloat(grab_counter)),true,false,FLinearColor::Yellow,0.1f);

			
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, " perform");
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, " perform_grab_weapon : "+ FString::SanitizeFloat(owning_skeleton->GetLastRenderTime()));

			//USkeletalMeshComponent* skeleton_input;

			if (owning_skeleton->GetLastRenderTime() > 0 || UKismetSystemLibrary::IsServer(owning_skeleton->GetWorld()))
			 grab_counter += GetWorld()->DeltaTimeSeconds;
			
			//dominant_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));
			if (grab_counter > 0.2f)
			 DelayedGrabEvent();

			if(grab_counter > 3)
			 perform_grab_weapon = false;
		}

	}

}

void UDragonIKWeaponPhysicsComponent::FirstTimeInitialization(USkeletalMeshComponent* skeleton_input)
{

	if (skeleton_input != nullptr)
	{
		owning_skeleton = skeleton_input;


		FString wephandles_name_str = "Weapon Physics Handle";
		wephandles_name_str.AppendInt(physanim_tag_index);


		FString wepconstraint_name_str = "Weapon Physics Constraint";
		wepconstraint_name_str.AppendInt(physanim_tag_index);


		weapon_physics_handle = NewObject<UPhysicsHandleComponent>(owning_skeleton->GetOwner(), FName(wephandles_name_str));

		weapon_physics_constraint = NewObject<UPhysicsConstraintComponent>(owning_skeleton->GetOwner(), FName(wepconstraint_name_str));



		if (!weapon_physics_handle->IsRegistered())
		{
			weapon_physics_handle->RegisterComponentWithWorld(owning_skeleton->GetOwner()->GetWorld());
			weapon_physics_handle->RegisterComponent();
		}

		if (!weapon_physics_constraint->IsRegistered())
		{
			weapon_physics_constraint->RegisterComponentWithWorld(owning_skeleton->GetOwner()->GetWorld());
			weapon_physics_constraint->RegisterComponent();
		}

		weapon_physics_handle->SetIsReplicated(true);
		weapon_physics_constraint->SetIsReplicated(true);

		weapon_physics_handle->LinearDamping = Linear_Damp;
		weapon_physics_handle->AngularDamping = Angular_Damp;

		weapon_physics_handle->LinearStiffness = Linear_Strength;
		weapon_physics_handle->AngularStiffness = Angular_Strength;

		weapon_physics_handle->InterpolationSpeed = Interpolation_Speed * 10;

		weapon_physics_handle->Activate();
		weapon_physics_constraint->Activate();

		/*
		weapon_physics_constraint->AttachToComponent(owning_skeleton,FAttachmentTransformRules::KeepWorldTransform,weapon_grabbed_bone);
		weapon_physics_constraint->SetConstrainedComponents(owning_skeleton,weapon_grabbed_bone,active_weapon_component,"None");
		*/

		//GetOwner()->AddInstanceComponent(weapon_physics_handle);

		arm_grabbing_status.Empty();
		arm_reset_queue_index.Empty();
		Frozen_Elbow_Component_Status.Empty();


		handbone_array_boneName.Empty();
		handbone_array_boneIndex.Empty();




		for (int i = 0; i < 10; i++)
		{
			arm_grabbing_status.Add(true);
			arm_reset_queue_index.Add(false);
			handbone_array_boneName.Add("");
			handbone_array_boneIndex.Add(0);
			Frozen_Elbow_Component_Status.Add(false);

			should_override_hand_location.Add(false);
			should_override_hand_rotation.Add(false);
			arm_override_transform.Add(FTransform::Identity);

		}

	}


}

void UDragonIKWeaponPhysicsComponent::Server_FirstTimeInitialization_Implementation(USkeletalMeshComponent* skeleton_input)
{
	Multicast_FirstTimeInitialization(skeleton_input);
}

bool UDragonIKWeaponPhysicsComponent::Server_FirstTimeInitialization_Validate(USkeletalMeshComponent* skeleton_input)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_FirstTimeInitialization_Implementation(USkeletalMeshComponent* skeleton_input)
{
	FirstTimeInitialization(skeleton_input);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_FirstTimeInitialization_Validate(USkeletalMeshComponent* skeleton_input)
{
	return true;
}



void UDragonIKWeaponPhysicsComponent::SetWeaponPhysicsBlendWeight(float InWeight)
{
	// Clamp the blend weight to [0, 1]
	InWeight = FMath::Clamp<float>(InWeight, 0.f, 1.f);

	// Get the current transform and physics transform
	FTransform PhysicsTransform = active_weapon_component->GetBodyInstance()->GetUnrealWorldTransform();


	// Blend the transforms using the weight
	FTransform BlendedTransform = FTransform::Identity;
	BlendedTransform.SetTranslation(FMath::Lerp(last_saved_relative_wep_transform.GetTranslation(), PhysicsTransform.GetTranslation(), InWeight));
	BlendedTransform.SetRotation(FQuat::Slerp(last_saved_relative_wep_transform.GetRotation(), PhysicsTransform.GetRotation(), InWeight));
	BlendedTransform.SetScale3D(FMath::Lerp(last_saved_relative_wep_transform.GetScale3D(), PhysicsTransform.GetScale3D(), InWeight));





	// Set the component's transform to the blended transform
	active_weapon_component->SetRelativeTransform(BlendedTransform);
}



void UDragonIKWeaponPhysicsComponent::ModifyWeaponTransform()
{


	

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "dommy "+(dominant_hand_transform).ToString());
	//UE_ASSUME(active_weapon_component != nullptr);

	if (is_grabbing && weapon_physics_handle != nullptr && owning_skeleton != nullptr)
	{


		if(active_weapon_component != nullptr)
		{
			if((Weapon_Delta_Transform.GetLocation() - active_weapon_component->GetComponentTransform().GetLocation()).Length() > Teleport_Distance)
			{
				if(teleport_freeze_count == 0)
 				 teleport_freeze_count = 1;
			}
			else if(teleport_freeze_count >= 10)
			{

				teleport_freeze_count = 0;

			}


			if(teleport_freeze_count < 10 && teleport_freeze_count > 0)
			{

			//	DisableWeaponsArms(true);
			//	EnableWeaponsArms();

				active_weapon_component->SetPhysicsLinearVelocity(FVector::ZeroVector,false);
				active_weapon_component->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector,false);
				active_weapon_component->SetWorldTransform(Saved_Relative_Hand_Transform*owning_skeleton->GetSocketTransform(weapon_grabbed_bone,ERelativeTransformSpace::RTS_World),false,nullptr,ETeleportType::TeleportPhysics);

				teleport_freeze_count++;

			//	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "teleport = "+(Saved_Relative_Hand_Transform).ToString());

			//	GEngine->AddOnScreenDebugMessage(-1, 5.1f, FColor::Red, "teleport "+(Saved_Relative_Hand_Transform*owning_skeleton->GetSocketTransform(weapon_grabbed_bone,ERelativeTransformSpace::RTS_World)).ToString());

			}
		}
		

		Weapon_Delta_Transform = active_weapon_component->GetComponentTransform();


		//dominant_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));

		//dominant_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));


		//FTransform inversed_transform_input = last_saved_hand_transform.Inverse() * dominant_hand_transform;


		FTransform inversed_transform_input;

		if(this_should_override_weapon_transform)
		{
			dominant_hand_transform = overrided_weapon_transform;
		}

		if(Weapon_Freeze_Bool)
		{
			inversed_transform_input = Frozen_Hand_Transform;
			frozen_lerp_alpha = 0;
		}
		else
		{
			frozen_lerp_alpha += owning_skeleton->GetWorld()->DeltaTimeSeconds*unfreeze_speed;

			frozen_lerp_alpha = FMath::Clamp<float>(frozen_lerp_alpha,0,1);

			if(frozen_lerp_alpha == 1)
			{
				inversed_transform_input = dominant_hand_transform;
			}
			else
			{
				inversed_transform_input = UKismetMathLibrary::TLerp(Frozen_Hand_Transform,dominant_hand_transform,frozen_lerp_alpha);
			}
		}

		//FTransform inversed_transform_input = FTransform::Identity;
		//inversed_transform_input.AddToTranslation(FVector(0,0,250));

		//inversed_transform_input = FTransform::Identity;

		/*
		if (grab_origin_type == EOrigin_Grab_DragonIK::ENUM_Origin_Hand)
		{
			inversed_transform_input = dominant_hand_transform;
		}
		*/

	
		if (perform_physics_simulation == true)
			weapon_physics_handle->SetTargetLocationAndRotation(inversed_transform_input.GetLocation(), inversed_transform_input.Rotator());
		

		//UPhysicsHandleComponent* test;
		//test->GrabbedComponent

		

		//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, " last_saved_hand_transform : " + inversed_transform_input.GetLocation().ToString());


	}
	

}

void UDragonIKWeaponPhysicsComponent::DelayedGrabEvent()
{

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, "dominant_hand_transform "+dominant_hand_transform.ToString());

	if (owning_skeleton->GetBoneIndex(weapon_grabbed_bone) > -1)
	{

		//last_saved_hand_transform = active_weapon_component->GetComponentTransform();


		//weapon_physics_handle->GrabComponentAtLocationWithRotation(active_weapon_component, "", active_weapon_component->GetComponentLocation(), active_weapon_component->GetComponentRotation());

		//dominant_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));

		/*
		dominant_hand_transform = FTransform::Identity;
		dominant_hand_transform.AddToTranslation(FVector(0, 0, 0.1f));
		*/


		//if (physics_welded_weapon == true)
		//	dominant_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));


		Saved_Relative_Hand_Transform = active_weapon_component->GetComponentTransform();
		Saved_Relative_Hand_Transform = Saved_Relative_Hand_Transform* owning_skeleton->GetSocketTransform(weapon_grabbed_bone,ERelativeTransformSpace::RTS_World).Inverse();
		



		last_saved_hand_transform = dominant_hand_transform;

		//last_saved_weapon_transform =  active_weapon_component->GetComponentTransform()* dominant_hand_transform.Inverse();
		last_saved_weapon_transform = active_weapon_component->GetComponentTransform();

		

		//GEngine->AddOnScreenDebugMessage(-1, 10.1f, FColor::Yellow, " last_saved_hand_transform : " + last_saved_hand_transform.GetLocation().ToString());


		

		if (!last_saved_hand_transform.Equals(FTransform::Identity))
		{
			
			if (perform_physics_simulation)
			{
				active_weapon_component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				active_weapon_component->SetSimulatePhysics(true);
			}
			else
			{
				active_weapon_component->SetSimulatePhysics(false);
			}

			
			active_weapon_component->SetEnableGravity(perform_gravity_simulation);

			
			//weapon_physics_handle->GrabComponentAtLocationWithRotation(active_weapon_component, "", active_weapon_component->GetComponentLocation(), active_weapon_component->GetComponentRotation());

			//weapon_physics_handle->GrabComponentAtLocationWithRotation(active_weapon_component, "", last_saved_hand_transform.GetLocation(), last_saved_hand_transform.Rotator());

			//if (physics_welded_weapon == false)
			{

				
				{
					weapon_physics_handle->GrabComponentAtLocationWithRotation(active_weapon_component, weapon_bone, last_saved_hand_transform.GetLocation(), last_saved_hand_transform.Rotator());
					
				}

			}
			

			weapon_physics_constraint->SetWorldLocation(dominant_shoulder.GetLocation(),false,nullptr,ETeleportType::TeleportPhysics);
			
			//weapon_physics_constraint->SetWorldLocation(active_weapon_component->GetComponentTransform().GetLocation());
			weapon_physics_constraint->AttachToComponent(owning_skeleton, FAttachmentTransformRules::KeepWorldTransform, dominant_shoulder_name);


			//UMeshComponent* active_weapon_component;
			FVector const cached_active_vector = active_weapon_component->GetComponentLocation();
			//active_weapon_component->SetWorldLocation(dominant_shoulder.GetLocation()+ (active_weapon_component->GetComponentLocation() - dominant_shoulder.GetLocation()).GetUnsafeNormal()*100);
			

			//FTransform weapon_hand_offset =  (active_weapon_component->GetComponentTransform() *  owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone)).Inverse());

			FVector weapon_hand_offset = (active_weapon_component->GetComponentLocation() - dominant_hand_transform.GetLocation());

        //  COMMENTED FOR NOW
		//	active_weapon_component->SetWorldLocation( (dominant_shoulder.GetLocation() + weapon_hand_offset));
			active_weapon_component->SetWorldLocation( (dominant_shoulder.GetLocation()),false,nullptr,ETeleportType::TeleportPhysics);
			
			if (UDragonIK_Library::IsBonePhysicsEnabled(owning_skeleton, dominant_shoulder_name))
			{
				weapon_physics_constraint->SetConstrainedComponents(owning_skeleton, dominant_shoulder_name, active_weapon_component, "None");
			}
			else
			{

				FName shoulder_temp_bone = dominant_shoulder_name;
				for (int i = 0; i < 5; i++)
				{
					shoulder_temp_bone = owning_skeleton->GetParentBone(shoulder_temp_bone);


					if (UDragonIK_Library::IsBonePhysicsEnabled(owning_skeleton, shoulder_temp_bone))
					{
						weapon_physics_constraint->SetConstrainedComponents(owning_skeleton, shoulder_temp_bone, active_weapon_component, "None");

						//dominant_shoulder_name = shoulder_temp_bone;

						break;
					}

				}
			}


			physics_constraint_used = true;


			active_weapon_component->SetWorldLocation(cached_active_vector,false,nullptr,ETeleportType::TeleportPhysics);



			float weapon_shoulder_length = (dominant_shoulder.GetLocation() - active_weapon_component->GetComponentLocation()).Length();
			float shoulder_hand_current_length = (dominant_shoulder.GetLocation() - owning_skeleton->GetBoneLocation(dominant_hand_name)).Length();
			
		//	arm_length = UDragonIK_Library::GetBoneLengthBetweenBones(owning_skeleton, dominant_shoulder_name,weapon_grabbed_bone);
			arm_length = UDragonIK_Library::GetBoneLengthBetweenBones(owning_skeleton, dominant_shoulder_name,dominant_hand_name);
			float gap_distance = FMath::Abs(shoulder_hand_current_length - arm_length);
			arm_length = (weapon_shoulder_length+gap_distance) * Max_Arm_Length_Percent;
			
			weapon_physics_constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, arm_length);
			weapon_physics_constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, arm_length);
			weapon_physics_constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, arm_length);

			weapon_physics_constraint->SetLinearBreakable(false, 0.0f);


			

			//GEngine->AddOnScreenDebugMessage(-1, 10.1f, FColor::Red, "grabbed "+dominant_hand_transform.ToString());

			

			//GEngine->AddOnScreenDebugMessage(-1, 10.1f, FColor::Yellow, " last_saved_hand_transform : " + last_saved_hand_transform.GetLocation().ToString());

			is_grabbing = true;

			perform_grab_weapon = false;
		}
		else
		{



			//UMeshComponent* wep_cmp;
			//active_weapon_component->AttachToComponent(owning_skeleton,FAttachmentTransformRules::KeepWorldTransform,weapon_grabbed_bone);

			FTransform temp_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));

			last_saved_hand_transform = temp_hand_transform;

			//active_weapon_component->SetWorldLocation(temp_hand_transform.GetLocation());

		}

	}

}



void UDragonIKWeaponPhysicsComponent::Release_Hand_Only(FName hand_bone)
{

	int hand_array_index = -1;

	if (handbone_array_boneName.Num() > 0 && handbone_array_boneIndex.Num() > 0)
	{
		int string_to_local_index = 0;
		handbone_array_boneName.Find(hand_bone.ToString(), string_to_local_index);

		if (string_to_local_index > -1)
		hand_array_index = handbone_array_boneIndex[string_to_local_index];


		if (hand_array_index > -1)
		{

			if (arm_grabbing_status.Num() > hand_array_index)
				arm_grabbing_status[hand_array_index] = false;


			
			
		}

		//GEngine->AddOnScreenDebugMessage(-1, 5.1f, FColor::Red, " TESTX ");
	}
}

void UDragonIKWeaponPhysicsComponent::Server_Release_Hand_Only_Implementation(FName hand_bone)
{
	Multicast_Release_Hand_Only( hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Server_Release_Hand_Only_Validate(FName hand_bone)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Release_Hand_Only_Implementation(FName hand_bone)
{
	Release_Hand_Only( hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Release_Hand_Only_Validate(FName hand_bone)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Freeze_Elbows_Function(FName hand_bone)
{
	
	int hand_array_index = -1;

	if (handbone_array_boneName.Num() > 0 && handbone_array_boneIndex.Num() > 0)
	{
		int string_to_local_index = 0;
		handbone_array_boneName.Find(hand_bone.ToString(), string_to_local_index);

		if (string_to_local_index > -1)
			hand_array_index = handbone_array_boneIndex[string_to_local_index];

		if (hand_array_index > -1)
		{

			if (arm_grabbing_status.Num() > hand_array_index)
			{
				Frozen_Elbow_Component_Status[hand_array_index] = true;

				Weapon_Freeze_Bool = true;

				Frozen_Hand_Transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(weapon_grabbed_bone));
			}

			

		}
	}

	
	
}

void UDragonIKWeaponPhysicsComponent::Server_Freeze_Elbows_Function_Implementation(FName hand_bone)
{
	Multicast_Freeze_Elbows_Function( hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Server_Freeze_Elbows_Function_Validate(FName hand_bone)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Freeze_Elbows_Function_Implementation(FName hand_bone)
{
	Freeze_Elbows_Function( hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Freeze_Elbows_Function_Validate(FName hand_bone)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Unfreeze_Elbows_Function(FName hand_bone)
{


	int hand_array_index = -1;

	if (handbone_array_boneName.Num() > 0 && handbone_array_boneIndex.Num() > 0)
	{
		int string_to_local_index = 0;
		handbone_array_boneName.Find(hand_bone.ToString(), string_to_local_index);

		if (string_to_local_index > -1)
			hand_array_index = handbone_array_boneIndex[string_to_local_index];

		if (hand_array_index > -1)
		{

			if (arm_grabbing_status.Num() > hand_array_index)
			{
				Frozen_Elbow_Component_Status[hand_array_index] = false;

				Weapon_Freeze_Bool = false;

				//Frozen_Hand_Transform = dominant_hand_transform;
			}

			

		}
	}
	
}

void UDragonIKWeaponPhysicsComponent::Server_Unfreeze_Elbows_Function_Implementation(FName hand_bone)
{
	Multicast_Unfreeze_Elbows_Function(hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Server_Unfreeze_Elbows_Function_Validate(FName hand_bone)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Unfreeze_Elbows_Function_Implementation(FName hand_bone)
{
	Unfreeze_Elbows_Function(hand_bone);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Unfreeze_Elbows_Function_Validate(FName hand_bone)
{
	return true;
}



void UDragonIKWeaponPhysicsComponent::Attach_Hand_Only(FName hand_bone,bool arm_position_recalibrate)
{


	int hand_array_index = -1;

	if (handbone_array_boneName.Num() > 0 && handbone_array_boneIndex.Num() > 0)
	{
		int string_to_local_index = 0;
		handbone_array_boneName.Find(hand_bone.ToString(), string_to_local_index);

		if (string_to_local_index > -1)
		hand_array_index = handbone_array_boneIndex[string_to_local_index];

		if (hand_array_index > -1)
		{

			if (arm_grabbing_status.Num() > hand_array_index)
			{
				arm_grabbing_status[hand_array_index] = true;
			}

			arm_reset_queue_index[hand_array_index] = true;

		}
	}

}

void UDragonIKWeaponPhysicsComponent::Server_Attach_Hand_Only_Implementation(FName hand_bone,bool arm_position_recalibrate)
{
	Multicast_Attach_Hand_Only( hand_bone, arm_position_recalibrate);
	
}

bool UDragonIKWeaponPhysicsComponent::Server_Attach_Hand_Only_Validate(FName hand_bone, bool arm_position_recalibrate)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Attach_Hand_Only_Implementation(FName hand_bone,bool arm_position_recalibrate)
{
	Attach_Hand_Only( hand_bone, arm_position_recalibrate);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Attach_Hand_Only_Validate(FName hand_bone, bool arm_position_recalibrate)
{
	return true;
}



void UDragonIKWeaponPhysicsComponent::Release_Hand_Plus_Override(FName hand_bone, FTransform hand_transform, bool override_hand_location, bool override_hand_rotation)
{


	int hand_array_index = -1;

	if (handbone_array_boneName.Num() > 0 && handbone_array_boneIndex.Num() > 0)
	{
		
		int string_to_local_index = 0;
		handbone_array_boneName.Find(hand_bone.ToString(), string_to_local_index);

		if(string_to_local_index > -1)
		hand_array_index = handbone_array_boneIndex[string_to_local_index];


		if (hand_array_index > -1)
		{


			if (should_override_hand_location.Num() > hand_array_index)
			{
				should_override_hand_location[hand_array_index] = override_hand_location;
			}

			if (should_override_hand_rotation.Num() > hand_array_index)
			{
				should_override_hand_rotation[hand_array_index] = override_hand_rotation;
			}


			arm_override_transform[hand_array_index] = hand_transform;


		}

	}

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, " handbone NUM : " + FString::SanitizeFloat(handbone_array_map.Num()));


}

void UDragonIKWeaponPhysicsComponent::Server_Release_Hand_Plus_Override_Implementation(FName hand_bone,FTransform hand_transform, bool override_hand_location, bool override_hand_rotation)
{
	Multicast_Release_Hand_Plus_Override(hand_bone,hand_transform, override_hand_location, override_hand_rotation);
}

bool UDragonIKWeaponPhysicsComponent::Server_Release_Hand_Plus_Override_Validate(FName hand_bone,FTransform hand_transform, bool override_hand_location, bool override_hand_rotation)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Multicast_Release_Hand_Plus_Override_Implementation(FName hand_bone,FTransform hand_transform, bool override_hand_location, bool override_hand_rotation)
{
	Release_Hand_Plus_Override(hand_bone,hand_transform, override_hand_location, override_hand_rotation);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Release_Hand_Plus_Override_Validate(FName hand_bone,FTransform hand_transform, bool override_hand_location, bool override_hand_rotation)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Weapon_Transform_Override(FTransform Weapon_Transform, bool bShould_Override_Weapon_Transform)
{

	this_should_override_weapon_transform = bShould_Override_Weapon_Transform;

	overrided_weapon_transform = Weapon_Transform;
	
	
}

void UDragonIKWeaponPhysicsComponent::Server_Weapon_Transform_Override_Implementation(FTransform Weapon_Transform,bool bShould_Override_Weapon_Transform)
{
	Multicast_Weapon_Transform_Override(Weapon_Transform,bShould_Override_Weapon_Transform);
}

bool UDragonIKWeaponPhysicsComponent::Server_Weapon_Transform_Override_Validate(FTransform Weapon_Transform,
	bool bShould_Override_Weapon_Transform)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Weapon_Transform_Override_Implementation(FTransform Weapon_Transform,bool bShould_Override_Weapon_Transform)
{
	Weapon_Transform_Override(Weapon_Transform,bShould_Override_Weapon_Transform);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Weapon_Transform_Override_Validate(FTransform Weapon_Transform,bool bShould_Override_Weapon_Transform)
{
	return true;
}



void UDragonIKWeaponPhysicsComponent::DisableWeaponsArms(bool reset_weapon_transform=true)
{

	if(active_weapon_component)
	{
		active_weapon_component->SetSimulatePhysics(false);

		active_weapon_component->AttachToComponent(owning_skeleton,FAttachmentTransformRules::KeepWorldTransform,weapon_grabbed_bone);

		if(reset_weapon_transform)
			active_weapon_component->SetWorldTransform(Saved_Relative_Hand_Transform* dominant_hand_transform);

		for(int i=0; i < handbone_array_boneName.Num(); i++)
		{
			Release_Hand_Only(FName(handbone_array_boneName[i]));
		}
		
	}
	
}

void UDragonIKWeaponPhysicsComponent::Server_DisableWeaponsArms_Implementation(bool reset_weapon_transform)
{
	Multicast_DisableWeaponsArms(reset_weapon_transform);
}

bool UDragonIKWeaponPhysicsComponent::Server_DisableWeaponsArms_Validate(bool reset_weapon_transform)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_DisableWeaponsArms_Implementation(bool reset_weapon_transform)
{
	DisableWeaponsArms(reset_weapon_transform);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_DisableWeaponsArms_Validate(bool reset_weapon_transform)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::EnableWeaponsArms()
{

	if(active_weapon_component)
	{
		active_weapon_component->SetSimulatePhysics(true);

		active_weapon_component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);


		for(int i=0; i < handbone_array_boneName.Num(); i++)
		{
			Attach_Hand_Only(FName(handbone_array_boneName[i]),false);
		}
		
	}
	
}

void UDragonIKWeaponPhysicsComponent::Server_EnableWeaponsArms_Implementation()
{
	Multicast_EnableWeaponsArms();
}

bool UDragonIKWeaponPhysicsComponent::Server_EnableWeaponsArms_Validate()
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_EnableWeaponsArms_Implementation()
{
	EnableWeaponsArms();
}

bool UDragonIKWeaponPhysicsComponent::Multicast_EnableWeaponsArms_Validate()
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Release_Weapon(bool should_simulatephysics, bool use_gravity,bool bKeep_Attachment)
{

	

	is_grabbing = false;

	//weapon_grabbed_bone = "None";

	if (active_weapon_component != nullptr)
	{
		active_weapon_component->SetEnableGravity(use_gravity);
		active_weapon_component->SetSimulatePhysics(should_simulatephysics);
	}


	if(weapon_physics_constraint != nullptr)
	weapon_physics_constraint->BreakConstraint();

	//UPhysicsHandleComponent* weapon_physics_handle;

	if (weapon_physics_handle != nullptr)
	weapon_physics_handle->ReleaseComponent();



	if(bKeep_Attachment && active_weapon_component != nullptr)
	{

		active_weapon_component->AttachToComponent(owning_skeleton,FAttachmentTransformRules::KeepWorldTransform,weapon_grabbed_bone);
		
	}


	last_saved_hand_transform = FTransform::Identity;

	active_weapon_component = nullptr;

	perform_grab_weapon = false;

	dominant_hand_transform = FTransform::Identity;

	

	//physics_welded_weapon = false;

}

void UDragonIKWeaponPhysicsComponent::Server_Release_Weapon_Implementation(bool should_simulatephysics,bool use_gravity, bool bKeep_Attachment)
{
	Multicast_Release_Weapon(should_simulatephysics, use_gravity, bKeep_Attachment);
}

bool UDragonIKWeaponPhysicsComponent::Server_Release_Weapon_Validate(bool should_simulatephysics, bool use_gravity, bool bKeep_Attachment)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Release_Weapon_Implementation(bool should_simulatephysics,bool use_gravity, bool bKeep_Attachment)
{
	Release_Weapon(should_simulatephysics, use_gravity, bKeep_Attachment);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Release_Weapon_Validate(bool should_simulatephysics, bool use_gravity, bool bKeep_Attachment)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Here we list the variables we want to replicate
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent, weapon_grabbed_bone);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,perform_physics_simulation);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,perform_gravity_simulation);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,active_weapon_component);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,Frozen_Elbow_Component_Status);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,owning_skeleton);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,perform_grab_weapon);

//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,weapon_physics_handle);
//	DOREPLIFETIME(UDragonIKWeaponPhysicsComponent,weapon_physics_constraint);
	
	



	
}
void UDragonIKWeaponPhysicsComponent::Grab_Weapon(FName grabbed_bone,bool simulate_physics,bool use_gravity,FName WeaponBoneName)
{
	/*
	active_weapon_component = weapon_mesh_input;
	
	if(active_weapon_component != nullptr)
	{
		if(active_weapon_component->GetOwner() != nullptr)
			active_weapon_component->GetOwner()->Destroy();
	}*/
	active_weapon_component = weapon_input_component;
	
	if (owning_skeleton != nullptr)
	{

		if (active_weapon_component != nullptr && owning_skeleton->GetBoneIndex(grabbed_bone) > -1)
		//if (weapon_mesh_input != nullptr && owning_skeleton->GetBoneIndex(grabbed_bone) > -1)
		{

			weapon_grabbed_bone = grabbed_bone;

			//weapon_mesh_input->SetSimulatePhysics(true);
			weapon_bone = WeaponBoneName;

			//active_weapon_component = weapon_mesh_input;
			active_weapon_component->SetIsReplicated(true);
			

			Saved_Relative_Hand_Transform = active_weapon_component->GetComponentTransform();
		//	Saved_Relative_Hand_Transform = Saved_Relative_Hand_Transform * owning_skeleton->GetComponentTransform().Inverse();

			
			Saved_Relative_Hand_Transform = Saved_Relative_Hand_Transform* owning_skeleton->GetSocketTransform(weapon_grabbed_bone,ERelativeTransformSpace::RTS_World).Inverse();
		//	Saved_Relative_Hand_Transform = Saved_Relative_Hand_Transform* dominant_hand_transform.Inverse();
		//	Saved_Relative_Hand_Transform = Saved_Relative_Hand_Transform * owning_skeleton->GetComponentTransform().Inverse();

		//	GEngine->AddOnScreenDebugMessage(-1, 5.1f, FColor::Red, "dominant_hand_transform "+dominant_hand_transform.ToString());
			
			
			//last_saved_hand_transform = owning_skeleton->GetBoneTransform(owning_skeleton->GetBoneIndex(grabbed_bone));

			last_saved_hand_transform = dominant_hand_transform;
			
			//last_saved_hand_transform = FTransform::Identity;

			perform_grab_weapon = true;

			grab_counter = 0;

			Wep_Offset_Calculated = false;

			perform_physics_simulation = simulate_physics;
			perform_gravity_simulation = use_gravity;

			//GEngine->AddOnScreenDebugMessage(-1, 5.1f, FColor::Red, "grab start");
			//physics_welded_weapon = is_physics_welded_weapon;

			//if(perform_physics_simulation)
			//active_weapon_component->AttachToComponent(owning_skeleton, FAttachmentTransformRules::FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false), weapon_grabbed_bone);

			if (perform_physics_simulation == false)
			{

				//USkeletalMeshComponent* test;
				
				//UPrimitiveComponent* test;

				

				active_weapon_component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				//owning_skel->
			}
			else
			{
				owning_skeleton->SetTickGroup(ETickingGroup::TG_PostPhysics);
			}
			active_weapon_component->SetEnableGravity(use_gravity);
			last_saved_relative_wep_transform = active_weapon_component->GetRelativeTransform();

		}

	}


}

void UDragonIKWeaponPhysicsComponent::Destroy_Weapon(AActor* weapon)
{

	//GEngine->AddOnScreenDebugMessage(-1, 10.1f, FColor::Yellow, " DESTROY TEST!");

	//UKismetSystemLibrary::PrintText(owning_skeleton->GetOwner()->GetWorld(),FText::AsCultureInvariant(" DESTROY TEST!"),true,false,FLinearColor::Yellow,10.1f);
	
	if(active_weapon_component != nullptr)
	{
		active_weapon_component->GetOwner()->Destroy();
		UKismetSystemLibrary::PrintText(owning_skeleton->GetOwner()->GetWorld(),FText::AsCultureInvariant(" DESTROY TEST!"),true,false,FLinearColor::Yellow,10.1f);
	
	}
}

void UDragonIKWeaponPhysicsComponent::Server_Destroy_Weapon_Implementation(AActor* weapon)
{
	Multicast_Destroy_Weapon(weapon);
}

bool UDragonIKWeaponPhysicsComponent::Server_Destroy_Weapon_Validate(AActor* weapon)
{
	return true;
}


void UDragonIKWeaponPhysicsComponent::Multicast_Destroy_Weapon_Implementation(AActor* weapon)
{
	//Destroy_Weapon(weapon);
	UKismetSystemLibrary::PrintText(owning_skeleton->GetOwner()->GetWorld(),FText::AsCultureInvariant(" MULTI TEST!"),true,false,FLinearColor::Yellow,0.1f);

}

bool UDragonIKWeaponPhysicsComponent::Multicast_Destroy_Weapon_Validate(AActor* weapon)
{
	return true;
}




void UDragonIKWeaponPhysicsComponent::Server_Grab_Weapon_Implementation(FName grabbed_bone, bool simulate_physics, bool use_gravity,FName WeaponBoneName)
{
	Multicast_Grab_Weapon(grabbed_bone,simulate_physics,use_gravity,WeaponBoneName);
}

bool UDragonIKWeaponPhysicsComponent::Server_Grab_Weapon_Validate(FName grabbed_bone, bool simulate_physics, bool use_gravity,FName WeaponBoneName)
{
	return true;
}

void UDragonIKWeaponPhysicsComponent::Multicast_Grab_Weapon_Implementation(FName grabbed_bone, bool simulate_physics, bool use_gravity,FName WeaponBoneName)
{
	Grab_Weapon(grabbed_bone,simulate_physics,use_gravity,WeaponBoneName);
}

bool UDragonIKWeaponPhysicsComponent::Multicast_Grab_Weapon_Validate(FName grabbed_bone, bool simulate_physics, bool use_gravity,FName WeaponBoneName)
{
	return true;
}







// Copyright Epic Games, Inc. All Rights Reserved.


#include "PhotoPawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/LocalPlayer.h"
#include "PhotoAlbumSubsystem.h"

// album path and photo prefix
// TODO: make these configurable
static FString AlbumPath = "Photos";
static FString PhotoPrefix = "Photo";

APhotoPawn::APhotoPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	USceneComponent* PawnRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	check(PawnRoot);

	SetRootComponent(PawnRoot);

	// create the camera collision sphere
	CameraSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Camera Sphere"));
	check(CameraSphere);

	CameraSphere->SetupAttachment(RootComponent);

	// create the spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	check(SpringArm);

	SpringArm->SetupAttachment(RootComponent);

	// create the camera
	PhotoCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Photo Camera"));
	check(PhotoCamera);

	PhotoCamera->SetupAttachment(SpringArm);

	// create the scene capture comp
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture"));
	check(SceneCapture);

	SceneCapture->SetupAttachment(PhotoCamera);
}

void APhotoPawn::BeginPlay()
{
	Super::BeginPlay();

}

void APhotoPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// get the player controller
	PC = Cast<APlayerController>(NewController);

	// set free mode by default
	SetupFreeMode(true);

	// build the HUD
	BuildHUD();
}

void APhotoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// tick the camera
	TickCameraMovement(DeltaTime);
}

void APhotoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhotoPawn::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APhotoPawn::MoveCompleted);

		// Look
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhotoPawn::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &APhotoPawn::LookCompleted);
	}
}

void APhotoPawn::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// set up the input vector. We flip the axis so they correspond with the expected Mover input
	CachedMoveInput.X = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
	CachedMoveInput.Y = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
}

void APhotoPawn::MoveCompleted(const FInputActionValue& Value)
{
	CachedMoveInput = FVector::ZeroVector;
}

void APhotoPawn::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// set up the look input rotator
	CachedLookInput.Yaw = FMath::Clamp(LookAxisVector.X, -1.0f, 1.0f);
	CachedLookInput.Pitch = FMath::Clamp(LookAxisVector.Y, -1.0f, 1.0f);
}

void APhotoPawn::LookCompleted(const FInputActionValue& Value)
{
	CachedLookInput = FRotator::ZeroRotator;
}

FVector APhotoPawn::ConstrainLocationToLimits(const FVector& Loc)
{
	// get the camera distance
	float CameraDist = Loc.Size();

	// are we out of bounds?
	if (CameraDist > MaxCameraDistance || CameraDist < MinCameraDistance)
	{
		// clamp to bounds and return
		CameraDist = FMath::Clamp(CameraDist, MinCameraDistance, MaxCameraDistance);

		return Loc.GetSafeNormal() * CameraDist;
	}

	// the original location is valid
	return Loc;
}

void APhotoPawn::SetInitialLocationAndRotation(const FVector& Loc, const FRotator& Rot)
{
	InitialLocation = Loc;
	InitialRotation = Rot;
}

void APhotoPawn::ToggleMovementMode()
{
	bFreeMovement = !bFreeMovement;

	if (bFreeMovement)
	{
		SetupFreeMode();
	}
	else {
		SetupOrbitMode();
	}
}

void APhotoPawn::SetupFreeMode(bool bInitial)
{
	// attach the camera to the collision sphere
	PhotoCamera->AttachToComponent(CameraSphere, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if (bInitial)
	{
		CameraSphere->SetWorldLocation(InitialLocation);

		// constrain the initial location to our bounds
		FVector CorrectedLocation = ConstrainLocationToLimits(CameraSphere->GetRelativeLocation());

		// move the collision sphere to the initial location/rotation
		CameraSphere->SetRelativeLocation(CorrectedLocation);
		CameraSphere->SetWorldRotation(InitialRotation);

	}
	else {

		// get the world location and rotation of the spring arm socket
		FVector SocketLoc;
		FQuat SocketRot;

		SpringArm->GetSocketWorldLocationAndRotation(USpringArmComponent::SocketName, SocketLoc, SocketRot);

		// move the collision sphere to the spring arm location
		CameraSphere->SetWorldLocation(SocketLoc);
		CameraSphere->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(GetActorLocation() - SocketLoc));

	}

	// reset the camera velocity
	CameraVelocity = FVector::ZeroVector;
}

void APhotoPawn::SetupOrbitMode(bool bInitial)
{
	// attach the camera to the spring arm
	PhotoCamera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	const FVector CameraRelativeLocation = CameraSphere->GetComponentLocation() - GetActorLocation();
	float CameraDistance = CameraRelativeLocation.Size();

	if (bInitial)
	{
		CameraDistance = DefaultCameraDistance;

		// initialize the rotation
		SpringArm->SetWorldRotation(InitialRotation);
	}

	const float ArmLength = FMath::Clamp(CameraDistance, MinCameraDistance, MaxCameraDistance);

	// set the target arm length
	SpringArm->TargetArmLength = ArmLength;

	// reset the arm length movement
	ArmLengthSpeed = 0.0f;
}

void APhotoPawn::TickCameraMovement(float DeltaTime)
{
	if (PC)
	{
		float RotMultiplier = bFreeMovement ? FreeMoveRotationMultiplier : 1.0f;

		PC->AddYawInput(CachedLookInput.Yaw * CameraRotationRateYaw * DeltaTime * RotMultiplier);
		PC->AddPitchInput(-CachedLookInput.Pitch * CameraRotationRatePitch * DeltaTime * RotMultiplier);
	}

	if (bFreeMovement)
	{
		TickFreeMode(DeltaTime);
	}
	else {
		TickOrbitMode(DeltaTime);
	}
}

void APhotoPawn::TickOrbitMode(float DeltaTime)
{
	// adjust the arm length using linear steering behavior
	float DesiredSpeed = -CachedMoveInput.X * CameraMaxSpeed;
	float Steering = DesiredSpeed - ArmLengthSpeed;

	Steering = FMath::Min(FMath::Abs(Steering), CameraAcceleration * DeltaTime) * FMath::Sign(Steering);

	ArmLengthSpeed += Steering;

	float ArmLength = SpringArm->TargetArmLength + ArmLengthSpeed * DeltaTime;
	ArmLength = FMath::Clamp(ArmLength, MinCameraDistance, MaxCameraDistance);

	SpringArm->TargetArmLength = ArmLength;
}

void APhotoPawn::TickFreeMode(float DeltaTime)
{
	// set the camera rotation
	CameraSphere->SetWorldRotation(GetControlRotation());

	// calculate the velocity using a simple steering behavior
	FVector Desired = GetControlRotation().RotateVector(FVector(CachedMoveInput.X, CachedMoveInput.Y, CachedMoveInput.Z)) * CameraMaxSpeed;
	FVector Steering = Desired - CameraVelocity;
	Steering = Steering.GetSafeNormal() * FMath::Min(Steering.Size(), CameraAcceleration * DeltaTime);
	CameraVelocity += Steering;

	// compute the move delta for this update
	FVector MoveDelta = CameraVelocity * DeltaTime;

	// move the camera collision sphere
	EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags;
	FHitResult Hit;

	CameraSphere->MoveComponent(MoveDelta, CameraSphere->GetComponentQuat(), true, &Hit, MoveFlags, ETeleportType::None);

	// if we hit something, try to slide along with it
	if (Hit.IsValidBlockingHit())
	{
		const FPlane MovementPlane(FVector::ZeroVector, Hit.ImpactNormal);

		FVector ConstrainedResult = FVector::PointPlaneProject(MoveDelta, MovementPlane);
		MoveDelta = ConstrainedResult.GetSafeNormal() * MoveDelta.Size() * (1.0f - Hit.Time);

		CameraSphere->MoveComponent(MoveDelta, CameraSphere->GetComponentQuat(), true, &Hit, MoveFlags, ETeleportType::None);
	}

	// apply min/max distance to the camera
	FVector CameraLocation = ConstrainLocationToLimits(CameraSphere->GetRelativeLocation());
	CameraSphere->SetRelativeLocation(CameraLocation);
}

void APhotoPawn::SetFOV(float FOVAngle)
{
	PhotoCamera->SetFieldOfView(FMath::Clamp(FOVAngle, MinCameraFOV, MaxCameraFOV));
}

void APhotoPawn::SetFOVByRatio(float FOVRatio)
{
	PhotoCamera->SetFieldOfView(FMath::Lerp(MinCameraFOV, MaxCameraFOV, FOVRatio));
}

void APhotoPawn::SetRoll(float Roll)
{
	CurrentCameraRoll = FMath::Clamp(Roll, -MaxCameraRoll, MaxCameraRoll);
}

void APhotoPawn::SetRollByRatio(float RollRatio)
{
	CurrentCameraRoll = FMath::Lerp(-MaxCameraRoll, MaxCameraRoll, RollRatio);
}

bool APhotoPawn::TakePhoto()
{
	// configure the scene capture component
	SceneCapture->FOVAngle = PhotoCamera->FieldOfView;

	// capture the scene
	SceneCapture->CaptureScene();

	if (PC)
	{
		// get the album subsystem
		UPhotoAlbumSubsystem* AlbumSubsystem = ULocalPlayer::GetSubsystem<UPhotoAlbumSubsystem>(PC->GetLocalPlayer());

		if (AlbumSubsystem)
		{
			// try to save the photo to a file
			FString PhotoFilename;

			if (AlbumSubsystem->SavePhotoFromRenderTarget(SceneCapture->TextureTarget, AlbumPath, PhotoPrefix, PhotoFilename))
			{
				// play the photo snap effect
				PlayPhotoSnap();

				// success!
				return true;
			}

		}
	}

	return false;
}


// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhotoPawn.generated.h"

class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class USceneCaptureComponent2D;
class APlayerController;

struct FInputActionValue;

/**
 *  APhotoPawn
 *  Pawn that allows the player to take in-game photos.
 *  Supports two camera modes: free-movement and orbit
 */
UCLASS(notplaceable)
class PHOTOALBUM_API APhotoPawn : public APawn
{
	GENERATED_BODY()

public:

	// Constructor
	APhotoPawn();

/////////////////////////////////////
	// Components
private:

	/** Collision sphere surrounding the camera in free movement mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CameraSphere;

	/** Spring arm for orbit mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Capture camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> PhotoCamera;

	/** Scene capture comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

public:

	/** public component getters */
	FORCEINLINE USphereComponent* GetCameraSphere() const { return CameraSphere; }
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return PhotoCamera; }
	FORCEINLINE USceneCaptureComponent2D* GetSceneCapture() const { return SceneCapture; }

protected:

	/** BeginPlay initialization */
	virtual void BeginPlay() override;

	/** PossessedBy initialization */
	virtual void PossessedBy(AController* NewController) override;

public:	

	/** Tick */
	virtual void Tick(float DeltaTime) override;

	/////////////////////////////////////
	// Input
protected:

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr <UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr <UInputAction> LookAction;

public:

	/** Sets up player input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void LookCompleted(const FInputActionValue& Value);

protected:

	/* cached move variables */
	FVector CachedMoveInput = FVector::ZeroVector;
	FRotator CachedLookInput = FRotator::ZeroRotator;

	/////////////////////////////////////
	// Movement
	
protected:

	TObjectPtr<APlayerController> PC;

	bool bFreeMovement = true;

	/** Default camera distance in orbit mode */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float DefaultCameraDistance = 600.0f;

	/** How close the camera can get to the pawn */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float MinCameraDistance = 200.0f;

	/** How far the camera can get from the pawn */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float MaxCameraDistance = 2000.0f;

	/** Max movement speed for the camera */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float CameraMaxSpeed = 800.0f;

	/** Camera movement acceleration */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float CameraAcceleration = 4000.0f;

	/** Camera turn rate */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float CameraRotationRateYaw = 200.0f;

	/** Camera up/down rate */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float CameraRotationRatePitch = 200.0f;

	/** Camera rotation multiplier for Free Mode. Allows for more accuracy when not orbiting. */
	UPROPERTY(Category = "Photo Pawn|Movement", EditAnywhere, meta = (ClampMin=0))
	float FreeMoveRotationMultiplier = 0.15;

	/** Speed at which the arm length is being adjusted */
	float ArmLengthSpeed = 0.0f;

	/** Last calculated camera velocity */
	FVector CameraVelocity = FVector::ZeroVector;

	/** Initial location and rotation for the camera */
	FVector InitialLocation;
	FRotator InitialRotation;

protected:

	/** */
	FVector ConstrainLocationToLimits(const FVector& Loc);

public:

	/** Sets the initial rotation for the camera */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void SetInitialLocationAndRotation(const FVector& Loc, const FRotator& Rot);

	/** Toggles between free mode and orbit mode */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void ToggleMovementMode();

	/** Returns true if the camera is in free mode, false if it's in orbit mode */
	UFUNCTION(BlueprintPure, Category="Photo Pawn")
	bool IsFreeMoving() const { return bFreeMovement; };

	/** Sets up the free mode camera */
	void SetupFreeMode(bool bInitial = false);

	/** Sets up the orbit mode camera */
	void SetupOrbitMode(bool bInitial = false);

	/** Entry point for camera update */
	void TickCameraMovement(float DeltaTime);

	/** Updates the orbit mode camera */
	void TickOrbitMode(float DeltaTime);

	/** Updates the free mode camera */
	void TickFreeMode(float DeltaTime);

	/////////////////////////////////////
	// Camera Interface
protected:

	/** Minimum allowed camera FOV half angle */
	UPROPERTY(Category = "Photo Pawn|Camera", EditAnywhere, meta = (ClampMin=10, ClampMax=180))
	float MinCameraFOV = 60.0f;

	/** Maximum allowed camera FOV half angle */
	UPROPERTY(Category = "Photo Pawn|Camera", EditAnywhere, meta = (ClampMin=10, ClampMax=180))
	float MaxCameraFOV = 120.0f;

	/** Maximum allowed camera roll. Minimum roll = -Max roll*/
	UPROPERTY(Category = "Photo Pawn|Camera", EditAnywhere, meta = (ClampMin=0, ClampMax=180))
	float MaxCameraRoll = 90.0f;

	/** Minimum allowed camera focus distance. */
	UPROPERTY(Category = "Photo Pawn|Camera", EditAnywhere, meta = (ClampMin=0, ClampMax=5000))
	float MinCameraFocusDistance = 0.0f;

	/** Maximum allowed camera focus distance. */
	UPROPERTY(Category = "Photo Pawn|Camera", EditAnywhere, meta = (ClampMin=0, ClampMax=5000))
	float MaxCameraFocusDistance = 1000.0f;

	/** Current camera roll rotation value */
	float CurrentCameraRoll = 0.0f;

public:

	/** Sets the camera FOV half angle */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void SetFOV(float FOVAngle);

	/** Sets the camera FOV half angle as a ratio of min/max */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void SetFOVByRatio(float FOVRatio);

	/** Sets the camera roll rotation */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void SetRoll(float Roll);

	/** Sets the camera roll rotation as a ratio of min/max */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	void SetRollByRatio(float RollRatio);

	/** Takes a photo from the current camera perspective */
	UFUNCTION(BlueprintCallable, Category="Photo Pawn")
	bool TakePhoto();

	/////////////////////////////////////
	// HUD Interface
protected:

	/** BP implementable event to build the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Photo Pawn")
	void BuildHUD();

	/** BP implementable event to play the photo snap effect */
	UFUNCTION(BlueprintImplementableEvent, Category="Photo Pawn")
	void PlayPhotoSnap();
};

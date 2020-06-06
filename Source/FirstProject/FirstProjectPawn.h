// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundCue.h"
#include "FirstProjectPawn.generated.h"


UCLASS(Config=Game)
class AFirstProjectPawn : public APawn
{
	GENERATED_BODY()

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* PlaneMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
public:
	AFirstProjectPawn();

	// Begin AActor overrides
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int CurrentHealth;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int Score;

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	FVector GunOffset;

	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int MGunAmmo;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MGunCone;

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	/*RR flap pitch*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RRUp;

	/*RR flap pitch*/
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RLUp;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RollIn;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float PitchIn;



	/** Current forward speed */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float CurrentForwardSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float CurrentAcceleration;

protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides

	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	void YawRightInput(float Val);

	void MGunInput();

	void MGunOutput();

	void MGunFire();

	void CameraRightInput(float Val);

	void CameraUpInput(float Val);

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	float CurrentPitchSpeed;

	/** Current roll speed */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	float CurrentRollSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	USoundCue* fireAudioCue;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	UAudioComponent* fireAudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	USoundCue* turbineAudioCue;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	USoundCue* turbineStartupCue;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	UAudioComponent* turbineAudioComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	USoundCue* ammoZeroAudioCue;

private:

	/** How quickly forward speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
	float Acceleration;

	/** How quickly pawn can steer */
	UPROPERTY(Category = Plane, EditAnywhere)
	float TurnSpeed;

	UPROPERTY(Category = Plane, EditAnywhere)
	float YawSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Plane, EditAnywhere)
	float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Plane, EditAnywhere)
	float MinSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Plane, EditAnywhere)
	float MaxAcceleration;

	/** Min forward speed */
	UPROPERTY(Category = Plane, EditAnywhere)
	float MinAcceleration;

	

	float CurrentCameraRight;

	float CurrentCameraUp;

	bool firing;

	UWorld* const World = GetWorld();

	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

public:
	/** Returns PlaneMesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};

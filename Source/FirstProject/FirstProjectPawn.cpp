// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstProjectPawn.h"
#include "MGunBullet.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AFirstProjectPawn::AFirstProjectPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			//: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
			: PlaneMesh(TEXT("/Game/new-f22-raptor/source/F22_Raptor.F22_Raptor"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(-140.f,0.f,80.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 40.f;
	SpringArm->CameraLagMaxDistance = 50.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.5f;
	SpringArm->SetRelativeLocation(FVector(20.f, 0.f, 0.f));

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	Camera->FieldOfView = 100.f;
	Camera->SetRelativeLocation(FVector(40.f, 0.f, -45.f));
	Camera->SetRelativeRotation(FRotator(6.f, 0.f, 0.f));

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	MaxSpeed = 10000.f;
	MinSpeed = 1000.f;
	CurrentForwardSpeed = 1000.f;
	YawSpeed = 10.f;
	CurrentHealth = 100.f;
	CurrentCameraRight = 0.f;
	CurrentCameraUp = 0.f;

	// Weapon
	GunOffset = FVector(600.f, 0.f, -20.f);
	FireRate = 0.0017f;
	MGunCone = 0.5;
	bCanFire = true;
	MGunAmmo = 480;

	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Effects/P226-9mm-Close.P226-9mm-Close"));
	FireSound = FireAudio.Object;
}

void AFirstProjectPawn::Tick(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0,0,0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);

	SpringArm->SetRelativeRotation(FRotator(CurrentCameraUp, CurrentCameraRight, 0.f));
}

void AFirstProjectPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
	CurrentHealth -= 10;
}


void AFirstProjectPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &AFirstProjectPawn::ThrustInput);
	PlayerInputComponent->BindAxis("MoveUp", this, &AFirstProjectPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFirstProjectPawn::MoveRightInput);
	PlayerInputComponent->BindAxis("Yaw", this, &AFirstProjectPawn::YawRightInput);
	PlayerInputComponent->BindAxis("MGun", this, &AFirstProjectPawn::MGunInput);
	PlayerInputComponent->BindAxis("CameraRight", this, &AFirstProjectPawn::CameraRightInput);
	PlayerInputComponent->BindAxis("CameraUp", this, &AFirstProjectPawn::CameraUpInput);
}

void AFirstProjectPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void AFirstProjectPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AFirstProjectPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetRollSpeed = (2 * Val * TurnSpeed);


	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AFirstProjectPawn::YawRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetYawSpeed = (2 * Val * YawSpeed);


	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// Smoothly interpolate roll speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AFirstProjectPawn::CameraRightInput(float Val)
{
	CurrentCameraRight = 180.f * Val;
}

void AFirstProjectPawn::CameraUpInput(float Val)
{
	CurrentCameraUp = -90.f * Val;
}

void AFirstProjectPawn::MGunInput(float Val)
{
	// If it's ok to fire again
	if (bCanFire == true && MGunAmmo > 0)
	{
		// If we are pressing fire stick in a direction
		if (Val > 0.0f)
		{
			const FRotator FireRotation = GetActorRotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<AMGunBullet>(SpawnLocation, FireRotation + FRotator(((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone));
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AFirstProjectPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}
			MGunAmmo--;

			bCanFire = false;
		}
	}
}

void AFirstProjectPawn::ShotTimerExpired()
{
	bCanFire = true;
}
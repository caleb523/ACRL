// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstProjectPawn.h"
#include "MGunBullet.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

AFirstProjectPawn::AFirstProjectPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Models/F22_Rigged/F22_Rigged_Scaled.F22_Rigged_Scaled"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetSkeletalMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	PlaneMesh->SetCollisionProfileName("Pawn");
	RootComponent = PlaneMesh;
	

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 1900.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,0.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 40.f;
	SpringArm->CameraLagMaxDistance = 50.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 15.f;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 305.f));

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	Camera->FieldOfView = 90.f;
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Camera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule0"));
	Capsule->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Capsule->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Capsule->SetRelativeScale3D(FVector(4.75f, 3.75f, 19.5f));
	Capsule->SetSimulatePhysics(true);
	Capsule->SetEnableGravity(false);
	Capsule->SetNotifyRigidBodyCollision(true);

	Capsule->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	

	// Set handling parameters
	Acceleration = 15000.f;
	CurrentAcceleration = 0.f;
	MinAcceleration = -7500.f;
	MaxAcceleration = 10000.f;
	TurnSpeed = 50.f;
	MaxSpeed = 67056.f;
	MinSpeed = 7200.f;
	CurrentForwardSpeed = 10000.f;
	YawSpeed = 10.f;
	CurrentHealth = 100.f;
	CurrentCameraRight = 0.f;
	CurrentCameraUp = 0.f;
	Score = 0;

	// Weapon
	GunOffset = FVector(70.f, 160.f, 45.f);
	FireRate = 0.004f;
	MGunCone = 0.2f;
	bCanFire = true;
	MGunAmmo = 480;
	firing = false;

	RROffset = FRotator(90.f, 0.f, -90.f);
	RLOffset = FRotator(90.f, 0.f, 90.f);
	RRUp1 = 0.f;
	RRUp2 = 0.f;
	RLUp1 = 0.f;
	RLUp2 = 0.f;

	// Load our Sound Cue for the turbine sound we created in the editor... note your path may be different depending
	// on where you store the asset on disk.
	static ConstructorHelpers::FObjectFinder<USoundCue> turbineCue(TEXT("'/Game/Audio/Fighter_Turbine_Steady_02_Cue.Fighter_Turbine_Steady_02_Cue'"));
	static ConstructorHelpers::FObjectFinder<USoundCue> fireCue(TEXT("'/Game/Audio/Effects/WPN_GUN_BRRT_Cue.WPN_GUN_BRRT_Cue'"));
	static ConstructorHelpers::FObjectFinder<USoundCue> ammoZeroCue(TEXT("'/Game/Audio/Effects/HUD_VOICE_AMMUNITION_ZERO_Cue.HUD_VOICE_AMMUNITION_ZERO_Cue'"));

	// Store a reference to the Cue asset - we'll need it later.
	turbineAudioCue = turbineCue.Object;
	fireAudioCue = fireCue.Object;
	ammoZeroAudioCue = ammoZeroCue.Object;
	// Create an audio component, the audio component wraps the Cue, and allows us to ineract with
	// it, and its parameters from code.
	turbineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TurbineAudioComp"));
	fireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("fireAudioComp"));
	// I don't want the sound playing the moment it's created.
	fireAudioComponent->bAutoActivate = false;
	turbineAudioComponent->bAutoActivate = false; // don't play the sound immediately.
	// I want the sound to follow the pawn around, so I attach it to the Pawns root.
	//turbineAudioComponent->AttachParent = RootComponent;
	turbineAudioComponent->SetRelativeLocation(FVector(-80.0f, 0.0f, 0.0f));
	fireAudioComponent->SetRelativeLocation(GunOffset);
}

void AFirstProjectPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (turbineAudioCue->IsValidLowLevelFast()) {
		turbineAudioComponent->SetSound(turbineAudioCue);
	}
	if (fireAudioCue->IsValidLowLevelFast()) {
		fireAudioComponent->SetSound(fireAudioCue);
	}
}

void AFirstProjectPawn::BeginPlay()
{
	Capsule->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Super::BeginPlay();
	Capsule->OnComponentHit.AddDynamic(this, &AFirstProjectPawn::OnCompHit);

	float startTime = 9.f;
	float volume = 1.0f;
	float fadeTime = 1.0f;
	turbineAudioComponent->FadeIn(fadeTime, volume, startTime);
}
void AFirstProjectPawn::Tick(float DeltaSeconds)
{
	Capsule->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	PlaneMesh->SetBoneRotationByName(FName("RR"), RROffset + FRotator(CurrentPitchSpeed / -5.f + CurrentRollSpeed / -10.f, 0.f, 0.f), EBoneSpaces::ComponentSpace);
	PlaneMesh->SetBoneRotationByName(FName("RL"), RLOffset + FRotator(CurrentPitchSpeed / -5.f + CurrentRollSpeed / 10.f, 0.f, 0.f), EBoneSpaces::ComponentSpace);
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcceleration);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
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

	
	//Turbine noise pitch is determined by a combination of the relative speed and acceleration with acceleration having preference
	float turbineRpm = (((CurrentAcceleration - MinAcceleration) / (MaxAcceleration - MinAcceleration)) * 0.75f + 0.25f * ((CurrentForwardSpeed - MinSpeed) / (MaxSpeed - MinSpeed))) * 1.25f + 0.75f;
	turbineAudioComponent->SetPitchMultiplier(turbineRpm);

	SpringArm->SetRelativeRotation(FRotator(CurrentCameraUp, CurrentCameraRight, 0.f));

	if (firing)
	{
		if (bCanFire == true)
		{
			if (MGunAmmo > 0)
			{
				MGunFire();
			}
			else
			{
				fireAudioComponent->Deactivate();
				firing = false;
			}
		}
	}
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void AFirstProjectPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
	CurrentHealth -= 10;
}

void AFirstProjectPawn::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), NormalImpulse.ToOrientationQuat(), 0.025f));
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
	PlayerInputComponent->BindAction("MGun", IE_Pressed, this, &AFirstProjectPawn::MGunInput);
	PlayerInputComponent->BindAction("MGun", IE_Released, this, &AFirstProjectPawn::MGunOutput);
	PlayerInputComponent->BindAxis("CameraRight", this, &AFirstProjectPawn::CameraRightInput);
	PlayerInputComponent->BindAxis("CameraUp", this, &AFirstProjectPawn::CameraUpInput);
}

void AFirstProjectPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.4f * CurrentAcceleration);
	// Calculate new speed
	float NewAccelerationSpeed = CurrentAcceleration + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentAcceleration = FMath::Clamp(NewAccelerationSpeed, MinAcceleration, MaxAcceleration);
}

void AFirstProjectPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
	RRUp1 = Val * -7.5f;
	RLUp1 = Val * -7.5f;
}

void AFirstProjectPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetRollSpeed = (2 * Val * TurnSpeed);

	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
	RRUp2 = Val * -2.5f;
	RLUp2 = Val * 2.5f;
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

void AFirstProjectPawn::MGunInput()
{
	if (MGunAmmo > 0)
	{
		firing = true;
		fireAudioComponent->Activate();
	}
	else
	{
		UGameplayStatics::PlaySound2D(World, ammoZeroAudioCue);
	}
}

void AFirstProjectPawn::MGunOutput()
{
	firing = false;
	fireAudioComponent->Deactivate();
}

void AFirstProjectPawn::MGunFire()
{
	MGunAmmo--;
	bCanFire = false;
	const FRotator FireRotation = GetActorRotation();
	// Spawn projectile at an offset from this pawn
	const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

	if (World != NULL)
	{
		// spawn the projectile
		AMGunBullet* bullet = World->SpawnActorDeferred<AMGunBullet>(AMGunBullet::StaticClass(), FTransform(FireRotation + FRotator(((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone), SpawnLocation), GetOwner(), GetOwner()->GetInstigator());
		bullet->SetVelocity(CurrentForwardSpeed);
		UGameplayStatics::FinishSpawningActor(bullet, FTransform(FireRotation + FRotator(((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone, ((float)rand()) / RAND_MAX * 2.0 * MGunCone - MGunCone), SpawnLocation));
	}
	else
	{
		printf("World == null!");
	}
	World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AFirstProjectPawn::ShotTimerExpired, FireRate);
}

void AFirstProjectPawn::ShotTimerExpired()
{
	bCanFire = true;
}
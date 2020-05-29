// Fill out your copyright notice in the Description page of Project Settings.


#include "MGunBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"

// Sets default values
AMGunBullet::AMGunBullet()
{
	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/Effects/Sphere.Sphere"));

	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetWorldScale3D(FVector(1.f, 0.3f, 0.3f));
	ProjectileMesh->SetCollisionProfileName("IgnoreOnlyPawn");
	ProjectileMesh->CastShadow = false;
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AMGunBullet::OnHit);		// set up a notification for when this component hits something
	RootComponent = ProjectileMesh;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = 103000.f;
	ProjectileMovement->MaxSpeed = 103000.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.f; // Normal gravity

	// Die after 2 seconds if no collision
	InitialLifeSpan = 2.0f;

}

void AMGunBullet::SetVelocity(double vel)
{
	ProjectileMovement->InitialSpeed = vel + 103000.f;
	ProjectileMovement->MaxSpeed = vel + 103000.f;
}

void AMGunBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Destroy object for now if it hits something
	Destroy();
}

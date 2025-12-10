// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/PooledProjectile.h"
#include "Game/Lesson12.09/ObjectPoolSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

APooledProjectile::APooledProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create static mesh component
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	ProjectileMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ProjectileMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Create projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity by default

	bIsActive = false;
}

void APooledProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Get the pool subsystem
	if (UWorld* World = GetWorld())
	{
		PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	}

	// Initially in pool - visible but inactive
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	ProjectileMovement->StopMovementImmediately();
}

void APooledProjectile::NativeActive(FObjectPoolActivationData ObjectPoolData)
{
	// Reset the projectile
	ResetProjectile();

	// Set transform from activation data
	if (!ObjectPoolData.ObjectPoolTransform.Equals(FTransform::Identity))
	{
		SetActorTransform(ObjectPoolData.ObjectPoolTransform);
	}

	// Activate the projectile
	bIsActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// Start movement if we have a valid direction
	FVector Direction = ObjectPoolData.ObjectPoolTransform.GetRotation().GetForwardVector();
	if (!Direction.IsNearlyZero())
	{
		ProjectileMovement->Velocity = Direction * InitialSpeed;
		ProjectileMovement->Activate();
	}

	// Set lifetime timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LifetimeTimerHandle,
			this,
			&APooledProjectile::OnLifetimeExpired,
			Lifetime,
			false
		);
	}
}

void APooledProjectile::NativeDeactive(FObjectPoolDeactivationData DeactivationData)
{
	// Clear lifetime timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	// Stop movement
	ProjectileMovement->StopMovementImmediately();

	// Reset the projectile
	ResetProjectile();

	// Move to pool location if specified
	if (DeactivationData.bShouldResetTransform)
	{
		if (!DeactivationData.DeactivationLocation.IsNearlyZero())
		{
			SetActorLocation(DeactivationData.DeactivationLocation);
		}
		else
		{
			SetActorLocation(PoolLocation);
		}
		SetActorRotation(FRotator::ZeroRotator);
	}
	else if (!DeactivationData.DeactivationLocation.IsNearlyZero())
	{
		SetActorLocation(DeactivationData.DeactivationLocation);
	}

	// Deactivate the projectile but keep it visible in the pool
	bIsActive = false;
	SetActorHiddenInGame(false); // Keep visible in pool
	SetActorEnableCollision(false);
}

void APooledProjectile::OnLifetimeExpired()
{
	// Return to pool when lifetime expires
	if (PoolSubsystem && bIsActive)
	{
		FObjectPoolDeactivationData DeactivationData;
		DeactivationData.DeactivationReason = FName("LifetimeExpired");
		DeactivationData.bShouldResetTransform = true;

		// Find the class in the pool map and return this actor
		if (UWorld* World = GetWorld())
		{
			UObjectPoolSubsystem* Subsystem = World->GetSubsystem<UObjectPoolSubsystem>();
			if (Subsystem)
			{
				// We need to find which class this projectile belongs to
				// For now, we'll use GetClass()
				TSubclassOf<AActor> ProjectileClass = GetClass();
				TScriptInterface<IObjectPoolInterface> Interface = this;
				Subsystem->ReturnObjectToPool(ProjectileClass, Interface);
			}
		}
	}
}

void APooledProjectile::ResetProjectile()
{
	// Reset velocity
	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->Deactivate();

	// Reset any other state if needed
}


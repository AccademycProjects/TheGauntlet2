// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/ProjectileSpawner.h"
#include "Game/Lesson12.09/ObjectPoolSubsystem.h"
#include "Game/Lesson12.09/PooledProjectile.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "Game/Utility/Gauntlet_DebugHelper.h"

AProjectileSpawner::AProjectileSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create arrow component to show spawn direction
	SpawnDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirectionArrow"));
	SpawnDirectionArrow->SetupAttachment(RootComponent);
	SpawnDirectionArrow->SetArrowColor(FLinearColor::Red);
	SpawnDirectionArrow->SetArrowSize(2.0f);
}

void AProjectileSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Get the pool subsystem
	if (UWorld* World = GetWorld())
	{
		PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
		
		// Initialize pool if requested
		if (bInitializePoolOnBeginPlay)
		{
			InitializePool();
		}
	}
}

void AProjectileSpawner::InitializePool()
{
	if (!PoolSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
		}
	}

	if (PoolSubsystem && ProjectileClass)
	{
		// Add pool for the projectile class
		PoolSubsystem->AddPool(ProjectileClass, InitialPoolSize);
	}
}

void AProjectileSpawner::SpawnProjectile()
{
	// Use the spawner's transform
	SpawnProjectileAtLocation(GetActorLocation(), GetActorRotation());
}

void AProjectileSpawner::SpawnProjectileAtLocation(const FVector& Location, const FRotator& Rotation)
{
	if (!PoolSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
		}
	}

	if (!PoolSubsystem || !ProjectileClass || bPrintDebugMessages)
	{
		UGauntlet_DebugHelper::ShowWarning(TEXT("ProjectileSpawner::SpawnProjectile - PoolSubsystem or ProjectileClass is null"));
		return;
	}

	// Get projectile from pool
	TScriptInterface<IObjectPoolInterface> PooledObject = PoolSubsystem->GetObjectFromPool(ProjectileClass);

	if (!PooledObject.GetObject() || bPrintDebugMessages)
	{
		UGauntlet_DebugHelper::ShowWarning(TEXT("ProjectileSpawner::SpawnProjectile - Failed to get projectile from pool"));
		return;
	}
	
	// Create activation data
	FObjectPoolActivationData ActivationData;
	ActivationData.ObjectPoolTransform = FTransform(Rotation, Location, FVector::OneVector);
	ActivationData.ObjectPoolName = FName("SpawnedProjectile");
	ActivationData.ObjectPoolID = FString::Printf(TEXT("Projectile_%d"), FMath::RandRange(1000, 9999));
	
	// Activate the projectile
	if (IObjectPoolInterface* PoolInterface = Cast<IObjectPoolInterface>(PooledObject.GetObject()))
	{
		// Reset transform and movement before activation so the projectile always starts at the requested location
		if (AActor* Actor = Cast<AActor>(PooledObject.GetObject()))
		{
			Actor->SetActorLocationAndRotation(Location, Rotation);
		}
		
		PoolInterface->Active(ActivationData);
	}
}

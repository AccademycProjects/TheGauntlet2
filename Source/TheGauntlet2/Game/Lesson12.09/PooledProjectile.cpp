// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/PooledProjectile.h"
#include "Game/Lesson12.09/ObjectPoolSubsystem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

APooledProjectile::APooledProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	/* ================= COLLISION ROOT ================= */

	CollisionRoot = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionRoot"));
	CollisionRoot->InitSphereRadius(5.f);
	CollisionRoot->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionRoot->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionRoot->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionRoot->SetNotifyRigidBodyCollision(true);
	CollisionRoot->SetSimulatePhysics(false);
	RootComponent = CollisionRoot;

	/* ================= VISUAL MESH ================= */

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionRoot);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* ================= MOVEMENT ================= */

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionRoot;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bAutoActivate = false;
}

void APooledProjectile::BeginPlay()
{
	Super::BeginPlay();

	PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();

	CollisionRoot->OnComponentHit.AddDynamic(
		this,
		&APooledProjectile::OnProjectileHit
	);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void APooledProjectile::NativeActive(FObjectPoolActivationData Data)
{
	bIsActive = true;

	SetActorTransform(Data.ObjectPoolTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	ProjectileMovement->Activate(true);

	const FVector Direction =
		Data.ObjectPoolTransform.GetRotation().GetForwardVector();

	ProjectileMovement->Velocity = Direction * InitialSpeed;
	ProjectileMovement->UpdateComponentVelocity();

	GetWorld()->GetTimerManager().SetTimer(
		LifetimeTimerHandle,
		this,
		&APooledProjectile::OnLifetimeExpired,
		Lifetime,
		false
	);
}

void APooledProjectile::NativeDeactive(FObjectPoolDeactivationData Data)
{
	bIsActive = false;

	GetWorld()->GetTimerManager().ClearTimer(LifetimeTimerHandle);

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	ResetProjectile();

	if (Data.bShouldResetTransform)
	{
		SetActorLocation(
			Data.DeactivationLocation.IsNearlyZero()
				? PoolLocation
				: Data.DeactivationLocation
		);
		SetActorRotation(FRotator::ZeroRotator);
	}

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void APooledProjectile::OnProjectileHit(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	FVector,
	const FHitResult& Hit
)
{
	if (!bIsActive || !OtherActor || OtherActor == this)
	{
		return;
	}

	ReturnToPool(TEXT("Hit"), Hit.ImpactPoint);
}

void APooledProjectile::OnLifetimeExpired()
{
	if (!bIsActive)
	{
		return;
	}

	ReturnToPool(TEXT("LifetimeExpired"), GetActorLocation());
}

void APooledProjectile::ReturnToPool(FName Reason, const FVector& Location)
{
	if (!PoolSubsystem)
	{
		return;
	}

	FObjectPoolDeactivationData Data;
	Data.DeactivationReason = Reason;
	Data.DeactivationLocation = Location;
	Data.bShouldResetTransform = true;

	TScriptInterface<IObjectPoolInterface> Interface = this;
	PoolSubsystem->ReturnObjectToPool(GetClass(), Interface);
}

void APooledProjectile::ResetProjectile()
{
	ProjectileMovement->Velocity = FVector::ZeroVector;
}

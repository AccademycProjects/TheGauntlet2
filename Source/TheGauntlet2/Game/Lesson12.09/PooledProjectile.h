// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Lesson12.09/ObjectPoolInterface.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PooledProjectile.generated.h"

/**
 * A projectile actor that implements the ObjectPoolInterface
 * Can be pooled and reused multiple times
 */
UCLASS()
class THEGAUNTLET2_API APooledProjectile : public AActor, public IObjectPoolInterface
{
	GENERATED_BODY()
	
public:	
	APooledProjectile();

protected:
	/** Static mesh component for the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	/** Initial speed of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialSpeed = 2000.0f;

	/** Maximum speed of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxSpeed = 2000.0f;

	/** Lifetime of the projectile in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Lifetime = 5.0f;

	/** Pool location where inactive projectiles are stored */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	FVector PoolLocation = FVector(0.0f, 0.0f, -1000.0f);

	/** Whether the projectile is currently active */
	bool bIsActive = false;

	/** Timer handle for lifetime */
	FTimerHandle LifetimeTimerHandle;

	/** Reference to the pool subsystem */
	UPROPERTY()
	class UObjectPoolSubsystem* PoolSubsystem;

public:
	// Begin IObjectPoolInterface implementation
	virtual void NativeActive(FObjectPoolActivationData ObjectPoolData) override;
	virtual void NativeDeactive(FObjectPoolDeactivationData DeactivationData) override;
	// End IObjectPoolInterface implementation

protected:
	virtual void BeginPlay() override;

	/** Called when the projectile lifetime expires */
	UFUNCTION()
	void OnLifetimeExpired();

	/** Reset the projectile to its default state */
	void ResetProjectile();
};


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Lesson12.09/ObjectPoolInterface.h"
#include "PooledProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UObjectPoolSubsystem;

/**
 * Pool-safe projectile actor.
 * Uses a collision root for stable movement and pooling.
 */
UCLASS()
class THEGAUNTLET2_API APooledProjectile
	: public AActor
	, public IObjectPoolInterface
{
	GENERATED_BODY()

public:
	APooledProjectile();

	// IObjectPoolInterface
	virtual void NativeActive(FObjectPoolActivationData ObjectPoolData) override;
	virtual void NativeDeactive(FObjectPoolDeactivationData DeactivationData) override;

protected:
	virtual void BeginPlay() override;

protected:
	/** Collision root component (movement + hit detection) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionRoot;

	/** Visual mesh (no collision) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** Initial speed of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialSpeed = 2000.f;

	/** Maximum speed of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxSpeed = 2000.f;

	/** Lifetime of the projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Lifetime = 5.f;

	/** Pool location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	FVector PoolLocation = FVector(0, 0, -1000);

	/** Whether the projectile is currently active */
	bool bIsActive = false;

	/** Lifetime timer */
	FTimerHandle LifetimeTimerHandle;

	/** Pool subsystem reference */
	UPROPERTY()
	UObjectPoolSubsystem* PoolSubsystem;

protected:
	/** Called when the projectile hits something */
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	/** Called when lifetime expires */
	UFUNCTION()
	void OnLifetimeExpired();

	/** Returns the projectile to the pool */
	void ReturnToPool(FName Reason, const FVector& Location);

	/** Resets internal state */
	void ResetProjectile();
};

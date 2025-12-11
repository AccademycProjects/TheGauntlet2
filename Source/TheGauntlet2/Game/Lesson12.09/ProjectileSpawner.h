// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Lesson12.09/ObjectPoolInterface.h"
#include "ProjectileSpawner.generated.h"

class UArrowComponent;
class USceneComponent;
class UObjectPoolSubsystem;

/**
 * Spawner actor that spawns projectiles from the object pool
 * Can be placed in the scene and used to spawn projectiles on demand
 */
UCLASS()
class THEGAUNTLET2_API AProjectileSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileSpawner();

protected:
	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* RootSceneComponent;

	/** Arrow component to show spawn direction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* SpawnDirectionArrow;

	/** Class of projectile to spawn from the pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Spawner")
	TSubclassOf<AActor> ProjectileClass;

	/** Initial size of the pool when created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Spawner", meta = (ClampMin = 1, ClampMax = 1000))
	int32 InitialPoolSize = 50;

	/** Whether to initialize the pool on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Spawner")
	bool bInitializePoolOnBeginPlay = true;

	/** If true, print debug messages */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Spawner", meta = (DisplayName = "Print Debug Messages"))
	bool bPrintDebugMessages = false;

	/** Reference to the pool subsystem */
	UPROPERTY()
	UObjectPoolSubsystem* PoolSubsystem;

public:
	/** Spawn a projectile from the pool at the spawner's location and direction */
	UFUNCTION(BlueprintCallable, Category = "Projectile Spawner")
	void SpawnProjectile();

	/** Spawn a projectile from the pool at a specific location and direction */
	UFUNCTION(BlueprintCallable, Category = "Projectile Spawner")
	void SpawnProjectileAtLocation(const FVector& Location, const FRotator& Rotation);

	/** Initialize the pool for this spawner's projectile class */
	UFUNCTION(BlueprintCallable, Category = "Projectile Spawner")
	void InitializePool();

protected:
	virtual void BeginPlay() override;
};


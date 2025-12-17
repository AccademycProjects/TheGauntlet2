// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Game/Lesson12.09/ObjectPoolInterface.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FObjectPool
{
	GENERATED_BODY()

	/** Desired base size of the pool (minimum size, never goes below this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	int32 BaseSize = 0;

	/** Timestamps of recent requests (for calculating consumption rate) */
	UPROPERTY()
	TArray<double> RecentRequestTimes;

	/** Usable objects in pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	TArray<TScriptInterface<IObjectPoolInterface>> UsablePoolingObjects;

	/** Active objects out of pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	TArray<TScriptInterface<IObjectPoolInterface>> ActivePoolingObjects;
};

/** Struct to hold pool statistics for a class */
USTRUCT(BlueprintType)
struct FPoolStatistics
{
	GENERATED_BODY()

	/** Index of the pool in the list */
	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 PoolIndex = 0;

	/** Name of the object class */
	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	FString ObjectName;

	/** Number of objects currently in the pool (available) */
	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 InPool = 0;

	/** Number of objects currently out of the pool (active) */
	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 OutPool = 0;

	/** Total number of objects (InPool + OutPool) */
	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 Total = 0;
};

UCLASS()
class THEGAUNTLET2_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Map with the Objects we want to pool
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Object Pool SubSystem")
	TMap<TSubclassOf<AActor>, FObjectPool> ObjectPoolMap;

	// Add a pool of objects
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	void AddPool(TSubclassOf<AActor> ClassPool, int32 InitialSize = 50);

	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TScriptInterface<IObjectPoolInterface> GetObjectFromPool(TSubclassOf<AActor> ClassPool);

	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	void ReturnObjectToPool(TSubclassOf<AActor> ClassPool, TScriptInterface<IObjectPoolInterface> ActorToReturn);
	
	void UpdateStats();
	
	/** Get the Object Pool Settings */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	const UObjectPoolSettings* GetPoolSettings() const;

	/** Get pool statistics as structured data */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TArray<FPoolStatistics> GetPoolStatistics() const;

	/** Get pool statistics as formatted strings (one string per row) */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TArray<FString> GetPoolStatisticsAsStrings() const;
	
private:
	// Check if a class implements the ObjectPoolInterface
	bool DoesClassImplementInterface(TSubclassOf<AActor> ClassPool) const;

	// Check if a class is valid
	bool IsClassValid(TSubclassOf<AActor> ClassPool) const;

	// Spawn and add a number of actors to a pool (usable list) and place them in the grid
	void SpawnAndPlaceInPool(TSubclassOf<AActor> ClassPool, int32 Count, FObjectPool& Pool);

	// Record a request and optionally auto-scale the pool
	void RecordUsageAndMaybeGrow(TSubclassOf<AActor> ClassPool, FObjectPool& Pool);

	// Shrink pool towards base size after inactivity
	void MaybeShrinkPool(TSubclassOf<AActor> ClassPool, FObjectPool& Pool);
};
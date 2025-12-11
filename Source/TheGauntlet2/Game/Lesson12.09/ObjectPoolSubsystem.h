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

	/** Desired base size of the pool (initial size) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Object Pool Struct")
	int32 BaseSize = 0;

	/** Last time (seconds) a request for this pool was made */
	UPROPERTY()
	double LastRequestTime = 0.0;

	/** Recent request timestamps used to estimate demand */
	UPROPERTY()
	TArray<double> RecentRequestTimes;

	// Array with the pointers of the USABLE objects IN this pool
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Object Pool Struct")
	TArray<TScriptInterface<IObjectPoolInterface>> UsablePoolingObjects;

	// Array with the pointers of the ACTIVE IN SCENE objects FROM this pool
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Object Pool Struct")
	TArray<TScriptInterface<IObjectPoolInterface>> ActivePoolingObjects;

};

/** Struct to hold pool statistics for a class */
USTRUCT(BlueprintType)
struct FPoolStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	FString ClassName;

	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 ActiveObjects = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 PullableObjects = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pool Statistics")
	int32 TotalObjects = 0;
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

	/** Force a pre-warm/resize for a given class */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	void PrewarmPool(TSubclassOf<AActor> ClassPool, int32 AdditionalSize);

	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TScriptInterface<IObjectPoolInterface> GetObjectFromPool(TSubclassOf<AActor> ClassPool);

	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	void ReturnObjectToPool(TSubclassOf<AActor> ClassPool, TScriptInterface<IObjectPoolInterface> ActorToReturn);
	
	void UpdateStats();

	/** Base location for the pool visualization in the scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	FVector PoolVisualizationLocation = FVector(0.0f, 0.0f, 100.0f);

	/** Spacing between pooled objects in the visualization grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	float PoolGridSpacing = 200.0f;

	/** Time window (seconds) to estimate request rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	float AutoScaleWindowSeconds = 2.0f;

	/** Look-ahead time (seconds) to pre-allocate before exhaustion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	float AutoScaleLookaheadSeconds = 1.0f;

	/** Minimum number of objects to add when scaling up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	int32 AutoScaleMinAdd = 5;

	/** Seconds of inactivity before shrinking back toward the base size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool SubSystem")
	float InactivityShrinkSeconds = 10.0f;

	/** Get statistics for all pools */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TArray<FPoolStatistics> GetPoolStatistics() const;

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
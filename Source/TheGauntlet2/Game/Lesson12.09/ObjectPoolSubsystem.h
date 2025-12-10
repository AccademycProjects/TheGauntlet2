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

	/** Get statistics for all pools */
	UFUNCTION(BlueprintCallable, Category = "Object Pool SubSystem")
	TArray<FPoolStatistics> GetPoolStatistics() const;

private:
	// Check if a class implements the ObjectPoolInterface
	bool DoesClassImplementInterface(TSubclassOf<AActor> ClassPool) const;

	// Check if a class is valid
	bool IsClassValid(TSubclassOf<AActor> ClassPool) const;
};
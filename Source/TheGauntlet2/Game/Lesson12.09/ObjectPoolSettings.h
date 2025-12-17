// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectPoolSettings.generated.h"

/**
 * Settings for Object Pool Subsystem
 * Access via: Edit > Project Settings > Game > Object Pool Settings
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Object Pool Settings"))
class THEGAUNTLET2_API UObjectPoolSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UObjectPoolSettings(const FObjectInitializer& ObjectInitializer);

	/** Time window (seconds) to measure consumption rate */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ConsumptionRateWindowSeconds = 2.0f;

	/** Minimum time to exhaustion (seconds) before adding more objects */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float MinExhaustionTimeSeconds = 1.0f;

	/** Target time to exhaustion (seconds) when shrinking pool */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float TargetExhaustionTimeSeconds = 5.0f;

	/** Maximum time to exhaustion (seconds) before shrinking pool */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "1.0", ClampMax = "120.0"))
	float MaxExhaustionTimeSeconds = 50.0f;

	/** Minimum number of objects to add when scaling up */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MinObjectsToAdd = 5;

	/** How often to check and adjust pool sizes (seconds) */
	UPROPERTY(Config, EditAnywhere, Category = "Auto Scaling", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float PoolAdjustmentInterval = 0.5f;

#if WITH_EDITORONLY_DATA
	/** Base location for the pool visualization in the scene */
	UPROPERTY(Config, EditAnywhere, Category = "Visualization")
	FVector PoolVisualizationLocation = FVector(0.0f, 0.0f, 100.0f);

	/** Spacing between pooled objects in the visualization grid */
	UPROPERTY(Config, EditAnywhere, Category = "Visualization", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float PoolGridSpacing = 200.0f;
#endif

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }
};


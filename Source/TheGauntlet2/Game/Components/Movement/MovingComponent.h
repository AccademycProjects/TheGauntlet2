// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MovingComponent.generated.h"

UENUM(BlueprintType)
enum class EMovingMode : uint8
{
	Loop,       // A -> B -> C -> A -> B -> C ...
	PingPong    // A -> B -> C -> B -> A -> B ...
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEGAUNTLET2_API UMovingComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMovingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Waypoints in local space (relative to actor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (MakeEditWidget = true))
	TArray<FVector> Waypoints;

	/** Movement mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovingMode MovementMode = EMovingMode::PingPong;

	/** Speed in units per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "1.0"))
	float Speed = 200.f;

	/** Wait time at each waypoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float WaitTime = 0.5f;

	/** Draw debug path in editor and game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Debug")
	bool bDrawDebugPath = true;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartMoving();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopMoving();

private:
	int32 CurrentWaypointIndex = 0;
	int32 Direction = 1; // 1 = forward, -1 = backward (for PingPong)
	float WaitTimer = 0.f;
	bool bIsMoving = true;
	FVector StartLocation;

	FVector GetWorldWaypoint(int32 Index) const;
	void MoveToNextWaypoint();
	void DrawDebugWaypoints() const;
};

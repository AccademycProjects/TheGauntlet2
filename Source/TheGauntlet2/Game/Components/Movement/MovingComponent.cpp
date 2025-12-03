// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Movement/MovingComponent.h"
#include "DrawDebugHelpers.h"

UMovingComponent::UMovingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMovingComponent::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetOwner()->GetActorLocation();

	// Add start position as first waypoint if not empty
	if (Waypoints.Num() > 0)
	{
		CurrentWaypointIndex = 0;
	}
}

void UMovingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDrawDebugPath)
	{
		DrawDebugWaypoints();
	}

	if (!bIsMoving || Waypoints.Num() < 2) return;

	// Handle wait time
	if (WaitTimer > 0.f)
	{
		WaitTimer -= DeltaTime;
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector TargetLocation = GetWorldWaypoint(CurrentWaypointIndex);
	const FVector CurrentLocation = Owner->GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= Speed * DeltaTime)
	{
		// Reached waypoint
		Owner->SetActorLocation(TargetLocation);
		WaitTimer = WaitTime;
		MoveToNextWaypoint();
	}
	else
	{
		// Move towards waypoint
		const FVector NewLocation = CurrentLocation + (ToTarget.GetSafeNormal() * Speed * DeltaTime);
		Owner->SetActorLocation(NewLocation);
	}
}

#if WITH_EDITOR
void UMovingComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FVector UMovingComponent::GetWorldWaypoint(int32 Index) const
{
	if (!Waypoints.IsValidIndex(Index)) return StartLocation;

	return StartLocation + Waypoints[Index];
}

void UMovingComponent::MoveToNextWaypoint()
{
	if (MovementMode == EMovingMode::Loop)
	{
		CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
	}
	else // PingPong
	{
		CurrentWaypointIndex += Direction;

		if (CurrentWaypointIndex >= Waypoints.Num() - 1)
		{
			CurrentWaypointIndex = Waypoints.Num() - 1;
			Direction = -1;
		}
		else if (CurrentWaypointIndex <= 0)
		{
			CurrentWaypointIndex = 0;
			Direction = 1;
		}
	}
}

void UMovingComponent::DrawDebugWaypoints() const
{
	if (Waypoints.Num() < 1) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	const FVector ActorLocation = GetOwner() ? GetOwner()->GetActorLocation() : StartLocation;

	for (int32 i = 0; i < Waypoints.Num(); ++i)
	{
		const FVector WaypointWorld = StartLocation + Waypoints[i];

		// Draw sphere at waypoint
		DrawDebugSphere(World, WaypointWorld, 20.f, 8, FColor::Cyan, false, -1.f, 0, 2.f);

		// Draw line to next waypoint
		if (i < Waypoints.Num() - 1)
		{
			const FVector NextWaypoint = StartLocation + Waypoints[i + 1];
			DrawDebugLine(World, WaypointWorld, NextWaypoint, FColor::Yellow, false, -1.f, 0, 2.f);
		}
		else if (MovementMode == EMovingMode::Loop && Waypoints.Num() > 1)
		{
			// Connect last to first for loop mode
			const FVector FirstWaypoint = StartLocation + Waypoints[0];
			DrawDebugLine(World, WaypointWorld, FirstWaypoint, FColor::Yellow, false, -1.f, 0, 2.f);
		}

		// Draw index number
		DrawDebugString(World, WaypointWorld + FVector(0, 0, 30), FString::FromInt(i), nullptr, FColor::White, -1.f, false);
	}

	// Draw current target line
	if (Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		DrawDebugLine(World, ActorLocation, GetWorldWaypoint(CurrentWaypointIndex), FColor::Green, false, -1.f, 0, 3.f);
	}
}

void UMovingComponent::StartMoving()
{
	bIsMoving = true;
}

void UMovingComponent::StopMoving()
{
	bIsMoving = false;
}

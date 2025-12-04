// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Teleporter.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AGauntlet_Character;

UCLASS()
class THEGAUNTLET2_API ATeleporter : public AActor
{
	GENERATED_BODY()

public:
	ATeleporter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Name of the level to teleport to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter")
	FName TargetLevelName;

	/** Delay in seconds before teleporting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleporter", meta = (ClampMin = "0.0"))
	float TeleportDelay = 2.f;

	/** Is teleportation in progress? */
	UPROPERTY(BlueprintReadOnly, Category = "Teleporter")
	bool bIsTeleporting = false;

	FTimerHandle TeleportTimerHandle;

	/** Called when player enters the teleporter */
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Teleports the player to the target level */
	void TeleportPlayer();

	/** Event called when teleportation starts - Implement visual feedback in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Teleporter")
	void OnTeleportationStarted();

	/** Event called when teleportation completes - Implement visual feedback in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Teleporter")
	void OnTeleportationCompleted();
};

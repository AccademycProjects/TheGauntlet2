// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Actors/Base/InteractableActor.h"
#include "CannonTurret.generated.h"

class UHealthComponent;

UCLASS()
class THEGAUNTLET2_API ACannonTurret : public AInteractableActor
{
	GENERATED_BODY()

public:
	ACannonTurret();

protected:
	virtual void BeginPlay() override;

	/** Where the shot originates (usually a scene component or mesh socket) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> MuzzlePoint;

	/** Damage per shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 20.f;

	/** Range of the hitscan trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Range = 3000.f;

	/** Called when the turret fires (for VFX/SFX in Blueprint) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnCannonFired(const FVector& HitLocation, AActor* HitActor);

public:
	/** Called when player interacts */
	virtual void OnInteracted_Implementation(AGauntlet_Character* Interactor) override;

	/** Fires a single hitscan shot */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireOnce();
};

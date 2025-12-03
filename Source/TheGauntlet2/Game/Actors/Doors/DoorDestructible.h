// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Actors/Base/DoorBase.h"
#include "DoorDestructible.generated.h"

class UHealthComponent;

/**
 * Destructible door that can be destroyed by taking damage
 */
UCLASS()
class THEGAUNTLET2_API ADoorDestructible : public ADoorBase
{
	GENERATED_BODY()

public:
	ADoorDestructible();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** Called when health reaches zero */
	UFUNCTION()
	void OnHealthComponentDeath(UHealthComponent* HealthComp, AActor* DamageInstigator);

	/** Destroys the door */
	UFUNCTION(BlueprintCallable, Category = "Destructible Door")
	void DestroyDoor();
};

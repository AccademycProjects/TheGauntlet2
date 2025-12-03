// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActivableComponent.generated.h"

/**
 * Base abstract component for activable objects.
 * Inherit from this to create components that can be activated (doors, platforms, etc.)
 */
UCLASS(Abstract, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEGAUNTLET2_API UActivableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActivableComponent();

	/**
	 * Activates this component.
	 * Override this in derived classes to implement custom activation logic.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Activable")
	void PerformActivation(AGauntlet_Character* Interactor);

protected:
	virtual void BeginPlay() override;
};

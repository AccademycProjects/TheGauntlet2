// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Components/Activables/ActivableComponent.h"
#include "ToggleComponent.generated.h"

class AGauntlet_Character;
class AInteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleOn, UToggleComponent*, ToggleComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleOff, UToggleComponent*, ToggleComponent);

/**
 * Component that toggles between on/off states and calls SystemInteract on a target actor
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEGAUNTLET2_API UToggleComponent : public UActivableComponent
{
	GENERATED_BODY()

public:
	UToggleComponent();

	/** Target actor to call SystemInteract on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle")
	TObjectPtr<AInteractableActor> TargetActor;

	/** Current toggle state */
	UPROPERTY(BlueprintReadWrite, Category = "Toggle")
	bool bIsOn = false;

	/** Get the current toggle state */
	UFUNCTION(BlueprintCallable, Category = "Toggle")
	bool IsOn() const { return bIsOn; }

	/** Called when toggled on */
	UFUNCTION(BlueprintNativeEvent, Category = "Toggle")
	void OnToggleOn();

	/** Called when toggled off */
	UFUNCTION(BlueprintNativeEvent, Category = "Toggle")
	void OnToggleOff();

	/** Delegate broadcast when toggle is turned on */
	UPROPERTY(BlueprintAssignable, Category = "Toggle")
	FOnToggleOn OnToggleOnDelegate;

	/** Delegate broadcast when toggle is turned off */
	UPROPERTY(BlueprintAssignable, Category = "Toggle")
	FOnToggleOff OnToggleOffDelegate;

protected:
	virtual void PerformActivation_Implementation(AGauntlet_Character* Interactor) override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class AGauntlet_Character;

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for all interactable objects (doors, levers, pickups, etc.)
 */
class THEGAUNTLET2_API IInteractable
{
	GENERATED_BODY()

public:
	/** Called when the player interacts with this object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AGauntlet_Character* Interactor);

	/** Called when the system interacts with this object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SystemInteract(AGauntlet_Character* Interactor);

	/** Returns whether this object can currently be interacted with */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AGauntlet_Character* Interactor) const;

	/** Called when interaction prompt visibility should change - Implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionPromptVisibilityChanged(bool bVisible);
};

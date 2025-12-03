// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Actors/Base/InteractableActor.h"
#include "DoorBase.generated.h"

class USceneComponent;
class AGauntlet_Character;

UCLASS()
class THEGAUNTLET2_API ADoorBase : public AInteractableActor
{
	GENERATED_BODY()

public:
	ADoorBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	/** Current door state */
	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bIsOpen = false;

public:
	/** Event called when the door is interacted with */
	virtual void OnInteracted_Implementation(AGauntlet_Character* Interactor) override;

	/** Opens the door */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

	/** Closes the door */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor();

	/** Toggles door state */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void ToggleDoor();

	/** Returns current door state */
	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsOpen() const { return bIsOpen; }

	/** Event called when door opens - Implement animation in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorOpened();

	/** Event called when door closes - Implement animation in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorClosed();
};

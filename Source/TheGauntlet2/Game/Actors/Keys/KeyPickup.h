// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Interfaces/Interactable.h"
#include "KeyPickup.generated.h"

class USceneComponent;
class UWidgetComponent;
class AGauntlet_Character;

UCLASS()
class THEGAUNTLET2_API AKeyPickup : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AKeyPickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	/** Unique ID for this key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	FName KeyID = NAME_None;

public:
	// IInteractable interface
	virtual void Interact_Implementation(AGauntlet_Character* Interactor) override;
	virtual bool CanInteract_Implementation(AGauntlet_Character* Interactor) const override;

};

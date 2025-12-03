// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Activables/Controller_Timed.h"
#include "Game/Actors/Base/InteractableActor.h"
#include "Game/Interfaces/Interactable.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Kismet/GameplayStatics.h"

UController_Timed::UController_Timed()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UController_Timed::BeginPlay()
{
	Super::BeginPlay();
}

void UController_Timed::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timer if component is being destroyed
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DeactivateTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UController_Timed::PerformActivation_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor || !TargetActor) return;

	InteractorCharacter = Interactor;

	// Interact with the target actor (e.g., open door)
	if (TargetActor->Implements<UInteractable>())
	{
		IInteractable::Execute_SystemInteract(TargetActor, Interactor);
	}

	// Set timer to deactivate after duration
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DeactivateTimerHandle,
			this,
			&UController_Timed::Deactivate,
			Duration,
			false
		);
	}
}

void UController_Timed::Deactivate()
{
	if (!TargetActor || !InteractorCharacter) return;

	// Interact again to deactivate (e.g., close door)
	if (TargetActor->Implements<UInteractable>())
	{
		IInteractable::Execute_SystemInteract(TargetActor, InteractorCharacter);
	}

	InteractorCharacter = nullptr;
}

void UController_Timed::DeactivateNow()
{
	// Clear timer and deactivate immediately
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DeactivateTimerHandle);
	}

	Deactivate();
}


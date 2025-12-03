// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Activables/ToggleComponent.h"
#include "Game/Actors/Base/InteractableActor.h"
#include "Game/Interfaces/Interactable.h"
#include "Game/Characters/Gauntlet_Character.h"

UToggleComponent::UToggleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UToggleComponent::PerformActivation_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor) return;

	// Toggle state
	bIsOn = !bIsOn;

	// Call appropriate event
	if (bIsOn)
	{
		OnToggleOn();
		OnToggleOnDelegate.Broadcast(this);
	}
	else
	{
		OnToggleOff();
		OnToggleOffDelegate.Broadcast(this);
	}

	// Call SystemInteract on target actor
	if (TargetActor && TargetActor->Implements<UInteractable>())
	{
		IInteractable::Execute_SystemInteract(TargetActor, Interactor);
	}
}

void UToggleComponent::OnToggleOn_Implementation()
{
	// Stub
}

void UToggleComponent::OnToggleOff_Implementation()
{
	// Stub
}

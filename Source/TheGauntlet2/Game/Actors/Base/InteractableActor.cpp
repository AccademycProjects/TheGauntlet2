// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Base/InteractableActor.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Game/Components/Requirements/RequirementComponent.h"
#include "Game/Components/Activables/ActivableComponent.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	// Find all requirement components
	GetComponents<URequirementComponent>(RequirementComponents);

	// Find all activable components
	GetComponents<UActivableComponent>(ActivableComponents);

	// Remove invalid entries (shouldn't happen, but safety check)
	RequirementComponents.RemoveAll([](const TObjectPtr<URequirementComponent>& Comp) { return !IsValid(Comp); });
	ActivableComponents.RemoveAll([](const TObjectPtr<UActivableComponent>& Comp) { return !IsValid(Comp); });
}

void AInteractableActor::Interact_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor)
		return;

	if (bIsLockedForPlayerInteraction)
		return;

	IInteractable::Execute_SystemInteract(this, Interactor);
}

void AInteractableActor::SystemInteract_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor)
		return;

	// Check requirements
	if (!IInteractable::Execute_CanInteract(this, Interactor))
		return;

	// Activate all activable components
	for (UActivableComponent* Comp : ActivableComponents)
	{
		if (IsValid(Comp))
		{
			Comp->PerformActivation(Interactor);
		}
	}

	// Call overridable virtual method for derived classes
	OnInteracted(Interactor);
}


void AInteractableActor::OnInteracted_Implementation(AGauntlet_Character* Interactor)
{
	// Base implementation does nothing
	// Override in derived classes
}

bool AInteractableActor::CanInteract_Implementation(AGauntlet_Character* Interactor) const
{
	if (!Interactor) return false;

	// Check all requirement components
	for (URequirementComponent* Requirement : RequirementComponents)
	{
		if (!IsValid(Requirement))
			continue;

		if (!Requirement->CheckRequirement(Interactor))
			return false;
	}

	return true;
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Requirements/Toggle_Req.h"
#include "Game/Components/Requirements/RequirementComponent.h"
#include "Game/Components/Activables/ToggleComponent.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "GameFramework/Actor.h"

UToggle_Req::UToggle_Req()
{
	// Stub
}

void UToggle_Req::BeginPlay()
{
	Super::BeginPlay();

	// Find ToggleComponents on target actors
	CachedToggleComponents.Empty();

	for (AActor* TargetActor : TargetActors)
	{
		if (!IsValid(TargetActor))
			continue;

		TArray<UToggleComponent*> Toggles;
		TargetActor->GetComponents<UToggleComponent>(Toggles);

		// Add the first toggle component found on each actor
		if (Toggles.Num() > 0 && IsValid(Toggles[0]))
		{
			CachedToggleComponents.Add(Toggles[0]);
		}
	}
}

bool UToggle_Req::CheckRequirement_Implementation(AGauntlet_Character* Interactor)
{
	// If no toggles found, requirement is always satisfied
	if (CachedToggleComponents.Num() == 0)
	{
		return true;
	}

	const bool bRequiredState = (RequirementMode == EToggleRequirementMode::AllMustBeOn);

	// Check all cached toggle components
	for (UToggleComponent* Toggle : CachedToggleComponents)
	{
		if (!IsValid(Toggle))
			continue;

		// If any toggle doesn't match the required state, requirement fails
		if (Toggle->IsOn() != bRequiredState)
		{
			return false;
		}
	}

	// All toggles match the required state
	return true;
}

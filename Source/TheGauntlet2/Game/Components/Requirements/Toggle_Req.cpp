// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Requirements/Toggle_Req.h"
#include "Game/Components/Activables/ToggleComponent.h"
#include "Game/Characters/Gauntlet_Character.h"

UToggle_Req::UToggle_Req()
{
}

void UToggle_Req::BeginPlay()
{
	Super::BeginPlay();

	// Cache toggle components from target actors
	CachedToggleComponents.Empty();
	
	for (AActor* TargetActor : TargetActors)
	{
		if (TargetActor)
		{
			UToggleComponent* ToggleComp = TargetActor->FindComponentByClass<UToggleComponent>();
			if (ToggleComp)
			{
				CachedToggleComponents.Add(ToggleComp);
			}
		}
	}
}

bool UToggle_Req::CheckRequirement_Implementation(AGauntlet_Character* Interactor)
{
	// If no toggles to check, requirement is satisfied
	if (CachedToggleComponents.Num() == 0)
	{
		return true;
	}

	// Check all toggles based on requirement mode
	for (UToggleComponent* ToggleComp : CachedToggleComponents)
	{
		if (!ToggleComp)
		{
			continue;
		}

		bool bIsOn = ToggleComp->IsOn();

		if (RequirementMode == EToggleRequirementMode::AllMustBeOn)
		{
			if (!bIsOn)
			{
				return false;
			}
		}
		else // AllMustBeOff
		{
			if (bIsOn)
			{
				return false;
			}
		}
	}

	return true;
}

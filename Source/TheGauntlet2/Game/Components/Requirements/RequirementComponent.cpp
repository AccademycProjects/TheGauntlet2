// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Requirements/RequirementComponent.h"
#include "Game/Characters/Gauntlet_Character.h"

URequirementComponent::URequirementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URequirementComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool URequirementComponent::CheckRequirement_Implementation(AGauntlet_Character* Interactor)
{
	// Base implementation always returns true
	// Override in derived classes to implement custom logic
	return true;
}

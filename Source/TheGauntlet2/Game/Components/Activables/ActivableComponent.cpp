// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Activables/ActivableComponent.h"

UActivableComponent::UActivableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UActivableComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UActivableComponent::PerformActivation_Implementation(AGauntlet_Character* Interactor)
{
	// Base implementation does nothing
	// Override in derived classes to implement custom logic
}

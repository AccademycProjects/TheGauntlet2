// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Keys/KeyPickup.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"

AKeyPickup::AKeyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
}

void AKeyPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AKeyPickup::Interact_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor) return;

	// Check if character has AddKey method (it should, but safety check)
	Interactor->AddKey(KeyID);

	// Destroy the key
	Destroy();
}

bool AKeyPickup::CanInteract_Implementation(AGauntlet_Character* Interactor) const
{
	// Keys can always be picked up
	return Interactor != nullptr;
}

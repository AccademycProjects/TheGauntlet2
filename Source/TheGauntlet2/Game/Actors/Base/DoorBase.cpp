// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Base/DoorBase.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Components/SceneComponent.h"

ADoorBase::ADoorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
}

void ADoorBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADoorBase::OnInteracted_Implementation(AGauntlet_Character* Interactor)
{
	ToggleDoor();
}

void ADoorBase::OpenDoor()
{
	if (bIsOpen) return;

	bIsOpen = true;
	OnDoorOpened();
}

void ADoorBase::CloseDoor()
{
	if (!bIsOpen) return;

	bIsOpen = false;
	OnDoorClosed();
}

void ADoorBase::ToggleDoor()
{
	if (bIsOpen)
	{
		CloseDoor();
	}
	else
	{
		OpenDoor();
	}
}

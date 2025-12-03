// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Doors/DoorDestructible.h"
#include "Game/Components/Health/HealthComponent.h"

ADoorDestructible::ADoorDestructible()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ADoorDestructible::BeginPlay()
{
	Super::BeginPlay();

	// Bind to health component death event
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ADoorDestructible::OnHealthComponentDeath);
	}
}

void ADoorDestructible::OnHealthComponentDeath(UHealthComponent* HealthComp, AActor* DamageInstigator)
{
	DestroyDoor();
}

void ADoorDestructible::DestroyDoor()
{
	Destroy();
}

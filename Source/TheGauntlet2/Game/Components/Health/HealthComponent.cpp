// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Health/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// Bind to owner's damage event
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
}

void UHealthComponent::ApplyDamage(float Damage, AActor* Instigator)
{
	if (!IsAlive() || Damage <= 0.f) return;

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	const float Delta = CurrentHealth - OldHealth;

	OnHealthChanged.Broadcast(this, CurrentHealth, Delta, Instigator);

	if (!IsAlive())
	{
		OnDeath.Broadcast(this, Instigator);
	}
}

void UHealthComponent::Heal(float Amount, AActor* Instigator)
{
	if (!IsAlive() || Amount <= 0.f) return;

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	const float Delta = CurrentHealth - OldHealth;

	if (Delta > 0.f)
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, Delta, Instigator);
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	ApplyDamage(Damage, DamageCauser);
}


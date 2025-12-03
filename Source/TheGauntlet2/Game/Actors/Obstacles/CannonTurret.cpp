// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Obstacles/CannonTurret.h"
#include "Game/Components/Health/HealthComponent.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

ACannonTurret::ACannonTurret()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(RootComponent);
}

void ACannonTurret::BeginPlay()
{
	Super::BeginPlay();
}

void ACannonTurret::OnInteracted_Implementation(AGauntlet_Character* Interactor)
{
	FireOnce();
}

void ACannonTurret::FireOnce()
{
	if (!MuzzlePoint) return;

	FVector Start = MuzzlePoint->GetComponentLocation();
	FVector End = Start + (MuzzlePoint->GetForwardVector() * Range);

	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		false,
		1.f,
		0,
		2.f
	);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	AActor* HitActor = nullptr;
	FVector HitLocation = End;

	if (bHit)
	{
		HitActor = Hit.GetActor();
		HitLocation = Hit.ImpactPoint;

		// Print the name of the actor that was hit
		if (GEngine && HitActor)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 2.f, FColor::Yellow,
				FString::Printf(TEXT("Hit Actor: %s"), *HitActor->GetName())
			);
		}

		// Apply damage
		if (HitActor)
		{
			if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
			{
				const float AppliedDamage = Damage;
				Health->ApplyDamage(AppliedDamage, this);

				// Print the amount of damage applied
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1, 2.f, FColor::Red,
						FString::Printf(TEXT("Damage Applied: %.1f"), AppliedDamage)
					);

					GEngine->AddOnScreenDebugMessage(
						-1, 2.f, FColor::Green,
						FString::Printf(TEXT("New Health: %.1f"), Health->CurrentHealth)
					);
				}
			}
			else
			{
				// Print the name of the actor that was hit
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1, 2.f, FColor::Cyan,
						TEXT("Hit Actor has NO HealthComponent")
					);
				}
			}
		}
	}
}

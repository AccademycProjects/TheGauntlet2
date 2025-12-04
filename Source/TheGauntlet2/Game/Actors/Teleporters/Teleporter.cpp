// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Actors/Teleporters/Teleporter.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "Game/Core/Gauntlet_GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ATeleporter::ATeleporter()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionBox;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

void ATeleporter::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnBeginOverlap);
	}
}

void ATeleporter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timer if component is being destroyed
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TeleportTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ATeleporter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it's the player character
	AGauntlet_Character* PlayerCharacter = Cast<AGauntlet_Character>(OtherActor);
	if (!PlayerCharacter || bIsTeleporting) return;

	// Check if this is the player's pawn
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || PC->GetPawn() != PlayerCharacter) return;

	bIsTeleporting = true;
	OnTeleportationStarted();

	// Set timer to teleport after delay
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportTimerHandle,
			this,
			&ATeleporter::TeleportPlayer,
			TeleportDelay,
			false
		);
	}
}

void ATeleporter::TeleportPlayer()
{
	if (TargetLevelName == NAME_None) return;

	// Load the target level
	if (UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>())
	{
		OnTeleportationCompleted();
		GI->LoadLevel(TargetLevelName);
	}
}

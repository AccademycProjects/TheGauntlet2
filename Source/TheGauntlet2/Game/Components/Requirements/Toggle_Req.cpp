// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Requirements/Toggle_Req.h"
#include "Game/Components/Activables/ToggleComponent.h"
#include "Game/Characters/Gauntlet_Character.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

UToggle_Req::UToggle_Req()
{
    // Stub
}

void UToggle_Req::BeginPlay()
{
    Super::BeginPlay();

    CachedToggleComponents.Empty();

    UWorld* World = GetWorld();
    if (!World)
        return;

    // Scan for actors with matching tags
    for (const FName& Tag : TargetActorTags)
    {
        if (Tag.IsNone())
            continue;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!IsValid(Actor))
                continue;

            if (!Actor->ActorHasTag(Tag))
                continue;

            // Search for Toggle Components on this actor
            TArray<UToggleComponent*> Toggles;
            Actor->GetComponents<UToggleComponent>(Toggles);

            for (UToggleComponent* Toggle : Toggles)
            {
                if (IsValid(Toggle))
                {
                    CachedToggleComponents.Add(Toggle);
                }
            }
        }
    }
}

bool UToggle_Req::CheckRequirement_Implementation(AGauntlet_Character* Interactor)
{
    // No toggle components → always valid
    if (CachedToggleComponents.Num() == 0)
        return true;

    const bool bRequiredState = (RequirementMode == EToggleRequirementMode::AllMustBeOn);

    for (UToggleComponent* Toggle : CachedToggleComponents)
    {
        if (!IsValid(Toggle))
            continue;

        if (Toggle->IsOn() != bRequiredState)
        {
            return false;
        }
    }

    return true;
}

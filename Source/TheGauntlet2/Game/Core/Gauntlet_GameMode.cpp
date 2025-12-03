// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Core/Gauntlet_GameMode.h"
#include "Game/Core/Gauntlet_GameInstance.h"
#include "Kismet/GameplayStatics.h"

void AGauntlet_GameMode::BeginPlay()
{
    Super::BeginPlay();

    // Detect if we're in main menu or gameplay based on map name
    const FString MapName = GetWorld()->GetMapName();

    if (UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>())
    {
        if (MapName.Contains(TEXT("MainMenu")))
        {
            GI->SetGameState(EGauntlet_GameState::InMainMenu);
        }
        else
        {
            GI->SetGameState(EGauntlet_GameState::InPlay);
        }
    }
}

void AGauntlet_GameMode::CompleteLevel()
{
    // Broadcast event so UI, GameInstance, etc. can react
    OnLevelCompleted.Broadcast();

    // Here you can add logic for win, like disabling input, starting a timer, etc.
}

void AGauntlet_GameMode::FailLevel()
{
    // Logic for lose, e.g., restart level, show lose UI, etc.
}

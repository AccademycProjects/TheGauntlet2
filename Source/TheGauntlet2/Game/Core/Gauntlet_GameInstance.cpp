// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Core/Gauntlet_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#pragma region Level Management

void UGauntlet_GameInstance::LoadMainMenu()
{
    SetGameState(EGauntlet_GameState::InLoading);
    UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UGauntlet_GameInstance::LoadLevel(FName LevelName)
{
    SetGameState(EGauntlet_GameState::InLoading);
    UGameplayStatics::OpenLevel(this, LevelName);
}

void UGauntlet_GameInstance::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

#pragma endregion

#pragma region Game State

void UGauntlet_GameInstance::SetGameState(EGauntlet_GameState NewState)
{
    if (CurrentGameState != NewState)
    {
        CurrentGameState = NewState;
        OnGameStateChanged.Broadcast(NewState);
    }
}

#pragma endregion

#pragma region Input Device

void UGauntlet_GameInstance::SetInputDevice(EGauntlet_InputDevice NewDevice)
{
    if (CurrentInputDevice != NewDevice)
    {
        CurrentInputDevice = NewDevice;
        OnInputDeviceChanged.Broadcast(NewDevice);
    }
}

#pragma endregion

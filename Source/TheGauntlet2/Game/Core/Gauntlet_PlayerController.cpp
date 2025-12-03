// Fill out your copyright notice in the Description page of Project Settings.

#include "Gauntlet_PlayerController.h"

void AGauntlet_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>())
	{
		// Bind to game state and input device changes
		GI->OnGameStateChanged.AddDynamic(this, &AGauntlet_PlayerController::OnGameStateChanged);
		GI->OnInputDeviceChanged.AddDynamic(this, &AGauntlet_PlayerController::OnInputDeviceChanged);

		// Apply current state immediately
		UpdateCursorState();
	}
}

void AGauntlet_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>())
	{
		GI->OnGameStateChanged.RemoveDynamic(this, &AGauntlet_PlayerController::OnGameStateChanged);
		GI->OnInputDeviceChanged.RemoveDynamic(this, &AGauntlet_PlayerController::OnInputDeviceChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AGauntlet_PlayerController::OnGameStateChanged(EGauntlet_GameState NewState)
{
	UpdateCursorState();
}

void AGauntlet_PlayerController::OnInputDeviceChanged(EGauntlet_InputDevice NewDevice)
{
	UpdateCursorState();
}

void AGauntlet_PlayerController::UpdateCursorState()
{
	UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>();
	if (!GI) return;

	const EGauntlet_GameState State = GI->GetGameState();
	const bool bUsingMouseKeyboard = (GI->GetInputDevice() == EGauntlet_InputDevice::MouseKeyboard);

	switch (State)
	{
	case EGauntlet_GameState::InMainMenu:
	case EGauntlet_GameState::InPause:
		// Show cursor only if using mouse & keyboard
		bShowMouseCursor = bUsingMouseKeyboard;
		SetInputMode(FInputModeUIOnly());
		break;

	case EGauntlet_GameState::InPlay:
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
		break;

	case EGauntlet_GameState::InLoading:
	case EGauntlet_GameState::InSaving:
		// Keep current state during loading/saving
		break;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/Utility/Gauntlet_DebugHelper.h"
#include "Engine/Engine.h"

int32 UGauntlet_DebugHelper::MessageKeyCounter = 0;

void UGauntlet_DebugHelper::ShowMessage(const FString& Message, float Duration, const FLinearColor& Color, bool bNewerOnTop)
{
	if (!GEngine)
	{
		return;
	}

	// Use a unique key for each message to prevent overlapping
	const int32 MessageKey = bNewerOnTop ? --MessageKeyCounter : ++MessageKeyCounter;

	GEngine->AddOnScreenDebugMessage(
		MessageKey,
		Duration,
		Color.ToFColor(true), // Convert LinearColor to FColor
		Message
	);
}

void UGauntlet_DebugHelper::ShowSuccess(const FString& Message, float Duration)
{
	ShowMessage(Message, Duration, FLinearColor::Green);
}

void UGauntlet_DebugHelper::ShowWarning(const FString& Message, float Duration)
{
	ShowMessage(Message, Duration, FLinearColor::Yellow);
}

void UGauntlet_DebugHelper::ShowError(const FString& Message, float Duration)
{
	ShowMessage(Message, Duration, FLinearColor::Red);
}

void UGauntlet_DebugHelper::ShowInfo(const FString& Message, float Duration)
{
	ShowMessage(Message, Duration, FLinearColor::Blue);
}


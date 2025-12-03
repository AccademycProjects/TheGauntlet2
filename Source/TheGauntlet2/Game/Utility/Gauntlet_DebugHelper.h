// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Gauntlet_DebugHelper.generated.h"

/**
 * Static utility class for displaying on-screen debug messages.
 * Provides a centralized way to show debug information across the entire project.
 * 
 * Usage example:
 *   UGauntlet_DebugHelper::ShowMessage(TEXT("Player health: 100"), FLinearColor::Green);
 */
UCLASS()
class THEGAUNTLET2_API UGauntlet_DebugHelper : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Displays a debug message on screen.
	 * 
	 * @param Message The message text to display.
	 * @param Duration Duration in seconds for the message to remain visible (default: 3.0f).
	 * @param Color The color of the message text (default: White).
	 * @param bNewerOnTop If true, newer messages appear on top (default: true).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug|On-Screen",
		meta = (Tooltip = "Displays a debug message on screen. Can be called from anywhere."))
	static void ShowMessage(
		const FString& Message,
		float Duration = 3.0f,
		const FLinearColor& Color = FLinearColor::White,
		bool bNewerOnTop = true
	);

	/**
	 * Displays a success message on screen (green color).
	 * 
	 * @param Message The message text to display.
	 * @param Duration Duration in seconds for the message to remain visible (default: 3.0f).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug|On-Screen",
		meta = (Tooltip = "Displays a success message on screen in green color."))
	static void ShowSuccess(const FString& Message, float Duration = 3.0f);

	/**
	 * Displays a warning message on screen (yellow color).
	 * 
	 * @param Message The message text to display.
	 * @param Duration Duration in seconds for the message to remain visible (default: 5.0f).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug|On-Screen",
		meta = (Tooltip = "Displays a warning message on screen in yellow color."))
	static void ShowWarning(const FString& Message, float Duration = 5.0f);

	/**
	 * Displays an error message on screen (red color).
	 * 
	 * @param Message The message text to display.
	 * @param Duration Duration in seconds for the message to remain visible (default: 5.0f).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug|On-Screen",
		meta = (Tooltip = "Displays an error message on screen in red color."))
	static void ShowError(const FString& Message, float Duration = 5.0f);

	/**
	 * Displays an info message on screen (cyan color).
	 * 
	 * @param Message The message text to display.
	 * @param Duration Duration in seconds for the message to remain visible (default: 3.0f).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug|On-Screen",
		meta = (Tooltip = "Displays an info message on screen in cyan color."))
	static void ShowInfo(const FString& Message, float Duration = 3.0f);

private:

	/** Counter for unique message keys to ensure messages don't overlap */
	static int32 MessageKeyCounter;
};


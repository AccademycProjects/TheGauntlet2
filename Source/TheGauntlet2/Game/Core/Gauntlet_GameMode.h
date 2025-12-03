// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gauntlet_GameMode.generated.h"

// Declare delegate for level completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelCompleted);

/**
 *
 */
UCLASS()
class THEGAUNTLET2_API AGauntlet_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // Called when the player wins the level
    UFUNCTION(BlueprintCallable)
    void CompleteLevel();

    // Called when the player loses (optional)
    UFUNCTION(BlueprintCallable)
    void FailLevel();

    // Delegate broadcasted when the level is completed
    UPROPERTY(BlueprintAssignable)
    FOnLevelCompleted OnLevelCompleted;

};

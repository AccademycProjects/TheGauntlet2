// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Components/Requirements/RequirementComponent.h"
#include "Toggle_Req.generated.h"

class UToggleComponent;
class AGauntlet_Character;

UENUM(BlueprintType)
enum class EToggleRequirementMode : uint8
{
    AllMustBeOn,
    AllMustBeOff
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEGAUNTLET2_API UToggle_Req : public URequirementComponent
{
    GENERATED_BODY()

public:
    UToggle_Req();

protected:
    virtual void BeginPlay() override;

    /** Tags of actors that contain ToggleComponents to check */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle Requirement")
    TArray<FName> TargetActorTags;

    /** Requirement mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toggle Requirement")
    EToggleRequirementMode RequirementMode = EToggleRequirementMode::AllMustBeOn;

    /** Cached toggle components found on target actors (runtime only) */
    UPROPERTY(Transient)
    TArray<UToggleComponent*> CachedToggleComponents;

    virtual bool CheckRequirement_Implementation(AGauntlet_Character* Interactor) override;
};

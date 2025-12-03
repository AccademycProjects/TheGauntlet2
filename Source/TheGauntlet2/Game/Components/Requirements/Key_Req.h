// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Components/Requirements/RequirementComponent.h"
#include "Key_Req.generated.h"

/**
 * Requirement component that checks if the player has a specific key
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEGAUNTLET2_API UKey_Req : public URequirementComponent
{
	GENERATED_BODY()

public:
	UKey_Req();

	/** Key ID required to open the door */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Requirement")
	FName RequiredKeyID = NAME_None;

protected:
	virtual bool CheckRequirement_Implementation(class AGauntlet_Character* Interactor) override;
};

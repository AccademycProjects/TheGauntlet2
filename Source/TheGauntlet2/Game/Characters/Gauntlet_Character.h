// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gauntlet_Character.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;

UCLASS()
class THEGAUNTLET2_API AGauntlet_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGauntlet_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

#pragma endregion

#pragma region Input

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void Interact();
	void TogglePause();

#pragma endregion

#pragma region Gameplay

public:
	// Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float SprintSpeed = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float JumpZVelocity = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float AirControl = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float GravityScale = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float GroundFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Movement")
	float BrakingDecelerationWalking = 2048.f;

	// Key inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Inventory")
	TArray<FName> CollectedKeys;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Inventory")
	void AddKey(FName KeyID);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Inventory")
	bool HasKey(FName KeyID) const;

#pragma endregion

#pragma region Interaction

public:
	virtual void Tick(float DeltaTime) override;

	// Interaction detection settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Interaction", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float InteractionConeAngle = 45.f; // Half-angle in degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay|Interaction|Debug")
	bool bDebugDrawInteractionCone = false;

	// Current focused interactable (can be used by UI)
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay|Interaction")
	TObjectPtr<AActor> FocusedInteractable;

protected:
	/** Scans for interactables in front of the character */
	void UpdateInteractionFocus();

	/** Called when focused interactable changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay|Interaction")
	void OnInteractionFocusChanged(AActor* NewFocus, AActor* OldFocus);

#pragma endregion
};

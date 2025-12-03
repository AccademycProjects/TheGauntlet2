// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Characters/Gauntlet_Character.h"
#include "Game/Core/Gauntlet_GameInstance.h"
#include "Game/Interfaces/Interactable.h"
#include "Game/Components/Health/HealthComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

AGauntlet_Character::AGauntlet_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Character rotation settings
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->GravityScale = GravityScale;
	GetCharacterMovement()->GroundFriction = GroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalking;

	// Health Component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void AGauntlet_Character::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGauntlet_Character: NO PlayerController in BeginPlay"));
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGauntlet_Character: NO LocalPlayer"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
	if (!Subsystem)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGauntlet_Character: NO EnhancedInput Subsystem"));
		return;
	}

	if (!InputMappingContext)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGauntlet_Character: InputMappingContext is NULL"));
		return;
	}

	Subsystem->AddMappingContext(InputMappingContext, 0);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AGauntlet_Character: MappingContext ADDED"));

}

void AGauntlet_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGauntlet_Character::Move);

			if (!MoveAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("MoveAction is NULL"));
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGauntlet_Character::Look);

			if (!LookAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LookAction is NULL"));
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

			if (!JumpAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("JumpAction is NULL"));
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AGauntlet_Character::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AGauntlet_Character::StopSprint);

			if (!SprintAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SprintAction is NULL"));
		}
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AGauntlet_Character::Interact);

			if (!InteractAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("InteractAction is NULL"));
		}
		if (PauseAction)
		{
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AGauntlet_Character::TogglePause);

			if (!PauseAction && GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PauseAction is NULL"));
		}
	}
}

#pragma region Input Functions

void AGauntlet_Character::Move(const FInputActionValue& Value)
{
	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (Controller && !MoveInput.IsNearlyZero())
	{
		// Get forward and right vectors based on controller rotation
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MoveInput.Y);
		AddMovementInput(RightDir, MoveInput.X);

		if (GEngine)
		{
			const FString Msg = FString::Printf(
				TEXT("MoveInput: X = %f, Y = %f"),
				MoveInput.X,
				MoveInput.Y
			);

			GEngine->AddOnScreenDebugMessage(
				1,
				5.f,
				FColor::Green,
				Msg
			);
		}
	}
}

void AGauntlet_Character::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}

	if (GEngine)
	{
		const FString Msg = FString::Printf(
			TEXT("LookInput: X = %f, Y = %f"),
			LookInput.X,
			LookInput.Y
		);

		GEngine->AddOnScreenDebugMessage(
			1,
			5.f,
			FColor::Green,
			Msg
		);
	}
}

void AGauntlet_Character::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AGauntlet_Character::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AGauntlet_Character::Interact()
{
	if (FocusedInteractable && FocusedInteractable->Implements<UInteractable>())
	{
		IInteractable::Execute_Interact(FocusedInteractable, this);
	}
}

void AGauntlet_Character::TogglePause()
{
	if (UGauntlet_GameInstance* GI = GetGameInstance<UGauntlet_GameInstance>())
	{
		const EGauntlet_GameState CurrentState = GI->GetGameState();

		if (CurrentState == EGauntlet_GameState::InPlay)
		{
			GI->SetGameState(EGauntlet_GameState::InPause);
		}
		else if (CurrentState == EGauntlet_GameState::InPause)
		{
			GI->SetGameState(EGauntlet_GameState::InPlay);
		}
	}
}

#pragma endregion

#pragma region Gameplay

void AGauntlet_Character::AddKey(FName KeyID)
{
	if (!CollectedKeys.Contains(KeyID))
	{
		CollectedKeys.Add(KeyID);
	}
}

bool AGauntlet_Character::HasKey(FName KeyID) const
{
	return CollectedKeys.Contains(KeyID);
}

#pragma endregion

#pragma region Interaction

void AGauntlet_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateInteractionFocus();

#if ENABLE_DRAW_DEBUG
	if (bDebugDrawInteractionCone)
	{
		const FVector Start = GetActorLocation();
		const FVector Forward = GetActorForwardVector();
		const float HalfAngleRad = FMath::DegreesToRadians(InteractionConeAngle);

		// Draw cone lines
		const int32 NumSegments = 16;
		for (int32 i = 0; i < NumSegments; ++i)
		{
			const float Angle = (2.f * PI * i) / NumSegments;
			FVector Dir = Forward.RotateAngleAxis(InteractionConeAngle, FVector::UpVector);
			Dir = Dir.RotateAngleAxis(FMath::RadiansToDegrees(Angle), Forward);
			DrawDebugLine(GetWorld(), Start, Start + Dir * InteractionRange, FColor::Yellow, false, -1.f, 0, 1.f);
		}

		// Draw range sphere
		DrawDebugSphere(GetWorld(), Start, InteractionRange, 16, FColor::Cyan, false, -1.f, 0, 0.5f);

		// Draw focused interactable
		if (FocusedInteractable)
		{
			DrawDebugLine(GetWorld(), Start, FocusedInteractable->GetActorLocation(), FColor::Green, false, -1.f, 0, 2.f);
		}
	}
#endif
}

void AGauntlet_Character::UpdateInteractionFocus()
{
	AActor* BestCandidate = nullptr;
	float BestScore = -1.f;

	const FVector CharacterLocation = GetActorLocation();
	const FVector ForwardVector = GetActorForwardVector();
	const float ConeAngleCos = FMath::Cos(FMath::DegreesToRadians(InteractionConeAngle));

	// Find all actors in range with IInteractable
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRange);

	if (GetWorld()->OverlapMultiByChannel(Overlaps, CharacterLocation, FQuat::Identity, ECC_Visibility, Sphere))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			if (!Actor || Actor == this) continue;

			// Check if implements IInteractable
			if (!Actor->Implements<UInteractable>()) continue;

			// Check if can interact
			if (!IInteractable::Execute_CanInteract(Actor, this)) continue;

			// Check if within cone
			const FVector ToActor = (Actor->GetActorLocation() - CharacterLocation).GetSafeNormal();
			const float DotProduct = FVector::DotProduct(ForwardVector, ToActor);

			if (DotProduct >= ConeAngleCos)
			{
				// Score based on angle (closer to center = higher score)
				if (DotProduct > BestScore)
				{
					BestScore = DotProduct;
					BestCandidate = Actor;
				}
			}
		}
	}

	// Update focus if changed
	if (BestCandidate != FocusedInteractable)
	{
		AActor* OldFocus = FocusedInteractable;
		FocusedInteractable = BestCandidate;

		// Notify old focus to hide prompt
		if (OldFocus && OldFocus->Implements<UInteractable>())
		{
			IInteractable::Execute_OnInteractionPromptVisibilityChanged(OldFocus, false);
		}

		// Notify new focus to show prompt
		if (BestCandidate && BestCandidate->Implements<UInteractable>())
		{
			IInteractable::Execute_OnInteractionPromptVisibilityChanged(BestCandidate, true);
		}

		OnInteractionFocusChanged(BestCandidate, OldFocus);
	}
}

#pragma endregion

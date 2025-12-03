// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Components/Requirements/Key_Req.h"
#include "Game/CHaracters/Gauntlet_Character.h"

UKey_Req::UKey_Req()
{
}

bool UKey_Req::CheckRequirement_Implementation(AGauntlet_Character* Interactor)
{
	if (!Interactor) return false;

	// If no key required, always pass
	if (RequiredKeyID == NAME_None)
	{
		return true;
	}

	// Check if player has the required key
	return Interactor->HasKey(RequiredKeyID);
}

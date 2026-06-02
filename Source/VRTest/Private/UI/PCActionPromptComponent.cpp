// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PCActionPromptComponent.h"

#include "Game/Characters/BasePCPlayer.h"
#include "Grabber/PCGrabHand.h"
#include "Skill/PlayerSkillComponent.h"

UPCActionPromptComponent::UPCActionPromptComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPCActionPromptComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<ABasePCPlayer>(GetOwner());
}

void UPCActionPromptComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPlayer)
	{
		return;
	}

	TArray<EPCActionPromptType> PrevPrompts = MoveTemp(CurrentPrompts);
	RefreshPrompts();

	if (!ArePromptsEqual(PrevPrompts, CurrentPrompts))
	{
		OnPromptsChanged.Broadcast(CurrentPrompts);
	}
}

void UPCActionPromptComponent::RefreshPrompts()
{
	TArray<EPCActionPromptType> Prompts;

	// --- Global ---
	if (OwnerPlayer->PlayerSkillComponent && OwnerPlayer->PlayerSkillComponent->IsStarDrawEnabled())
	{
		Prompts.Add(EPCActionPromptType::StarDraw);
	}
	if (OwnerPlayer->InventoryComponent && OwnerPlayer->InventoryComponent->HasBow())
	{
		Prompts.Add(EPCActionPromptType::ToggleBow);
	}
	Prompts.Add(EPCActionPromptType::Crouch);

	// --- Vault & Ignite ---
	if (OwnerPlayer->TargetedHitComponent && OwnerPlayer->TargetedHitComponent->ComponentHasTag(FName(TEXT("Interact_WindowVault"))))
	{
		Prompts.Add(EPCActionPromptType::Vault);
	}
	if (OwnerPlayer->bCanIgniteBySight)
	{
		Prompts.Add(EPCActionPromptType::Ignite);
	}

	if (OwnerPlayer->GetBowArmed())
	{
		// --- Bow mode ---
		if (OwnerPlayer->bIsAiming)
		{
			Prompts.Add(EPCActionPromptType::StopAim);
			if (OwnerPlayer->bIsDrawingBow)
			{
				Prompts.Add(EPCActionPromptType::ReleaseBow);
			}
			else
			{
				Prompts.Add(EPCActionPromptType::DrawBow);
			}
		}
		else
		{
			Prompts.Add(EPCActionPromptType::Aim);
		}
	}
	else
	{
		// --- Non-Bow mode ---
		const UPCGrabHand* LeftHand  = OwnerPlayer->PCLeftHand;
		const UPCGrabHand* RightHand = OwnerPlayer->PCRightHand;
		const bool bHasTarget = OwnerPlayer->TargetedObject != nullptr;
		const bool bLeftEmpty  = LeftHand  && !LeftHand->bIsHolding;
		const bool bRightEmpty = RightHand && !RightHand->bIsHolding;
		const bool bLeftHolding  = LeftHand  && LeftHand->bIsHolding;
		const bool bRightHolding = RightHand && RightHand->bIsHolding;

		if (bLeftHolding)
		{
			Prompts.Add(EPCActionPromptType::LeftRelease);
			Prompts.Add(EPCActionPromptType::LeftThrow);
		}
		else if (bLeftEmpty && bHasTarget)
		{
			Prompts.Add(EPCActionPromptType::LeftGrab);
		}

		if (bRightHolding)
		{
			Prompts.Add(EPCActionPromptType::RightRelease);
			Prompts.Add(EPCActionPromptType::RightThrow);
		}
		else if (bRightEmpty && bHasTarget)
		{
			Prompts.Add(EPCActionPromptType::RightGrab);
		}
	}

	CurrentPrompts = MoveTemp(Prompts);
}

bool UPCActionPromptComponent::ArePromptsEqual(const TArray<EPCActionPromptType>& A, const TArray<EPCActionPromptType>& B) const
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int32 i = 0; i < A.Num(); ++i)
	{
		if (A[i] != B[i])
		{
			return false;
		}
	}
	return true;
}

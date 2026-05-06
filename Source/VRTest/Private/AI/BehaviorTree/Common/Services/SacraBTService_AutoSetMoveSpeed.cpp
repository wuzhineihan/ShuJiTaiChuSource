// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTree/Common/Services/SacraBTService_AutoSetMoveSpeed.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USacraBTService_AutoSetMoveSpeed::USacraBTService_AutoSetMoveSpeed()
{
	NodeName = TEXT("Sacra Auto Set Move Speed");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	DesiredMoveSpeedKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USacraBTService_AutoSetMoveSpeed, DesiredMoveSpeedKey));
}

void USacraBTService_AutoSetMoveSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	SyncMoveSpeed(OwnerComp);
}

void USacraBTService_AutoSetMoveSpeed::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	SyncMoveSpeed(OwnerComp);
}

void USacraBTService_AutoSetMoveSpeed::SyncMoveSpeed(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	ACharacter* Character = Cast<ACharacter>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);
	if (!BlackboardComponent || !Character)
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
	if (!CharacterMovement || DesiredMoveSpeedKey.SelectedKeyName.IsNone())
	{
		if (!CharacterMovement || !bOverrideMoveSpeed)
		{
			return;
		}
	}

	const float DesiredMoveSpeed = bOverrideMoveSpeed
		? OverrideMoveSpeed
		: BlackboardComponent->GetValueAsFloat(DesiredMoveSpeedKey.SelectedKeyName);
	if (DesiredMoveSpeed < 0.0f)
	{
		return;
	}

	if (!FMath::IsNearlyEqual(CharacterMovement->MaxWalkSpeed, DesiredMoveSpeed, SpeedTolerance))
	{
		CharacterMovement->MaxWalkSpeed = DesiredMoveSpeed;
	}
}

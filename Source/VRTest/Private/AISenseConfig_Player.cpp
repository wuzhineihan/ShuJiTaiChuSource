// Fill out your copyright notice in the Description page of Project Settings.

#include "AISenseConfig_Player.h"
#include "Perception/AIPerceptionComponent.h" // so we can use the perception system
#include "GameplayDebuggerCategory.h" // so we can use the debugger

UAISenseConfig_Player::UAISenseConfig_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DebugColor = FColor::Green;
}

TSubclassOf<UAISense> UAISenseConfig_Player::GetSenseImplementation() const
{
	return Implementation;
}

#if WITH_GAMEPLAY_DEBUGGER
void UAISenseConfig_Player::DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent, FGameplayDebuggerCategory* DebuggerCategory) const
{
	if (PerceptionComponent == nullptr || DebuggerCategory == nullptr)
	{
		return;
	}
	
	const AActor* BodyActor = PerceptionComponent->GetBodyActor();
	if (BodyActor != nullptr)
	{
		FVector BodyLocation, BodyFacing;
		PerceptionComponent->GetLocationAndDirection(BodyLocation, BodyFacing);

		DebuggerCategory->AddShape(FGameplayDebuggerShape::MakeCylinder(BodyLocation, PlayerRadius, 25.0f, DebugColor));
	}

}
#endif // WITH_GAMEPLAY_DEBUGGER
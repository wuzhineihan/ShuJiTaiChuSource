// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Goap/Goap_Component.h"

// Sets default values for this component's properties
UGoap_Component::UGoap_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called every frame
void UGoap_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



TArray<UGoap_PlanAction*> UGoap_Component::Call_Planner(UGoap_PlanGoal* Goal)
{
	TArray<UGoap_PlanAction*> ChosenActions;
	if (Goal==NULL)
	{
		UE_LOG(LogTemp,Warning,TEXT("Goal == NULL"));
		return ChosenActions;
	}
	ChosenActions = Planner_Instance->PlanActionsAStar(WorldModel_Instance,Goal);
	WorldModel_Instance->Initialize();
	return ChosenActions;
}

void UGoap_Component::ChangeWorldState(FName StateName, bool IsCheck, bool StateCheck, FVector StateVector)
{
	if (!WorldModel_Instance || !WorldModel_Instance->WorldState)
	{
		UE_LOG(LogTemp, Error, TEXT("WorldModel_Instance or WorldState is NULL!"));
		return;
	}
	
	if (IsCheck)
	{

		if (WorldModel_Instance->WorldState->WorldCheck.Contains(StateName))
		{
			WorldModel_Instance->WorldState->WorldCheck[StateName] = StateCheck;
		}else
		{
			UE_LOG(LogTemp,Warning,TEXT("Do not have this World State"));
		}
	}else
	{
		if (WorldModel_Instance->WorldState->WorldPosition.Contains(StateName))
		{
			WorldModel_Instance->WorldState->WorldPosition[StateName] = StateVector;
		}else
		{
			UE_LOG(LogTemp,Warning,TEXT("Do not have this World State"));
		}
	}
}



UGoap_PlanGoal* UGoap_Component::FindGoal()
{
	UGoap_PlanGoal* ChosenGoal = nullptr;
	float CheckValue = 0.0f;
	for (const auto &goal : Goals)
	{
		float CurrentValue = goal->GetCurrentvalue(WorldModel_Instance->WorldState);
		if (CurrentValue > CheckValue)
		{
			ChosenGoal = goal;
			CheckValue = CurrentValue;
		}
		
	}
	return ChosenGoal;
}

void UGoap_Component::ApplyActionEffect(UGoap_PlanAction* ApplyAction)
{
	*WorldModel_Instance->WorldState = ApplyAction->ActionEffect(*WorldModel_Instance->WorldState);
}


// Called when the game starts
void UGoap_Component::BeginPlay()
{
	Super::BeginPlay();
	WorldModel_Instance = NewObject<UGoap_WorldModel>(this, WorldModelClass);
	Planner_Instance = NewObject<UGoap_Planner>(this,PlannerClass);
	WorldModel_Instance->WorldState = &BaseWorldState;
	
	Goals.Reserve(GoalsClass.Num());
	
	for (const auto& goal : GoalsClass)
	{
		UGoap_PlanGoal* goal_instance = NewObject<UGoap_PlanGoal>(this,goal);
		if (goal_instance==nullptr)
		{
			UE_LOG(LogTemp,Warning,TEXT("Null Goal"));
		}
		Goals.Emplace(goal_instance);
	}
	
	Actions.Reserve(ActionsClass.Num());
	
	for (const auto& action : ActionsClass)
	{
		UGoap_PlanAction* action_instance = NewObject<UGoap_PlanAction>(this,action);
		if (action_instance==nullptr)
		{
			UE_LOG(LogTemp,Warning,TEXT("Null Action"));
		}
		Actions.Emplace(action_instance);
	}
	
	WorldModel_Instance->initActions(Actions);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Goap/Goap_WorldModel.h"

UGoap_WorldModel::UGoap_WorldModel(){}

UGoap_WorldModel::~UGoap_WorldModel(){}

void UGoap_WorldModel::AddGoal(UGoap_PlanGoal* goal)
{
	goals.Add(goal);
}


void UGoap_WorldModel::initActions(TArray<UGoap_PlanAction*>& actions)
{
	actionslibrary = actions;
}

void UGoap_WorldModel::Initialize()
{
	goals.Empty();
}
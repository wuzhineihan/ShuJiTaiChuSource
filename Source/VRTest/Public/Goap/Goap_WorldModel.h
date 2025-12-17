// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Goap_PlanGoal.h"
#include"Goap_PlanAction.h"
#include"Goap_WorldState.h"
#include"Goap_WorldModel.generated.h"
/**
 * 
 */


UCLASS()
class VRTEST_API UGoap_WorldModel:public UObject
{
	GENERATED_BODY()
public:
	//...
	UGoap_WorldModel();
	~UGoap_WorldModel();
	
	//...
	virtual void initActions(TArray<UGoap_PlanAction*>& actions);
	virtual void Initialize();
	virtual void AddGoal(UGoap_PlanGoal* goal);//添加目标

	//废弃函数：
	//...

	UPROPERTY(VisibleDefaultsOnly)
	TArray<UGoap_PlanAction*> actionslibrary;
	
	UPROPERTY(VisibleDefaultsOnly)
	TArray<UGoap_PlanGoal*> goals;

	FWorldState* WorldState;
	
};

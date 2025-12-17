// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include"Goap_WorldState.h"
#include "Goap_PlanGoal.generated.h"

/**
 * 
 */
//class说明该枚举需要在有作用域的情况下使用: EGoalType::...
//旧注释：enum的


UCLASS()
class VRTEST_API UGoap_PlanGoal:public UObject
{
	GENERATED_BODY()
public:

	UGoap_PlanGoal();
	~UGoap_PlanGoal();
	
	virtual float GetDiscontentment(float value);
	virtual float GetCurrentvalue(FWorldState* CurrentWorldState);
	virtual TArray<FName> CheckGoalPreCondition(FWorldState* CurrentWorldState);
	static  bool CheckState(FName CurrentState,bool bCheck_,FWorldState* CurrentWorldState);

	//...
	//目标的名字
	FString goal_name;
	//目标当前的优先级价值
	float goal_value = 0;
	//目标优先级价值随时间改变的量
	float ChangeOverTime = 0.0;
	//影响目标的状态
	TArray<FName> StateToChange;
};

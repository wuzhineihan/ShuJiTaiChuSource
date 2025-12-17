// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Goap_WorldState.h"
#include"Goap_PlanAction.generated.h"
/**
 * 
 */
UCLASS()
class VRTEST_API UGoap_PlanAction:public UObject
{
	GENERATED_BODY()
public:
	//构造析构函数
	UGoap_PlanAction();
	~UGoap_PlanAction();

	//成员函数
	UFUNCTION(BlueprintCallable)
	virtual FWorldState ActionEffect(FWorldState CurrentWorldState);
	virtual TArray<FName> CheckActionPreCondition(FWorldState* CurrentWorldState);
	virtual float Get_Duration();
	virtual bool CanActionBeChosen(FWorldState* CurrentWorldState);
	static  bool CheckState(FName CurrentState,bool bCheck_,FWorldState* CurrentWorldState);
	virtual int GetCost(FWorldState* CurrentWorldState);
	
	UPROPERTY(BlueprintReadWrite)
	FVector ActionLocation;
	
	UPROPERTY(BlueprintReadWrite)
	bool NeedToMove;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag ActionTag;
	//该动作的名字
	UPROPERTY(BlueprintReadWrite)
	FString ActionName;
	//动作的前提状态包含哪些
	TArray<FName> PreCondition;
	//动作影响的状态包括哪些
	TArray<FName> EffectState;
	//动作的持续时间
	float duration = 0.0;
	//是否可以被打断
	bool Canbeinterrupted = false;
	//动作的消耗
	int ActionCost = 0;

};

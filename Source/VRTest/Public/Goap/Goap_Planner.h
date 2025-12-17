// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include"Goap_PlanAction.h"
#include"Goap_PlanGoal.h"
#include"Goap_WorldModel.h"
#include "Goap_Planner.generated.h"
/**
 * 
 */

struct FNode
{
	TArray<FName> StateNeedToChange;
	UGoap_PlanAction* Action;
	FNode* ParentNode;
	int Value_G;
	int Value_H;
	int Value_F;
	
	FNode(UGoap_PlanAction* _Action = nullptr, FNode* _ParentNode = nullptr)
	{
		Action = _Action;
		ParentNode = _ParentNode;
		Value_G = Value_H = Value_F =0;
	}
};

struct TNodeLess
{
	FORCEINLINE bool operator()(const FNode& a, const FNode& b) const
	{
		return a.Value_F < b.Value_F;
	}
};


UCLASS(blueprintable,BlueprintType)
class VRTEST_API UGoap_Planner:public UObject
{
	GENERATED_BODY()
public:
	//...
	UGoap_Planner();
	~UGoap_Planner();
	//...
	virtual TArray<UGoap_PlanAction*> PlanActionsAStar(UGoap_WorldModel* Model,UGoap_PlanGoal* CurrentGoal);
	virtual TArray<UGoap_PlanAction*> ReconsturtPath(FNode* CurrentNode);
	virtual int CalculateHeuristic(UGoap_PlanAction* CurrentAction,FNode* CurrentNode = nullptr);
	virtual void AddState(TArray<FName> StateGroup, FNode* CurrentNode = nullptr);
	virtual void RemoveState(TArray<FName> StateGroup,FNode* CurrentNode = nullptr);
	
	//变量
	TArray<FName> StateNeedToChange;



	
};

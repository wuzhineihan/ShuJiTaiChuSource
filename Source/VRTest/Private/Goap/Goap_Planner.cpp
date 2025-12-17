// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Goap/Goap_Planner.h"

#include "Runtime/Core/Tests/Containers/TestUtils.h"

UGoap_Planner::UGoap_Planner()
{
}

UGoap_Planner::~UGoap_Planner()
{
}

TArray<UGoap_PlanAction*> UGoap_Planner::PlanActionsAStar(UGoap_WorldModel* Model, UGoap_PlanGoal* CurrentGoal)
{
	TArray<UGoap_PlanAction*> BestActions;
	TArray<FNode*> Open_Nodes;
	Model->Initialize();
	Model->goals.Add(CurrentGoal);
	FNode* FirstNode = new FNode();
	AddState(CurrentGoal->CheckGoalPreCondition(Model->WorldState), FirstNode);
	Open_Nodes.Heapify(TNodeLess());
	Open_Nodes.HeapPush(FirstNode,TNodeLess());
	while (!Open_Nodes.IsEmpty())
	{
		FNode* CurrentNode;
		Open_Nodes.HeapPop(CurrentNode,TNodeLess());
		Open_Nodes.Remove(CurrentNode);
		
		if (CurrentNode->StateNeedToChange.IsEmpty())
		{
			BestActions = ReconsturtPath(CurrentNode);
			break;
		}
		
		for (const auto& NeighborAction:Model->actionslibrary)
		{
			int CurrentValueH = CalculateHeuristic(NeighborAction, CurrentNode);
			int CurrentValueG = CurrentNode->Value_G + NeighborAction->ActionCost;
			if (CurrentValueH >= CurrentNode->StateNeedToChange.Num())
			{
				continue;
			}
			
			FNode* NeighborNode = new FNode(NeighborAction,CurrentNode);
			NeighborNode->Value_H = CurrentValueH;
			NeighborNode->Value_G = CurrentValueG;
			NeighborNode->Value_F = CurrentValueH + CurrentValueG;
			AddState(CurrentNode->StateNeedToChange,NeighborNode);
			RemoveState(NeighborAction->EffectState,NeighborNode);
			AddState(NeighborAction->CheckActionPreCondition(Model->WorldState),NeighborNode);
			Open_Nodes.HeapPush(NeighborNode,TNodeLess());
		}
	}

	for (auto Node:Open_Nodes)
	{
		delete Node;
	}
	
	return BestActions;
}

TArray<UGoap_PlanAction*> UGoap_Planner::ReconsturtPath(FNode* CurrentNode)
{
	TArray<UGoap_PlanAction*> BestActions;
	while (CurrentNode->ParentNode != nullptr)
	{
		BestActions.Add(CurrentNode->Action);
		CurrentNode = CurrentNode->ParentNode;
	}

	return BestActions;
}


int UGoap_Planner::CalculateHeuristic(UGoap_PlanAction* CurrentAction, FNode* CurrentNode)
{
	int MatchCount = CurrentNode->StateNeedToChange.Num();
	for (const auto& StateName : CurrentAction->EffectState)
	{
		if (CurrentNode->StateNeedToChange.Contains(StateName))
		{
			MatchCount--;
		}
	}
	return MatchCount;
}

void UGoap_Planner::AddState(TArray<FName> StateGroup,FNode* CurrentNode)
{
	for (const auto& StateName : StateGroup)
	{
		CurrentNode->StateNeedToChange.Add(StateName);
	}
}

//避免访问到空指针
void UGoap_Planner::RemoveState(TArray<FName> StateGroup, FNode* CurrentNode)
{
	for (const auto& StateName : StateGroup)
	{
		if (CurrentNode->StateNeedToChange.Contains(StateName))
		{
			CurrentNode->StateNeedToChange.Remove(StateName);
		}
	}
}

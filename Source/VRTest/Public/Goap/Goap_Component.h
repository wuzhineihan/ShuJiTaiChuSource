// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Goap_Planner.h"
#include"Goap_PlanAction.h"
#include"Goap_PlanGoal.h"
#include"Goap_WorldModel.h"
#include"Goap_WorldState.h"
#include "Goap_Component.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTEST_API UGoap_Component : public UActorComponent
{
	GENERATED_BODY()
public:

	// Sets default values for this component's properties
	UGoap_Component();
	
	//成员函数	
	UFUNCTION(BlueprintCallable)
	virtual TArray<UGoap_PlanAction*> Call_Planner(UGoap_PlanGoal* Goal);

	//这里改成enum最好，直接勾就行
	UFUNCTION(BlueprintCallable)
	virtual void ChangeWorldState(FName StateName,bool IsCheck,bool StateCheck = false,FVector StateVector = FVector::ZeroVector);
	
	UFUNCTION(BlueprintCallable)
	virtual UGoap_PlanGoal* FindGoal();
	
	UFUNCTION(BlueprintCallable)
	virtual void ApplyActionEffect(UGoap_PlanAction* ApplyAction);
	
	//成员变量
	UPROPERTY(EditAnywhere,Category="Goap")
	TSubclassOf<UGoap_WorldModel> WorldModelClass;
	
	UPROPERTY(EditAnywhere,Category="Goap")
	TSubclassOf<UGoap_Planner> PlannerClass;
	
	UPROPERTY(EditAnywhere,Category="Goap")
	TArray<TSubclassOf<UGoap_PlanGoal>> GoalsClass;
	
	UPROPERTY(EditAnywhere,category="Goap")
	TArray<TSubclassOf<UGoap_PlanAction>> ActionsClass;
	
	UPROPERTY(BlueprintReadWrite,BlueprintType,category="Goap")
	TArray<UGoap_PlanAction*> BestActions;

	UPROPERTY(BlueprintReadWrite,BlueprintType,category="Goap")
	UGoap_PlanGoal* CurrentGoal;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Goap")
	FWorldState BaseWorldState;
	//这里可能还要改一下，不要在编辑器里面修改。
	
	UPROPERTY()
	TArray<UGoap_PlanAction*> Actions;

	UPROPERTY()
	TArray<UGoap_PlanGoal*> Goals;

	UPROPERTY()
	UGoap_Planner* Planner_Instance;

	UPROPERTY()
	UGoap_WorldModel* WorldModel_Instance;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};

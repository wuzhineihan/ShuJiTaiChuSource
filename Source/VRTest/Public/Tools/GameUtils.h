// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameUtils.generated.h"

/**
 * 锥形检测结果项
 */
USTRUCT(BlueprintType)
struct FActorWithAngle
{
	GENERATED_BODY()

	/** Actor 引用 */
	UPROPERTY(BlueprintReadOnly)
	AActor* Actor = nullptr;

	/** 与方向的角度（度） */
	UPROPERTY(BlueprintReadOnly)
	float Angle = 0.0f;

	FActorWithAngle() {}
	FActorWithAngle(AActor* InActor, float InAngle) : Actor(InActor), Angle(InAngle) {}
};

/**
 * 游戏通用工具函数库
 */
UCLASS()
class VRTEST_API UGameUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 在锥形范围内查找所有 Actor
	 * @param WorldContextObject 世界上下文对象
	 * @param Origin 检测起点（通常是手部/摄像机位置）
	 * @param Direction 检测方向
	 * @param Radius 球形检测半径
	 * @param MaxAngleDegrees 最大角度（度）
	 * @param ObjectTypes 检测的对象类型
	 * @param IgnoreActors 忽略的 Actor
	 * @return 找到的所有 Actor 及其角度，按角度从小到大排序
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Utils", meta = (WorldContext = "WorldContextObject"))
	static TArray<FActorWithAngle> FindActorsInCone(
		UObject* WorldContextObject,
		const FVector& Origin,
		const FVector& Direction,
		float Radius,
		float MaxAngleDegrees,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& IgnoreActors
	);
	
	UFUNCTION(BlueprintCallable, Category = "Game|Utils")
	static TArray<AActor*> SweepByChannelTest(FVector Location, UObject* WorldContextObject);
};


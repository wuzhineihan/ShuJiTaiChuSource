// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRStasisFireMonitor.generated.h"

class AStasisPoint;
class UPlayerGrabHand;
class USceneComponent;

/**
 * VR 定身球发射监视器
 * 
 * 职责：
 * - 监测 VR 手部速度
 * - 当速度超过阈值后开始下降时，自动触发定身球发射
 * - 发射后解锁手部并自毁
 */
UCLASS()
class VRTEST_API AVRStasisFireMonitor : public AActor
{
	GENERATED_BODY()
	
public:	
	AVRStasisFireMonitor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/**
	 * 初始化监视器
	 * @param InHand 要监视的手部
	 * @param InStasisPoint 要发射的定身球
	 */
	UFUNCTION(BlueprintCallable, Category = "Stasis")
	void Initialize(UPlayerGrabHand* InHand, AStasisPoint* InStasisPoint);

protected:
	// ==================== 配置参数 ====================
	
	/** 触发发射的最小手部速度阈值（cm/s） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float SpeedThreshold = 500.0f;

	/** 发射速度倍数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float FireSpeedFactor = 1.5f;

	/** 定身球目标检测���径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float DetectionRadius = 500.0f;

	/** 定身球目标检测最大角度（度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float DetectionAngle = 30.0f;

	// ==================== 状态 ====================
	
	/** 监视的手部 */
	UPROPERTY()
	UPlayerGrabHand* MonitoredHand = nullptr;

	/** 要发射的定身球 */
	UPROPERTY()
	AStasisPoint* StasisPoint = nullptr;

	/** 上一帧手部位置 */
	FVector LastHandLocation;

	/** 当前手部速度 */
	FVector CurrentVelocity;

	/** 上一帧手部速度 */
	FVector LastVelocity;

	/** 当前速度大小 */
	float CurrentSpeed = 0.0f;

	/** 上一帧速度大小 */
	float LastSpeed = 0.0f;

	/** 速度是否已超过阈值 */
	bool bSpeedOverThreshold = false;

	// ==================== 内部函数 ====================
	
	/** 更新手部速度 */
	void UpdateHandVelocity(float DeltaTime);

	/** 查找定身球目标 */
	USceneComponent* FindStasisTarget();

	/** 触发发射 */
	void FireStasisPoint();
};


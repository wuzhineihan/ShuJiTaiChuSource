// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Game/BasePlayer.h"
#include "BasePCPlayer.generated.h"

class UPCGrabHand;
class UCameraComponent;
class IGrabbable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrabTargetChanged, AActor*, NewTarget, AActor*, OldTarget);

/**
 * PC 模式玩家基类
 *
 * 包含第一人称摄像机和双手抓取逻辑。
 * 弓箭模式由基类 bIsBowArmed 控制。
 */
UCLASS()
class VRTEST_API ABasePCPlayer : public ABasePlayer
{
	GENERATED_BODY()

public:
	ABasePCPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== 组件 ====================
	
	/** 第一人称摄像机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FirstPersonCamera;

	/** 左手抓取组件（PC 具体类型，与 BasePlayer::LeftHand 指向同一对象） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* PCLeftHand;

	/** 右手抓取组件（PC 具体类型，与 BasePlayer::RightHand 指向同一对象） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* PCRightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* LeftHandCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* RightHandCollision;

	// ==================== 目标检测配置 ====================

	/** 抓取射线最大距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MaxGrabDistance = 300.0f;

	/** 抓取检测通道 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	TEnumAsByte<ECollisionChannel> GrabTraceChannel = ECC_Visibility;

	/** 是否绘制抓取射线调试（线 + 命中点） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug")
	bool bDrawGrabLineTraceDebug = false;

	/** 调试绘制持续时间（秒）；0 表示仅一帧 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug", meta=(ClampMin="0.0"))
	float GrabLineTraceDebugDrawTime = 1.0f;

	/** 调试线粗细 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Debug", meta=(ClampMin="0.1"))
	float GrabLineTraceDebugThickness = 0.5f;

	// ==================== 目标检测状态 ====================

	/** 当前瞄准的可抓取物体 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	AActor* TargetedObject = nullptr;

	/** 当前瞄准的骨骼名（如果目标是骨骼网格体） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	FName TargetedBoneName;

	/** 射线检测的碰撞点位置（用于丢弃物体等操作） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	FVector TraceTargetLocation = FVector::ZeroVector;

	/** 射线检测是否命中目标 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab")
	bool bTraceHit = false;

	// ==================== 弓箭模式配置 ====================
	
	/** 瞄准时左手的位置（相对于摄像机） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Aiming")
	FTransform AimingLeftHandTransform;

	/**
	 * PC 模式固定拉弓距离（沿摄像机前向的反方向拉）
	 * 注意：这是“手的位置偏移”，不是 Bow::MaxPullDistance。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Draw")
	float PCDrawDistance = 30.0f;

	// ==================== 弓箭状态 ====================
	
	/** 是否正在瞄准 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bIsAiming = false;

	/** 是否正在拉弓 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|State")
	bool bIsDrawingBow = false;

	// ==================== 重写基类 ====================
	
	/** 重写：进入/退出弓箭模式 */
	virtual void SetBowArmed(bool bArmed) override;

	/** 重写：返回摄像机作为追踪原点 */
	virtual USceneComponent* GetTrackOrigin() const override;

	// ==================== 输入处理 ======================================
	
	/**
	 * 处理左扳机输入
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleLeftTrigger(bool bPressed);

	/**
	 * 处理右扳机输入
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleRightTrigger(bool bPressed);

	// ==================== 弓箭操作 ====================
	
	/**
	 * 开始瞄准
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StartAiming();

	/**
	 * 停止瞄准
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StopAiming();

	/**
	 * 开始拉弓
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void StartDrawBow();

	/**
	 * 停止拉弓（已废弃：一旦开始拉弓不能取消，会直接发射）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow", meta = (DeprecatedFunction, DeprecationMessage = "StopDrawBow is deprecated. Once drawing starts, releasing will always fire the arrow."))
	void StopDrawBow();

	/**
	 * 释放弓弦
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ReleaseBowString();

protected:
	// ==================== 内部函数 ====================

	/** 更新瞄准目标检测（每帧执行） */
	void UpdateTargetDetection();

	/** 执行射线检测 */
	bool PerformLineTrace(FHitResult& OutHit, float MaxDistance) const;

	/** 当手抓取物体时的回调 */
	UFUNCTION()
	void OnHandGrabbedObject(AActor* GrabbedObject);


	/** 播放无箭音效 */
	void PlayNoArrowSound();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Game/BasePlayer.h"
#include "Game/CollisionConfig.h"
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CameraCollision;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|CameraCollision", meta=(ClampMin="0.0"))
	float CameraCollisionRadius = 12.0f;

	// ==================== 目标检测配置 ====================

	/** 抓取射线最大距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MaxGrabDistance = 300.0f;

	/** 抓取检测通道 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	TEnumAsByte<ECollisionChannel> GrabTraceChannel = TCC_GRAB;

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

	// ==================== 投掷（PC） ====================

	/** 最大投掷射线距离（摄像机前方） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	float MaxThrowDistance = 1000.0f;

	/** 投掷抛物线弧度参数（0-1，越小越平） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ThrowArcParam = 0.35f;

	// ==================== 定身术（PC） ====================

	/** 定身球发射速度倍数（相对于摄像机前向） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisFireSpeedScalar = 1000.0f;

	/** 定身球目标检测半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisDetectionRadius = 1000.0f;

	/** 定身球目标检测最大角度（度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stasis")
	float StasisDetectionAngle = 30.0f;
	
	
	// ==================== Movement ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PCCrouchedHalfHeight = 40.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PCMaxCrouchWalkSpeed = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch")
	float PCCrouchCameraInterpSpeed = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch", meta=(ClampMin="0.01"))
	float PCCrouchCameraStopThreshold = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCrouched(bool bCrouch);
	/**
	 * 投掷入口（唯一入口）。
	 * @param bRightHand true=右手投掷，false=左手投掷。
	 */
	UFUNCTION(BlueprintCallable, Category = "Throw")
	void TryThrow(bool bRightHand);

	// ==================== 重写基类 ====================
	
	/** 重写：进入/退出弓箭模式 */
	virtual void SetBowArmed(bool bArmed) override;

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
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StartStarDraw();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopStarDraw();

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
	bool PerformLineTrace(FHitResult& OutHit, float MaxDistance, ECollisionChannel TraceChannel) const;

	/** 处理 StasisPoint 投掷 */
	void HandleStasisPointThrow(UPCGrabHand* ThrowHand, class AStasisPoint* StasisPoint);

	/** 当手抓取物体时的回调 */
	UFUNCTION()
	void OnHandGrabbedObject(AActor* GrabbedObject);


	/** 播放无箭音效 */
	void PlayNoArrowSound();
	
	UPROPERTY(Transient)
	bool bIsCrouchCameraInterping = false;
	
	UPROPERTY(Transient)
	float RegularCameraRelativeZ = 0.0f;
	
	UPROPERTY(Transient)
	float RegularCapsuleHalfHeight = 0.0f;
	
	void UpdateCrouchCameraInterp(float DeltaTime);
};

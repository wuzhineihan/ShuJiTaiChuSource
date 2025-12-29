// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BasePlayer.h"
#include "BasePCPlayer.generated.h"

class UPCGrabHand;
class UCameraComponent;

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

	/** 左手 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* LeftHand;

	/** 右手 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGrabHand* RightHand;

	// ==================== 弓箭模式配置 ====================
	
	/** 瞄准时左手的位置（相对于摄像机） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Aiming")
	FTransform AimingLeftHandTransform;

	/** 最大射击距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Draw")
	float MaxShootDistance = 5000.0f;

	/** 最小拉弓距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Draw")
	float MinDrawDistance = 5.0f;

	/** 最大拉弓距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Draw")
	float MaxDrawDistance = 50.0f;

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

	/**
	 * 处理模式切换输入
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleModeSwitch(bool bToBowMode);

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
	 * 释放弓弦
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void ReleaseBowString();

protected:
	// ==================== 内部函数 ====================

	/** 计算拉弓距离 */
	float CalculateDrawDistance() const;

	/** 播放无箭音效 */
	void PlayNoArrowSound();
};

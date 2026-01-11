// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BasePlayer.h"
#include "BaseVRPlayer.generated.h"

class UVRGrabHand;
class UCameraComponent;
class UMotionControllerComponent;
class UBoxComponent;
class USphereComponent;
class UWidgetInteractionComponent;

/**
 * VR 模式玩家基类
 * 
 * 包含 VR 组件结构（VROrigin、MotionController、GrabHand）。
 * 支持 Gravity Gloves 和背包交互。
 */
UCLASS()
class VRTEST_API ABaseVRPlayer : public ABasePlayer
{
	GENERATED_BODY()

public:
	ABaseVRPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ==================== VR 组件 ====================
	
	/** VR 原点 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	USceneComponent* VROrigin;

	/** VR 摄像机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UCameraComponent* VRCamera;

	/** 背包碰撞区域 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UBoxComponent* BackpackCollision;

	/** 左手 MotionController */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UMotionControllerComponent* MotionControllerLeft;

	/** 右手 MotionController */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UMotionControllerComponent* MotionControllerRight;

	/** 左手 Aim MotionController */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UMotionControllerComponent* MotionControllerLeftAim;

	/** 右手 Aim MotionController */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UMotionControllerComponent* MotionControllerRightAim;

	/** 左手 Widget 交互 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UWidgetInteractionComponent* WidgetInteractionLeft;

	/** 右手 Widget 交互 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UWidgetInteractionComponent* WidgetInteractionRight;

	/** 左手抓取组件（VR 具体类型，与 BasePlayer::LeftHand 指向同一对象） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UVRGrabHand* VRLeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	USphereComponent* LeftHandCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	USphereComponent* RightHandCollision;

	/** 右手抓取组件（VR 具体类型，与 BasePlayer::RightHand 指向同一对象） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	UVRGrabHand* VRRightHand;

	// ==================== 配置参数 ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Components")
	FRotator VRHandRotationOffset = FRotator(0.f, 0.f, 0.f);
	
	// ==================== 输入处理 ====================
	
	/**
	 * 处理左手握键
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleLeftGrip(bool bPressed);

	/**
	 * 处理右手握键
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleRightGrip(bool bPressed);

	// ==================== 重写基类 ====================

	virtual void PlaySimpleForceFeedback(EControllerHand Hand) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Components")
	UHapticFeedbackEffect_Base* SimpleHapticEffect;
	
	// ==================== 内部函数 ====================

	/** 通用的抓取处理逻辑 */
	void HandleGrip(UVRGrabHand* Hand, bool bPressed, bool bIsLeft = false);

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void SetVRHandRotationOffset(FRotator RotationOffset);
};

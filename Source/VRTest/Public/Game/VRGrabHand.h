// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/PlayerGrabHand.h"
#include "VRGrabHand.generated.h"

class UBoxComponent;

/**
 * VR 模式手部组件
 * 
 * 使用球形检测进行抓取，支持 Gravity Gloves 隔空抓取。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UVRGrabHand : public UPlayerGrabHand
{
	GENERATED_BODY()

public:
	UVRGrabHand();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 配置 ====================
	
	/** 球形检测半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Grab")
	float GrabSphereRadius = 15.0f;

	/** 抓取检测对象类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Grab")
	TArray<TEnumAsByte<EObjectTypeQuery>> GrabObjectTypes;

	/** 背包碰撞区域引用（由 Player 设置） */
	UPROPERTY(BlueprintReadWrite, Category = "VR|Backpack")
	UBoxComponent* BackpackCollision = nullptr;

	// ==================== Gravity Gloves 配置 ====================
	
	/** 是否启用 Gravity Gloves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	bool bEnableGravityGloves = true;

	/** Gravity Gloves 检测距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	float GravityGlovesDistance = 1000.0f;

	/** Gravity Gloves 锁定角度（度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	float GravityGlovesAngle = 15.0f;

	/** 向后拉动的速度阈值（点积） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	float PullBackThreshold = 0.3f;

	/** 发射物体的抛物线弧度参数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	float LaunchArcParam = 0.5f;

	// ==================== Gravity Gloves 状态 ====================
	
	/** 当前选中的远程目标（未抓取，仅瞄准） */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	AGrabbeeObject* GravityGlovesTarget = nullptr;

	/** 是否正在虚拟抓取（Grip 按下但物体未到手） */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	bool bIsVirtualGrabbing = false;

	/** 上一帧手部位置（用于计算速度） */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	FVector LastHandLocation;

	/** 当前手部速度 */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	FVector HandVelocity;

	// ==================== 重写 ====================
	
	virtual AGrabbeeObject* FindTarget_Implementation() override;
	virtual void TryGrab(bool bFromBackpack = false) override;
	virtual void TryRelease(bool bToBackpack = false) override;

	// ==================== VR 专用接口 ====================
	
	/**
	 * 查找角度最近的可抓取物体（Gravity Gloves 用）
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	AGrabbeeObject* FindAngleClosestTarget();

	/**
	 * 虚拟抓取（Gravity Gloves）
	 * 设置状态但不实际附加物体
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	void VirtualGrab(AGrabbeeObject* Target);

	/**
	 * 虚拟释放（Gravity Gloves）
	 * 清除状态，可选择是否发射物体
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	void VirtualRelease(bool bLaunch = false);

	// ==================== VR 背包检测 ====================
	
	/**
	 * 检查手是否在背包区域内
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|Backpack")
	bool IsInBackpackArea() const;

protected:
	// ==================== 内部函数 ====================
	
	/** 更新 Gravity Gloves 逻辑 */
	void UpdateGravityGloves(float DeltaTime);

	/** 更新手部速度 */
	void UpdateHandVelocity(float DeltaTime);

	/** 检测向后拉动手势 */
	bool CheckPullBackGesture() const;

	/** 球形检测 */
	AGrabbeeObject* PerformSphereOverlap() const;

	/** 检查物体是否在 Gravity Gloves 角度范围内 */
	bool IsInGravityGlovesAngle(AGrabbeeObject* Target) const;
};

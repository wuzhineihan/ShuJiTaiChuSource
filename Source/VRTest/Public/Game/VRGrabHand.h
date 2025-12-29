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

	/** Gravity Gloves 拉取速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|GravityGloves")
	float GravityGlovesPullSpeed = 1500.0f;

	// ==================== Gravity Gloves 状态 ====================
	
	/** 当前锁定的远程目标 */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	AGrabbeeObject* GravityGlovesTarget = nullptr;

	/** 是否正在隔空拖拽 */
	UPROPERTY(BlueprintReadOnly, Category = "VR|GravityGloves")
	bool bIsGravityGlovesDragging = false;

	// ==================== 重写 ====================
	
	virtual AGrabbeeObject* FindTarget_Implementation() override;
	virtual bool IsInBackpackArea_Implementation() const override;
	virtual void TryRelease(bool bToBackpack = false) override;

	// ==================== VR 专用接口 ====================
	
	/**
	 * 查找角度最近的可抓取物体（Gravity Gloves 用）
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	AGrabbeeObject* FindAngleClosestTarget();

	/**
	 * 开始 Gravity Gloves 拉取
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	void StartGravityGlovesPull(AGrabbeeObject* Target);

	/**
	 * 停止 Gravity Gloves 拉取
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|GravityGloves")
	void StopGravityGlovesPull();

protected:
	// ==================== 内部函数 ====================
	
	/** 更新 Gravity Gloves 逻辑 */
	void UpdateGravityGloves(float DeltaTime);

	/** 球形检测 */
	AGrabbeeObject* PerformSphereOverlap() const;

	/** 检查物体是否在 Gravity Gloves 角度范围内 */
	bool IsInGravityGlovesAngle(AGrabbeeObject* Target) const;
};

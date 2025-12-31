// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/GrabTypes.h"
#include "Grab/IGrabbable.h"
#include "GrabbeeObject.generated.h"

class UPlayerGrabHand;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * 可抓取物体基类
 * 
 * 所有可以被玩家抓取的物体都应继承此类。
 * 包含抓取类型、抓取状态、以及抓取/释放时的回调。
 */
UCLASS()
class VRTEST_API AGrabbeeObject : public AActor, public IGrabbable
{
	GENERATED_BODY()
	
public:	
	AGrabbeeObject();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 组件 ====================
	
	/** 根组件 - 静态网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// ==================== 抓取配置 ====================
	
	/** 抓取类型 - 决定抓取时的物理处理方式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	EGrabType GrabType = EGrabType::Free;

	/** 是否可以被抓取 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	bool bCanGrab = true;

	/** 是否支持双手同时抓取（仅 HumanBody 类型默认支持） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	bool bSupportsDualHandGrab = false;

	// ==================== 抓取状态 ====================
	
	/** 当前是否被抓取 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsHeld = false;

	/** 当前抓取此物体的手（主手，兼容旧逻辑） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	UPlayerGrabHand* HoldingHand = nullptr;

	/** 当前控制此物体的所有手（HumanBody 双手抓取用） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	TSet<UPlayerGrabHand*> ControllingHands;

	/** 当前是否被选中（Gravity Gloves） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsSelected = false;

	// ==================== IGrabbable 接口实现 ====================
	
	virtual EGrabType GetGrabType_Implementation() const override;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const override;
	virtual bool CanBeGrabbedBy_Implementation(const UPlayerGrabHand* Hand) const override;
	virtual bool SupportsDualHandGrab_Implementation() const override;
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand) override;
	virtual void OnGrabSelected_Implementation() override;
	virtual void OnGrabDeselected_Implementation() override;

	// ==================== 自有函数 ====================

	/**
	 * 向目标位置发射物体（Gravity Gloves 用）
	 * 使用抛物线轨迹计算速度并施加冲量
	 * @param TargetLocation 目标位置（通常是手的位置）
	 * @param ArcParam 抛物线弧度参数（0.0-1.0，越小越平）
	 * @return 是否成功施加冲量
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool LaunchTowards(const FVector& TargetLocation, float ArcParam = 0.5f);

	/**
	 * 设置物理模拟状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	void SetSimulatePhysics(bool bSimulate);

	/**
	 * 强制从当前手释放
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	void ForceRelease();
};

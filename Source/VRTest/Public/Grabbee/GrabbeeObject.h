// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/GrabTypes.h"
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
class VRTEST_API AGrabbeeObject : public AActor
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

	/** Snap 类型的目标相对变换（相对于手） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab", meta = (EditCondition = "GrabType == EGrabType::Snap"))
	FTransform SnapOffset;

	// ==================== 抓取状态 ====================
	
	/** 当前是否被抓取 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsHeld = false;

	/** 当前抓取此物体的手 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	UPlayerGrabHand* HoldingHand = nullptr;

	// ==================== 抓取接口 ====================
	
	/**
	 * 检查此物体是否可以被指定的手抓取
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	bool CanBeGrabbedBy(UPlayerGrabHand* Hand) const;
	virtual bool CanBeGrabbedBy_Implementation(UPlayerGrabHand* Hand) const;

	/**
	 * 当被抓取时调用
	 * @param Hand 抓取此物体的手
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnGrabbed(UPlayerGrabHand* Hand);
	virtual void OnGrabbed_Implementation(UPlayerGrabHand* Hand);

	/**
	 * 当被释放时调用
	 * @param Hand 释放此物体的手
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnReleased(UPlayerGrabHand* Hand);
	virtual void OnReleased_Implementation(UPlayerGrabHand* Hand);

	/**
	 * Custom 类型的自定义抓取逻辑
	 * 子类重写此函数实现特殊抓取行为
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Grab")
	void CustomGrab(UPlayerGrabHand* Hand);
	virtual void CustomGrab_Implementation(UPlayerGrabHand* Hand);

	/**
	 * Custom 类型的自定义释放逻辑
	 * 子类重写此函数实现特殊释放行为
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Grab")
	void CustomRelease(UPlayerGrabHand* Hand);
	virtual void CustomRelease_Implementation(UPlayerGrabHand* Hand);

	// ==================== 辅助函数 ====================
	
	/**
	 * 获取用于物理控制的 PrimitiveComponent
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	UPrimitiveComponent* GetGrabPrimitive() const;
	virtual UPrimitiveComponent* GetGrabPrimitive_Implementation() const;

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

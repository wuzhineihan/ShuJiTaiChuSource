// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Grabber/GrabTypes.h"
#include "IGrabbable.generated.h"

class UPlayerGrabHand;
class UPrimitiveComponent;

/**
 * 可抓取物体接口
 * 
 * 所有可以被玩家抓取的Actor都应实现此接口。
 * Grabber侧完全面向此接口编程，不需要知道具体实现类。
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UGrabbable : public UInterface
{
	GENERATED_BODY()
};

class VRTEST_API IGrabbable
{
	GENERATED_BODY()

public:
	// ==================== 查询方法 ====================
	
	/**
	 * 获取抓取类型
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	EGrabType GetGrabType() const;

	/**
	 * 获取用于物理控制的 PrimitiveComponent
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	UPrimitiveComponent* GetGrabPrimitive() const;

	/**
	 * 检查是否可以被指定的手抓取
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	bool CanBeGrabbedBy(const UPlayerGrabHand* Hand) const;

	/**
	 * 检查是否可以被重力手套抓取
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	bool CanBeGrabbedByGravityGlove() const;

	/**
	 * 是否支持双手同时抓取
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	bool SupportsDualHandGrab() const;

	// ==================== 状态回调 ====================
	
	/**
	 * 当被抓取时调用
	 * @param Hand 抓取此物体的手
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnGrabbed(UPlayerGrabHand* Hand);

	/**
	 * 当被释放时调用
	 * @param Hand 释放此物体的手
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnReleased(UPlayerGrabHand* Hand);

	/**
	 * 当被选中时调用（Gravity Gloves 瞄准）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnGrabSelected();

	/**
	 * 当取消选中时调用
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	void OnGrabDeselected();
};

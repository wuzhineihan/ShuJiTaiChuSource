// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Game/GrabTypes.h"
#include "PlayerGrabHand.generated.h"

class AGrabbeeObject;
class AGrabbeeWeapon;
class UInventoryComponent;
class UPhysicsControlComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectGrabbed, AGrabbeeObject*, GrabbedObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectReleased, AGrabbeeObject*, ReleasedObject);

/**
 * 玩家手部组件基类
 * 
 * 负责抓取逻辑，是 Grabber 侧的核心类。
 * PC 和 VR 各自继承此类实现不同的 FindTarget 和交互逻辑。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract)
class VRTEST_API UPlayerGrabHand : public USceneComponent
{
	GENERATED_BODY()

public:	
	UPlayerGrabHand();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 配置 ====================
	
	/** 是否是右手 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand")
	bool bIsRightHand = true;

	/** 另一只手的引用 */
	UPROPERTY(BlueprintReadWrite, Category = "Hand")
	UPlayerGrabHand* OtherHand = nullptr;

	/** 武器抓取偏移 - 每种武器类型对应一个相对变换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Weapon")
	TMap<EWeaponType, FTransform> WeaponGrabOffsets;

	// ==================== PhysicsControl 配置 ====================
	
	/** Free 类型的物理控制强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabStrength = 1.0f;

	/** Free 类型的物理控制阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabDamping = 1.0f;

	/** Snap 类型的物理控制强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float SnapGrabStrength = 2.0f;

	/** Snap 类型的物理控制阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float SnapGrabDamping = 1.5f;

	// ==================== 状态 ====================
	
	/** 当前持有的物体 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	AGrabbeeObject* HeldObject = nullptr;

	/** 当前是否持有物体 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsHolding = false;

	/** 当前 PhysicsControl 句柄名称 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	FName CurrentControlName;

	// ==================== 委托 ====================
	
	UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
	FOnObjectGrabbed OnObjectGrabbed;

	UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
	FOnObjectReleased OnObjectReleased;

	// ==================== 核心接口 ====================
	
	/**
	 * 尝试抓取
	 * @param bFromBackpack 是否从背包取物
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void TryGrab(bool bFromBackpack = false);

	/**
	 * 尝试释放
	 * @param bToBackpack 是否放入背包
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void TryRelease(bool bToBackpack = false);

	/**
	 * 查找抓取目标（子类实现）
	 * @return 找到的可抓取物体，或 nullptr
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	AGrabbeeObject* FindTarget();
	virtual AGrabbeeObject* FindTarget_Implementation();

	/**
	 * 检查是否在背包区域（VR 用）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Grab")
	bool IsInBackpackArea() const;
	virtual bool IsInBackpackArea_Implementation() const;

	// ==================== 抓取实现 ====================
	
	/**
	 * 执行抓取
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void GrabObject(AGrabbeeObject* Target);

	/**
	 * 执行释放
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void ReleaseObject();

	/**
	 * 仅释放附加，不触发 OnReleased 回调
	 * 用于武器模式切换等场景
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void ReleaseAttachOnly();

protected:
	// ==================== 内部实现 ====================
	
	/** Free 类型抓取 - 使用 PhysicsControl */
	virtual void GrabFree(AGrabbeeObject* Target);

	/** Snap 类型抓取 - 使用 PhysicsControl + 目标位置 */
	virtual void GrabSnap(AGrabbeeObject* Target);

	/** WeaponSnap 类型抓取 - 使用 Attach */
	virtual void GrabWeaponSnap(AGrabbeeWeapon* Weapon);

	/** HumanBody 类型抓取 - 使用 PhysicsControl 控制骨骼 */
	virtual void GrabHumanBody(AGrabbeeObject* Target);

	/** Custom 类型抓取 - 调用物体的自定义函数 */
	virtual void GrabCustom(AGrabbeeObject* Target);

	/** 释放 PhysicsControl */
	virtual void ReleasePhysicsControl();

	/** 释放 Attach (内部实现) */
	virtual void ReleaseAttach();

	// ==================== 辅助函数 ====================
	
	/** 获取 InventoryComponent */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	UInventoryComponent* GetInventoryComponent() const;

	/** 获取 PhysicsControlComponent */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	UPhysicsControlComponent* GetPhysicsControlComponent() const;

	/** 生成唯一的 PhysicsControl 名称 */
	FName GenerateControlName() const;

	/** 抓取前的验证 */
	virtual bool ValidateGrab(AGrabbeeObject* Target) const;

	/** 释放前的验证 */
	virtual bool ValidateRelease() const;

	/** 处理另一只手持有同一物体的情况 */
	virtual void HandleOtherHandHolding(AGrabbeeObject* Target);
};

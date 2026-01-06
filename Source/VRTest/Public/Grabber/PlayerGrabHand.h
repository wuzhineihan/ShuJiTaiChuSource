// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Grabber/GrabTypes.h"
#include "PlayerGrabHand.generated.h"

class IGrabbable;
class AGrabbeeWeapon;
class UInventoryComponent;
class UPhysicsHandleComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectGrabbed, AActor*, GrabbedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectReleased, AActor*, ReleasedActor);

/**
 * 玩家手部组件基类
 * 
 * 负责抓取逻辑，是 Grabber 侧的核心类。
 * 完全面向 IGrabbable 接口编程，不需要知道具体实现类。
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	/** 抓取检测对象类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	TArray<TEnumAsByte<EObjectTypeQuery>> GrabObjectTypes;

	// ==================== 组件 ====================
	
	/** 手部碰撞体 - 用于检测与弓弦的 overlap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HandCollision;

	// ==================== PhysicsHandle 配置 ====================
	
	/** Free 类型的物理控制强度（线性刚度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabLinearStiffness = 1500.0f;

	/** Free 类型的物理控制阻尼（线性阻尼） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabLinearDamping = 200.0f;

	/** Free 类型的角度刚度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabAngularStiffness = 1500.0f;

	/** Free 类型的角度阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float FreeGrabAngularDamping = 500.0f;

	/** WeaponSnap 类型的线性刚度（更高以保持武器紧跟手部） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float WeaponSnapLinearStiffness = 5000.0f;

	/** WeaponSnap 类型的线性阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float WeaponSnapLinearDamping = 500.0f;

	/** WeaponSnap 类型的角度刚度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float WeaponSnapAngularStiffness = 10000.0f;

	/** WeaponSnap 类型的角度阻尼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
	float WeaponSnapAngularDamping = 1000.0f;

	// ==================== 状态 ====================
	
	/** 当前持有的 Actor */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	AActor* HeldActor = nullptr;

	/** 当前持有物体的抓取类型（缓存，避免重复接口调用） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	EGrabType HeldGrabType = EGrabType::None;

	/** 当前是否持有物体 */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsHolding = false;


	/** 当前抓取的骨骼名（HumanBody 类型用） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	FName GrabbedBoneName;

	/** Free/Snap 抓取时记录的相对变换（物体相对于手） */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	FTransform GrabOffset;

	// ==================== 缓存组件 ====================

	/** 缓存的 PhysicsHandleComponent（从 BasePlayer 获取，根据 bIsRightHand 选择） */
	UPROPERTY()
	UPhysicsHandleComponent* CachedPhysicsHandle = nullptr;

	/** 缓存的 InventoryComponent（优化性能，避免重复查找） */
	UPROPERTY()
	UInventoryComponent* CachedInventory = nullptr;

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

	// ==================== 抓取实现 ====================
	
	/**
	 * 执行抓取（接受实现 IGrabbable 的 Actor）
	 * @param TargetActor 目标 Actor（必须实现 IGrabbable）
	 * @param BoneName 骨骼名（HumanBody 类型用，其他类型忽略）
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void GrabObject(AActor* TargetActor, FName BoneName = NAME_None);

	/**
	 * 执行释放
	 */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	virtual void ReleaseObject();

	/**
	 * 设置此手部使用的 PhysicsHandle（由 BasePlayer 在 BeginPlay 中调用）
	 */
	void SetPhysicsHandle(UPhysicsHandleComponent* InPhysicsHandle);

	/**
	 * 设置此手部使用的 Inventory（由 BasePlayer 在 BeginPlay 中调用）
	 */
	void SetInventory(UInventoryComponent* InInventory);

protected:
	// ==================== 目标查找 ====================
	
	/**
	 * 查找抓取目标（子类实现）
	 * @param bFromBackpack 是否从背包取物
	 * @param OutBoneName 输出参数：命中的骨骼名（如果目标是骨骼网格体）
	 * @return 找到的实现 IGrabbable 的 Actor，或 nullptr
	 */
	virtual AActor* FindTarget(bool bFromBackpack, FName& OutBoneName);

	// ==================== 内部实现 ====================
	
	/** 释放 PhysicsHandle */
	virtual void ReleasePhysicsHandle();

	// ==================== 辅助函数 ====================

	/** 处理另一只手持有同一物体的情况 仅在处理非双手抓取的物体时调用 支持双手抓取的物体不调用*/
	virtual void HandleOtherHandHolding(AActor* TargetActor, IGrabbable* Grabbable);
};

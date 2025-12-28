// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArrowCountChanged, int32, NewCount, int32, MaxCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBowStateChanged, bool, bInBackpack);

/**
 * 背包组件 - 管理角色的弓和箭
 * 
 * 设计原则：
 * - 背包只存储弓(1把)和箭(有上限)
 * - 物品是"虚拟存储"：放入时销毁Actor，取出时重新生成
 * - 提供纯数据接口，不处理VR/PC的具体交互逻辑
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRTEST_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 配置 ====================
	
	/** 箭矢最大数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxArrowCount = 3;

	/** 弓的类 - 用于生成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSubclassOf<AActor> BowClass;

	/** 箭的类 - 用于生成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSubclassOf<AActor> ArrowClass;

	// ==================== 状态 ====================
	
	/** 弓是否在背包中 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bBowInBackpack = false;

	/** 当前箭矢数量 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 ArrowCount = 0;

	// ==================== 委托 ====================
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnArrowCountChanged OnArrowCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBowStateChanged OnBowStateChanged;

	// ==================== 弓操作 ====================
	
	/**
	 * 尝试将弓放入背包
	 * @return true 如果成功放入（背包里原来没有弓）
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	bool TryStoreBow();

	/**
	 * 尝试从背包取出弓
	 * @param SpawnTransform 生成位置
	 * @return 生成的弓Actor，如果背包里没有弓则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	AActor* TryRetrieveBow(const FTransform& SpawnTransform);

	/**
	 * 检查背包里是否有弓
	 */
	UFUNCTION(BlueprintPure, Category = "Bow")
	bool HasBow() const { return bBowInBackpack; }

	// ==================== 箭操作 ====================
	
	/**
	 * 尝试将箭放入背包
	 * @return true 如果成功放入（未达到上限）
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	bool TryStoreArrow();

	/**
	 * 尝试从背包取出箭
	 * @param SpawnTransform 生成位置
	 * @return 生成的箭Actor，如果背包里没有箭则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	AActor* TryRetrieveArrow(const FTransform& SpawnTransform);

	/**
	 * 检查背包里是否有箭
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	bool HasArrow() const { return ArrowCount > 0; }

	/**
	 * 检查箭是否已满
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	bool IsArrowFull() const { return ArrowCount >= MaxArrowCount; }

	/**
	 * 获取箭数量
	 */
	UFUNCTION(BlueprintPure, Category = "Arrow")
	int32 GetArrowCount() const { return ArrowCount; }

	// ==================== 通用操作 ====================
	
	/**
	 * 检查背包是否有物品可取
	 * @param OutIsBow 返回可取物品是否为弓
	 * @return true 如果有物品可取
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItemToRetrieve(bool& OutIsBow) const;

	/**
	 * 从背包取出物品（优先弓）
	 * @param SpawnTransform 生成位置
	 * @return 生成的Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	AActor* RetrieveItem(const FTransform& SpawnTransform);

	/**
	 * 尝试将物品放入背包
	 * @param Item 要放入的物品Actor
	 * @return true 如果成功放入
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryStoreItem(AActor* Item);

	// ==================== 直接设置（用于存档/初始化） ====================
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetBowState(bool bInBackpack);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetArrowCount(int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddArrows(int32 Amount);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameSettings.generated.h"

class ABow;
class AArrow;

/**
 * 游戏全局设置
 * 
 * 在 Project Settings -> Game -> Game Settings 中配置
 * 用于统一管理蓝图类的引用，避免在多个地方重复配置
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Game Settings"))
class VRTEST_API UGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameSettings();

	/** 获取单例实例 */
	static UGameSettings* Get();

	// ==================== 弓箭相关 ====================
	
	/** 弓的蓝图类 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bow")
	TSoftClassPtr<ABow> BowClass;

	/** 箭的蓝图类 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Bow")
	TSoftClassPtr<AArrow> ArrowClass;

	// ==================== 辅助函数 ====================
	
	/** 获取弓的类（同步加载） */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	TSubclassOf<ABow> GetBowClass() const;

	/** 获取箭的类（同步加载） */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	TSubclassOf<AArrow> GetArrowClass() const;
};


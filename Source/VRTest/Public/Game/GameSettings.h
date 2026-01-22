// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameSettings.generated.h"

class ABow;
class AArrow;
class USkillAsset;
class UMaterialInterface;

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

	// ==================== StarDraw 相关 ====================

	/** 技能总资产：包含 StarDraw 的轨迹映射 + FingerPoint/MainStar/OtherStar 蓝图类 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	TSoftObjectPtr<USkillAsset> SkillAsset;

	/** 玩家相机后处理材质（Post Process Material） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Player|PostProcess")
	TSoftObjectPtr<UMaterialInterface> PlayerCameraPostProcessMaterial;

	/** 获取 SkillAsset（同步加载）。未配置则返回 nullptr。 */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	USkillAsset* GetSkillAsset() const;

	/** 获取玩家相机后处理材质（同步加载）。未配置则返回 nullptr。 */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	UMaterialInterface* GetPlayerCameraPostProcessMaterial() const;

	// ==================== 辅助函数 ====================
	
	/** 获取弓的类（同步加载） */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	TSubclassOf<ABow> GetBowClass() const;

	/** 获取箭的类（同步加载） */
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	TSubclassOf<AArrow> GetArrowClass() const;
};

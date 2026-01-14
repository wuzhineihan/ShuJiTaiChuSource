// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Skill/SkillTypes.h"
#include "SkillAsset.generated.h"

class AStarDrawFingerPoint;
class AStarDrawMainStar;
class AStarDrawOtherStar;
class ASkillStrategyBase;

/**
 * SkillAsset：技能系统的统一配置资产。
 *
 * - StarDraw 识别层：轨迹序列字符串 -> 技能类型
 * - StarDraw 表现层：FingerPoint/MainStar/OtherStar 的蓝图类
 */
UCLASS(BlueprintType)
class VRTEST_API USkillAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==================== 识别配置 ====================

	/** 蓝图配置：轨迹方向序列 -> 技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	TArray<FStarDrawTrailPair> StarDrawTrailPairs;

	/**
	 * 运行时缓存：轨迹序列字符串 -> 技能类型。例："0247" -> EagleEye。
	 * 不对蓝图/编辑器开放配置，由 StarDrawTrailPairs 预处理生成。
	 */
	UPROPERTY(Transient)
	TMap<FString, ESkillType> TrailToSkill;

	/** 按方向序列查询对应技能；非法/找不到返回 None。 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|StarDraw")
	ESkillType GetSkillTypeFromTrail(const TArray<EStarDrawDirection>& Trail) const;

	// ==================== 表现配置 ====================

	/** FingerPoint 蓝图类（用于自定义模型/特效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Visual")
	TSubclassOf<AStarDrawFingerPoint> FingerPointClass;

	/** MainStar 蓝图类（用于自定义模型/特效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Visual")
	TSubclassOf<AStarDrawMainStar> MainStarClass;

	/** OtherStar 蓝图类（用于自定义模型/特效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Visual")
	TSubclassOf<AStarDrawOtherStar> OtherStarClass;

	// ==================== 策略配置 ====================

	/** SkillType -> StrategyClass（用于技能释放逻辑的策略模式） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Strategy")
	TMap<ESkillType, TSubclassOf<ASkillStrategyBase>> StrategyClassMap;

protected:
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void RebuildTrailToSkillCache();
	FString BuildTrailKey(const TArray<EStarDrawDirection>& Trail) const;
};

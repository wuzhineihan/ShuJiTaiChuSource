// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/StarDrawDirection.h"
#include "SkillTypes.generated.h"

/**
 * 技能类型枚举（框架层）。
 *
 * 说明：此处仅收敛“可被系统识别/路由”的技能类型。
 * 具体技能实现由策略类（ASkillStrategyBase 的子类）承载。
 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	None UMETA(DisplayName = "None"),
	// 预留：后续可按项目需要添加
	EagleEye UMETA(DisplayName = "EagleEye"),
	Shield UMETA(DisplayName = "Shield"),
	Invisible UMETA(DisplayName = "Invisible"),
	Freeze UMETA(DisplayName = "Freeze")
};

/**
 * 技能执行上下文。
 *
 * 目前框架仅要求最小信息；后续可加入：命中点、方向、手别、持续时间、配置 DataAsset 等。
 */
USTRUCT(BlueprintType)
struct FSkillContext
{
	GENERATED_BODY()

	/** 触发该次技能的输入源（PC=Camera，VR=Hand） */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USceneComponent> InputSource = nullptr;

	/** 是否右手作为语义上的“绘制手/施法手” */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRightHand = true;
};

/**
 * StarDraw 轨迹到技能的配置项（用于蓝图直观配置）。
 * 运行时会预处理成字符串 Key（例如 "0247"）以便快速查找。
 */
USTRUCT(BlueprintType)
struct FStarDrawTrailPair
{
	GENERATED_BODY()

	/** 轨迹方向序列（按触碰顺序） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	TArray<EStarDrawDirection> Trail;

	/** 该轨迹对应的技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw")
	ESkillType Skill = ESkillType::None;
};

/**
 * Skill 系统内使用的 Actor Tag / Name 常量。
 * 约定：Tag 的字符串一律使用全小写 + 下划线（lower_snake_case）。
 */
namespace SkillTags
{
	/** StarDrawOtherStar 使用的 Actor Tag */
	inline const FName OtherStars(TEXT("other_stars"));
}

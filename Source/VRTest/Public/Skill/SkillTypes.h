// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTypes.generated.h"

/**
 * 技能类型枚举（框架层）。
 *
 * 说明：此处仅收敛“可被系统识别/路由”的技能类型。
 * 具体技能实现由策略类（USkillStrategyBase 的子类）承载。
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


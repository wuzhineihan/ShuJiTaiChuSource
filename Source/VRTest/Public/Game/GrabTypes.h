// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabTypes.generated.h"

/**
 * 抓取类型枚举
 */
UENUM(BlueprintType)
enum class EGrabType : uint8
{
	None,        // 不可抓取或抓取被禁用
	Free,        // 自由抓取，物理跟随手部（PhysicsControl）
	WeaponSnap,  // 武器吸附，使用 PhysicsControl 对齐到武器偏移
	HumanBody,   // 人体拖拽，控制指定骨骼（PhysicsControl）
	Custom       // 完全自定义，由子类重写 OnGrab/OnRelease
};

/**
 * 武器类型枚举
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Bow,
	Arrow
};

/**
 * 箭的状态枚举
 */
UENUM(BlueprintType)
enum class EArrowState : uint8
{
	Idle,      // 闲置（可抓取）
	Nocked,    // 搭在弓弦上
	Flying,    // 飞行中
	Stuck      // 插在目标上
};

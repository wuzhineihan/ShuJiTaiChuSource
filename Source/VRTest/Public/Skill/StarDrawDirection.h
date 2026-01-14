// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarDrawDirection.generated.h"

/**
 * 星图绘制方向（对���旧蓝图 OtherStars.DirectionFlag）。
 *
 * 说明：蓝图里使用 int 作为 DirectionFlag。
 * C++ 里用枚举更安全，同时仍可通过 int 编码用于序列匹配。
 */
UENUM(BlueprintType)
enum class EStarDrawDirection : uint8
{
	Left = 0 UMETA(DisplayName = "Left"),
	Right = 1 UMETA(DisplayName = "Right"),
	Up = 2 UMETA(DisplayName = "Up"),
	Down = 3 UMETA(DisplayName = "Down"),
	LeftUp = 4 UMETA(DisplayName = "LeftUp"),
	RightUp = 5 UMETA(DisplayName = "RightUp"),
	LeftDown = 6 UMETA(DisplayName = "LeftDown"),
	RightDown = 7 UMETA(DisplayName = "RightDown")
};

FORCEINLINE int32 StarDrawDirectionToInt(EStarDrawDirection Dir)
{
	return static_cast<int32>(Dir);
}

FORCEINLINE EStarDrawDirection StarDrawDirectionGetOpposite(EStarDrawDirection Dir)
{
	switch (Dir)
	{
	case EStarDrawDirection::Left: return EStarDrawDirection::Right;
	case EStarDrawDirection::Right: return EStarDrawDirection::Left;
	case EStarDrawDirection::Up: return EStarDrawDirection::Down;
	case EStarDrawDirection::Down: return EStarDrawDirection::Up;
	case EStarDrawDirection::LeftUp: return EStarDrawDirection::RightDown;
	case EStarDrawDirection::RightDown: return EStarDrawDirection::LeftUp;
	case EStarDrawDirection::RightUp: return EStarDrawDirection::LeftDown;
	case EStarDrawDirection::LeftDown: return EStarDrawDirection::RightUp;
	default: return Dir;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Skill/SkillTypes.h"
#include "ShujiSaveGame.generated.h"

/**
 * 蜀祭太初 存档数据
 *
 * 单存档槽位，存储在默认 save slot 中。
 */
UCLASS()
class VRTEST_API UShujiSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ==================== 玩家位置 ====================

	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator PlayerRotation = FRotator::ZeroRotator;

	// ==================== 玩家技能 ====================

	UPROPERTY()
	bool bStarDrawEnabled = false;

	UPROPERTY()
	TSet<ESkillType> LearnedSkills;

	UPROPERTY()
	int32 CurrentEnergyPoints = 0;

	// ==================== 玩家弓箭 ====================

	UPROPERTY()
	bool bHasBow = false;

	UPROPERTY()
	int32 ArrowCount = 0;

	// ==================== 敌人状态 ====================

	UPROPERTY()
	TSet<FName> DeadEnemyIDs;
};

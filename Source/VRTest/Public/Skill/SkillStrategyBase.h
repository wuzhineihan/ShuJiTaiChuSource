// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Skill/SkillTypes.h"
#include "SkillStrategyBase.generated.h"

class ABasePlayer;

/**
 * 技能策略基类：每个具体技��实现一个策略类。
 *
 * 框架约束：策略只关心“如何执行技能”，不处理技能学习状态，也不管理绘制会话生命周期。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class VRTEST_API USkillStrategyBase : public UObject
{
	GENERATED_BODY()

public:
	/** 执行技能。返回值表示是否成功触发（可以用于日志/调试/UI）。 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Skill")
	bool Execute(ABasePlayer* Player, const FSkillContext& Context);
	virtual bool Execute_Implementation(ABasePlayer* Player, const FSkillContext& Context) { return false; }
};


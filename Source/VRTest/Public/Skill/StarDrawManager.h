// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill/SkillTypes.h"
#include "StarDrawManager.generated.h"

class AStarDrawFingerPoint;
class AStarDrawMainStar;
class AStarDrawOtherStar;
class USkillAsset;

/**
 * 星图绘制管理器（Actor）。
 *
 * 职责：管理星图绘制的生命周期，并在结束时输出识别到的技能类型。
 * 说明：本类只负责绘制/识别，不负责判断技能是否已学习，也不直接触发技能效果。
 */
UCLASS(Blueprintable)
class VRTEST_API AStarDrawManager : public AActor
{
	GENERATED_BODY()

public:
	AStarDrawManager();

	/** 开始绘制。InputSource：PC=Camera，VR=Hand。 */
	UFUNCTION(BlueprintCallable, Category = "Skill|StarDraw")
	virtual void StartDraw(USceneComponent* InInputSource);

	/** 结束绘制并返回识别结果。未识别/无效则返回 ESkillType::None�� */
	UFUNCTION(BlueprintCallable, Category = "Skill|StarDraw")
	virtual ESkillType FinishDraw();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skill|StarDraw")
	bool IsDrawing() const { return bIsDrawing; }

	/**
	 * 由 FingerPoint 回调：当 FingerPoint 触碰到某个 Actor（通常是 OtherStar）时通知 DrawManager。
	 *
	 * 说明：这里不直接依赖碰撞通道/Trace，以便 VR/PC 使用同一套回调机制。
	 */
	void NotifyFingerTouchActor(AActor* TouchedActor);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void FirstTouch();
	void TouchOtherStar(AStarDrawOtherStar* OtherStar);

	void SpawnFingerPoint();
	void SpawnMainStarAt(const FVector& Location);
	void SpawnOtherStarsAround(const FVector& CenterLocation);
	void ClearOtherStars();
	void ClearMainStars();

	FVector GetPlayerPivotLocation() const;
	FVector GetInputSourceLocation() const;
	FVector GetInputSourceForwardVector() const;

	void UpdateFingerPointLocation(float DeltaSeconds);

	/**
	 * 计算“圆柱面上的邻接点”——替代旧蓝图函数库。
	 *
	 * 设计：以 PlayerPivot 为圆柱轴心，半径为 Radius，围绕 CenterLocation 生成 8 个邻接点。
	 * - 左/右：绕 Z 轴旋转
	 * - 上/下：沿 Z 轴移动
	 * - 左上/右上/左下/右下：组合
	 */
	TMap<EStarDrawDirection, FVector> CalculateAdjacentPointsOnCylinder(const FVector& PlayerPivot, const FVector& CenterLocation, float InLineLength) const;

	float CalculateRadiusOnXY(const FVector& PlayerPivot, const FVector& ControllerLocation, const FVector& Forward, float Distance) const;
	FVector CalculateProjectionPointOnCylinder(const FVector& PlayerPivot, const FVector& ControllerLocation, const FVector& Forward, float Radius) const;

	virtual ESkillType ResolveSkillFromDraw() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw")
	bool bIsDrawing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw")
	TObjectPtr<USceneComponent> InputSource;

	/** 最近一次识别到的技能类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw")
	ESkillType CachedResult = ESkillType::None;

	// ==================== Draw State ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Config")
	float Distance = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Config")
	float LineLength = 30.f;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw|State")
	float Radius = 0.f;

	/** 玩家轴心位置（旧蓝图 TriggerCameraLocation） */
	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw|State")
	FVector PlayerPivotLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw|State")
	FVector FingerPointLocation = FVector::ZeroVector;

	/** 轨迹方向数组（旧蓝图 DirectionChoose） */
	UPROPERTY(BlueprintReadOnly, Category = "Skill|StarDraw|State")
	TArray<EStarDrawDirection> DirectionChoose;


	UPROPERTY(EditDefaultsOnly, Category = "Skill|StarDraw|Spawn")
	TSubclassOf<AStarDrawFingerPoint> FingerPointClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|StarDraw|Spawn")
	TSubclassOf<AStarDrawMainStar> MainStarClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|StarDraw|Spawn")
	TSubclassOf<AStarDrawOtherStar> OtherStarClass;

	UPROPERTY(Transient)
	TObjectPtr<AStarDrawFingerPoint> FingerPoint;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AStarDrawMainStar>> MainStars;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AStarDrawOtherStar>> OtherStars;

	/** 缓存：技能总资产（BeginPlay 从 GameSettings 加载一次） */
	UPROPERTY(Transient)
	TObjectPtr<USkillAsset> CachedSkillAsset;

	/** 是否启用 Debug 绘制 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|StarDraw|Debug")
	bool bDebugDraw = false;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/DataAsset/SacraEnemyConfigDataAsset.h"

#include "SacraEnemyContextComponent.generated.h"

class AAIController;
class UBlackboardComponent;
class UEnemyPatrolSplineComponent;
class USacraBlackboardComponent;
class USacraEnemyHatredComponent;
class USacraEnemyWeaponComponent;

UCLASS(ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class VRTEST_API USacraEnemyContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USacraEnemyContextComponent();

	virtual void BeginPlay() override;

	// ==================== Shared References ====================

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	AAIController* GetCachedAIController() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	USacraBlackboardComponent* GetCachedSacraBlackboardComponent() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	UBlackboardComponent* GetCachedBlackboardComponent() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	USacraEnemyHatredComponent* GetCachedHatredComponent() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	USacraEnemyWeaponComponent* GetCachedWeaponComponent() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Shared")
	UEnemyPatrolSplineComponent* GetPatrolSplineComponent() const;

	// ==================== Idle Context ====================

	UFUNCTION(BlueprintPure, Category = "AI|Context|Idle")
	FVector GetStandLocation() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Idle")
	FRotator GetStandRotation() const;

	UFUNCTION(BlueprintPure, Category = "AI|Context|Idle")
	float GetIdleMoveSpeed() const { return IdleMoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Idle")
	float GetPatrolMoveSpeed() const { return PatrolMoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Idle")
	bool HasPatrolRoute() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Idle")
	bool TryGetCurrentPatrolPoint(FVector& OutPatrolPointLocation) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Idle")
	bool AdvancePatrolPoint();

	// ==================== Warning Context ====================

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	float GetWarningMoveSpeed() const { return WarningMoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	float GetWarningSearchRadius() const { return WarningSearchRadius; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	int32 GetWarningSearchPointCount() const { return WarningSearchPointCount; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	float GetWarningSearchReachableRadius() const { return WarningSearchReachableRadius; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	bool IsWarningSupportRequestEnabled() const { return bEnableWarningSupportRequest; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	float GetWarningSupportRequestRadius() const { return WarningSupportRequestRadius; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	bool HasCachedWarningSearchLocation() const { return bHasCachedWarningSearchLocation; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	FVector GetCachedWarningSearchLocation() const { return CachedWarningSearchLocation; }

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Warning")
	void SetCachedWarningSearchLocation(const FVector& InSearchLocation);

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Warning")
	void ClearCachedWarningSearchLocation();

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	bool HasCachedWarningAnchorLocation() const { return bHasCachedWarningAnchorLocation; }

	UFUNCTION(BlueprintPure, Category = "AI|Context|Warning")
	FVector GetCachedWarningAnchorLocation() const { return CachedWarningAnchorLocation; }

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Warning")
	void SetCachedWarningAnchorLocation(const FVector& InAnchorLocation);

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Warning")
	void ClearCachedWarningAnchorLocation();

	UFUNCTION(BlueprintCallable, Category = "AI|Context|Config")
	void ApplyConfigData(const FSacraEnemyContextConfig& ConfigData);

protected:
	// ==================== Shared Config ====================

	// 允许组件在 BeginPlay 时自动解析控制器、黑板、仇恨组件等公共引用。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Shared")
	bool bAutoResolveSharedReferences = true;

	// ==================== Idle Config ====================

	// 默认使用敌人初始落点作为 Idle 站立位置。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle")
	bool bUseSpawnTransformAsStandTransform = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle", meta = (EditCondition = "!bUseSpawnTransformAsStandTransform"))
	FVector StandLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle", meta = (EditCondition = "!bUseSpawnTransformAsStandTransform"))
	FRotator StandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle")
	bool bEnablePatrol = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle", meta = (ClampMin = "0.0"))
	float IdleMoveSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Idle", meta = (ClampMin = "0.0"))
	float PatrolMoveSpeed = 250.0f;

	// ==================== Warning Config ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning", meta = (ClampMin = "0.0"))
	float WarningMoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning", meta = (ClampMin = "0.0"))
	float WarningSearchRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning", meta = (ClampMin = "1"))
	int32 WarningSearchPointCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning", meta = (ClampMin = "0.0"))
	float WarningSearchReachableRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning")
	bool bEnableWarningSupportRequest = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Context|Warning", meta = (ClampMin = "0.0", EditCondition = "bEnableWarningSupportRequest"))
	float WarningSupportRequestRadius = 3000.0f;

private:
	// ==================== Internal Helpers ====================

	void CacheSpawnTransform();
	void ResolveSharedReferences();
	void ResolveAIController();
	void ResolveBlackboardComponent();
	void ResolveHatredComponent();
	void ResolveWeaponComponent();
	void ResolvePatrolSplineComponent();

private:
	// ==================== Runtime ====================

	UPROPERTY(Transient)
	FVector CachedSpawnLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator CachedSpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> CachedAIController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraBlackboardComponent> CachedSacraBlackboardComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBlackboardComponent> CachedBlackboardComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyHatredComponent> CachedHatredComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USacraEnemyWeaponComponent> CachedWeaponComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UEnemyPatrolSplineComponent> CachedPatrolSplineComponent = nullptr;

	UPROPERTY(Transient)
	bool bHasCachedWarningSearchLocation = false;

	UPROPERTY(Transient)
	FVector CachedWarningSearchLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasCachedWarningAnchorLocation = false;

	UPROPERTY(Transient)
	FVector CachedWarningAnchorLocation = FVector::ZeroVector;
};

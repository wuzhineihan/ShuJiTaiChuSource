// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SacraEnemyConfigDataAsset.generated.h"

class UBehaviorTree;
class UUserWidget;
class USacraEnemyHatredDataAsset;
class USacraEnemyLoadoutDataAsset;
class USacraEnemyWeaponComponent;

USTRUCT(BlueprintType)
struct FSacraEnemyGeneralConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	bool bOverrideWeaponComponentClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", meta = (EditCondition = "bOverrideWeaponComponentClass"))
	TSubclassOf<USacraEnemyWeaponComponent> EnemyWeaponComponentClass;
};

USTRUCT(BlueprintType)
struct FSacraEnemyLoadoutConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	bool bOverrideLoadoutConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (EditCondition = "bOverrideLoadoutConfig"))
	bool bAutoInitializeLoadoutOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (EditCondition = "bOverrideLoadoutConfig"))
	TObjectPtr<USacraEnemyLoadoutDataAsset> LoadoutDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (EditCondition = "bOverrideLoadoutConfig"))
	bool bRandomizeAppearance = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (EditCondition = "bOverrideLoadoutConfig", ClampMin = "0"))
	int32 AppearanceIndex = 0;
};

USTRUCT(BlueprintType)
struct FSacraEnemyHatredConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred")
	bool bOverrideHatredConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hatred", meta = (EditCondition = "bOverrideHatredConfig"))
	TObjectPtr<USacraEnemyHatredDataAsset> HatredConfigAsset = nullptr;
};

USTRUCT(BlueprintType)
struct FSacraEnemyContextConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
	bool bOverrideContextConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	bool bAutoResolveSharedReferences = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	bool bUseSpawnTransformAsStandTransform = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	FVector StandLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	FRotator StandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	bool bEnablePatrol = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float IdleMoveSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float PatrolMoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float WarningMoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float WarningSearchRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "1"))
	int32 WarningSearchPointCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float WarningSearchReachableRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig"))
	bool bEnableWarningSupportRequest = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context", meta = (EditCondition = "bOverrideContextConfig", ClampMin = "0.0"))
	float WarningSupportRequestRadius = 3000.0f;
};

USTRUCT(BlueprintType)
struct FSacraEnemyStatusUIConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI")
	bool bOverrideStatusUIConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI", meta = (EditCondition = "bOverrideStatusUIConfig"))
	TSubclassOf<UUserWidget> StatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI", meta = (EditCondition = "bOverrideStatusUIConfig"))
	FVector WidgetRelativeLocation = FVector(0.0f, 0.0f, 110.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI", meta = (EditCondition = "bOverrideStatusUIConfig"))
	FVector2D DrawSize = FVector2D(160.0f, 48.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI", meta = (EditCondition = "bOverrideStatusUIConfig"))
	bool bHideWhenIdle = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusUI", meta = (EditCondition = "bOverrideStatusUIConfig", ClampMin = "0.0"))
	float MaxVisibleDistance = 3000.0f;
};

USTRUCT(BlueprintType)
struct FSacraEnemyControllerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controller")
	bool bOverrideControllerConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controller", meta = (EditCondition = "bOverrideControllerConfig"))
	TObjectPtr<UBehaviorTree> DefaultBehaviorTreeAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controller", meta = (EditCondition = "bOverrideControllerConfig", ClampMin = "0.0"))
	float NonFightRotationRateYaw = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controller", meta = (EditCondition = "bOverrideControllerConfig", ClampMin = "0.0"))
	float FightRotationRateYaw = 540.0f;
};

UCLASS(BlueprintType)
class VRTEST_API USacraEnemyConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyGeneralConfig GeneralConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyLoadoutConfig LoadoutConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyHatredConfig HatredConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyContextConfig ContextConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyStatusUIConfig StatusUIConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Config")
	FSacraEnemyControllerConfig ControllerConfig;
};

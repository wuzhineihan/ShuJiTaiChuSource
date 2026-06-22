// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/BaseEnemy.h"

#include "SacraEnemy.generated.h"

class USacraEnemyContextComponent;
class USacraEnemyActivityComponent;
class USacraEnemyConfigDataAsset;
class USacraEnemyLoadoutComponent;
class USacraEnemyStatusUIComponent;
class USacraEnemyWeaponComponent;

UCLASS()
class VRTEST_API ASacraEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	ASacraEnemy();
	virtual void PostInitializeComponents() override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "AI|Loadout")
	bool RebuildEnemyLoadoutInEditor();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "AI|Loadout")
	bool RerollEnemyLoadoutInEditor();

	UFUNCTION(BlueprintPure, Category = "AI|Config")
	USacraEnemyConfigDataAsset* GetEnemyConfigDataAsset() const { return EnemyConfigDataAsset; }

	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void ApplyEnemyConfigDataAsset();

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	USacraEnemyWeaponComponent* GetEnemyWeaponComponent() const { return EnemyWeaponComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	USacraEnemyActivityComponent* GetEnemyActivityComponent() const { return EnemyActivityComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	USacraEnemyLoadoutComponent* GetEnemyLoadoutComponent() const { return EnemyLoadoutComponent; }

	// ==================== 存档 ====================

	/** 敌人唯一标识（在蓝图中为每个 NPC 设置，用于存档/读档匹配） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SaveGame")
	FName EnemyID;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyConfigDataAsset> EnemyConfigDataAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<USacraEnemyWeaponComponent> EnemyWeaponComponentClass;

private:
	void EnsureWeaponComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyContextComponent> EnemyContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyActivityComponent> EnemyActivityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyStatusUIComponent> EnemyStatusUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyLoadoutComponent> EnemyLoadoutComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "AI|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyWeaponComponent> EnemyWeaponComponent;
};

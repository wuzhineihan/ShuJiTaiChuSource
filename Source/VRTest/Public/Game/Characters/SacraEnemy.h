// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/BaseEnemy.h"

#include "SacraEnemy.generated.h"

class USacraEnemyContextComponent;
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

	UFUNCTION(BlueprintPure, Category = "AI|Weapon")
	USacraEnemyWeaponComponent* GetEnemyWeaponComponent() const { return EnemyWeaponComponent; }

	UFUNCTION(BlueprintPure, Category = "AI|Loadout")
	USacraEnemyLoadoutComponent* GetEnemyLoadoutComponent() const { return EnemyLoadoutComponent; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<USacraEnemyWeaponComponent> EnemyWeaponComponentClass;

private:
	void EnsureWeaponComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyContextComponent> EnemyContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyStatusUIComponent> EnemyStatusUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyLoadoutComponent> EnemyLoadoutComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "AI|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USacraEnemyWeaponComponent> EnemyWeaponComponent;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyHorseBase.generated.h"

class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeadSignature, AActor*, EnemyHorse);

UCLASS()
class VRTEST_API AEnemyHorseBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyHorseBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnEnemyDeadSignature OnEnemyDead;

	UFUNCTION(BlueprintCallable, Category="HorseEnemy")
	virtual void SetChasePoint(USceneComponent* InChasePoint);

	UFUNCTION(BlueprintPure, Category="HorseEnemy")
	USceneComponent* GetChasePoint() const { return ChasePoint.Get(); }

	UFUNCTION(BlueprintCallable, Category="HorseEnemy")
	void SetOver(bool bInOver) { bOver = bInOver; }

	UFUNCTION(BlueprintPure, Category="HorseEnemy")
	bool IsOver() const { return bOver; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="HorseEnemy")
	TObjectPtr<USceneComponent> ChasePoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HorseEnemy")
	bool bOver = false;

};

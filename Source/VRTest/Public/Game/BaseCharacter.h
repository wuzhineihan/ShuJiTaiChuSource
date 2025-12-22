// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Effect/Effectable.h"
#include "Effect/AliveComponent.h"
#include "BaseCharacter.generated.h"

UCLASS()
class VRTEST_API ABaseCharacter : public ACharacter, public IEffectable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAliveComponent* AliveComponent;

	// IEffectable Interface
	virtual void ApplyEffect_Implementation(const FEffect& Effect) override;

	// Effect Handlers
	virtual void TakeArrowEffect(const FEffect& Effect);
	virtual void TakeSmashEffect(const FEffect& Effect);
	virtual void TakeMeleeEffect(const FEffect& Effect);
	virtual void TakeFireEffect(const FEffect& Effect);
	virtual void TakeStasisEffect(const FEffect& Effect);

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual USceneComponent* GetTrackOrigin() const;
};

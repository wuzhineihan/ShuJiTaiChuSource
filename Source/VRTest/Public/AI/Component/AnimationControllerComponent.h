// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AI/EventPayloads/AnimPayloads.h"
#include "Components/ActorComponent.h"
#include "AI/Component/EventBusComponent.h"
#include "AnimationControllerComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAnimationControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAnimationControllerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//核心接口
	UFUNCTION(BlueprintCallable,Category = "AnimationController")//播放蒙太奇，监听事件总线回调，也可被外界调用
	void PlayMontage(FGameplayTag Tag, UObject* Payload);

	UFUNCTION(BlueprintCallable,Category = "AnimationController")//停止当前播放的蒙太奇，可被外界调用
	void StopCurrentMontage();


	//查询相关数值
	UFUNCTION(BlueprintCallable,Category = "AnimationController")
	bool IsPlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintCallable,Category = "AnimationController")
	FGameplayTag GetCurrentActionTag() const { return CurrentActionTag; }

	UFUNCTION(BlueprintCallable,Category = "AnimationController")
	float GetCurrentMontagePosition() const;
	
	
private:
	FGameplayTag CurrentActionTag;

	FGuid CurrentRequestId;

	bool bIsPlaying = false;

	FDelegateHandle CurrentMontageDelegateHandle;

	UEventBusComponent* EventBusComponent = nullptr;

	UAnimInstance* AnimInstance = nullptr;

	//回调和处理函数
	
	UFUNCTION()//动画蒙太奇结束回调，转发到真正执行结束的函数
	void OnMontageEnded_CallBack(UAnimMontage* Montage,bool bInterrupted);

	UFUNCTION()//处理动画蒙太奇结束并广播结果
	void HandleMontageEndedAndBroadcastResult(bool bInterrupted);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "AnimPayloads.generated.h"

class UAnimMontage;
/**
 * 
 */
UENUM(BlueprintType)
enum class EAnimCompletionResult : uint8
{
	Success, 		//完整播放完毕
	Interrupted,	//中途被打断
	BlendOut,		//混合退出
	Cancelled		//取消
};

UCLASS(BlueprintType)
class VRTEST_API UAnimRequestPayload : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category="Request")
	UAnimMontage* Montage  = nullptr; 

	UPROPERTY(BlueprintReadWrite, Category="Request")
	FGuid RequestId;

	UPROPERTY(BlueprintReadWrite, Category="Request")
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadWrite, Category="Request")
	float PlayRate = 1.f;

	UPROPERTY(BlueprintReadWrite, Category="Request")
	FName StartSectionName = NAME_None;

	static UAnimRequestPayload* Create(UAnimMontage* InMontage, const FGuid& InRequestId, const FGameplayTag& InActionTag, float InPlayRate = 1.f, const FName& InStartSectionName = NAME_None)
	{
		UAnimRequestPayload* Payload = NewObject<UAnimRequestPayload>();
		Payload->Montage = InMontage;
		Payload->RequestId = InRequestId;
		Payload->ActionTag = InActionTag;
		Payload->PlayRate = InPlayRate;
		Payload->StartSectionName = InStartSectionName;
		return Payload;
	}

};


UCLASS(BlueprintType)
class VRTEST_API UAnimResultPayload : public UObject
{
	GENERATED_BODY()
	
	public:
	UPROPERTY(BlueprintReadWrite, Category="Result")
	FGuid RequestId;

	UPROPERTY(BlueprintReadWrite, Category="Result")
	EAnimCompletionResult CompletionResult;

	UPROPERTY(BlueprintReadWrite, Category="Result")
	FGameplayTag ActionTag;

	static UAnimResultPayload* Create(const FGuid& InRequestId, EAnimCompletionResult InCompletionResult, const FGameplayTag& InActionTag)
	{
		UAnimResultPayload* Payload = NewObject<UAnimResultPayload>();
		Payload->RequestId = InRequestId;
		Payload->CompletionResult = InCompletionResult;
		Payload->ActionTag = InActionTag;
		return Payload;
	}


};
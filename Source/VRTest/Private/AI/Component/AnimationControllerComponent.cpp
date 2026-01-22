// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Component/AnimationControllerComponent.h"

#include "Misc/MapErrors.h"

// Sets default values for this component's properties
UAnimationControllerComponent::UAnimationControllerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	

	// ...
}


// Called when the game starts
void UAnimationControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	EventBusComponent = GetOwner()->FindComponentByClass<UEventBusComponent>();
	if(EventBusComponent)
	{
		CurrentMontageDelegateHandle = EventBusComponent->RegisterNativeListener(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Animation.PlayMontage"))),FOnGameplayEventNative::FDelegate::CreateUObject(this,&UAnimationControllerComponent::PlayMontage));
	}else
	{
		UE_LOG(LogTemp,Error,TEXT("CanNotGetTheEventBusComponent"));
	}
	USkeletalMeshComponent* OwnerMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (OwnerMeshComponent && OwnerMeshComponent->GetAnimInstance())
	{
		AnimInstance = OwnerMeshComponent->GetAnimInstance();
		AnimInstance->OnMontageEnded.AddDynamic(this,&UAnimationControllerComponent::OnMontageEnded_CallBack);
	}
	
	
}

void UAnimationControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(CurrentMontageDelegateHandle.IsValid() && EventBusComponent)
	{
		EventBusComponent->UnregisterNativeListener(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Animation.PlayMontage"))),CurrentMontageDelegateHandle);	
	}

	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this,&UAnimationControllerComponent::OnMontageEnded_CallBack);
	}
	
}


// Called every frame
void UAnimationControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UAnimationControllerComponent::PlayMontage(FGameplayTag Tag, UObject* Payload)
{
	UAnimRequestPayload* AnimPayload = Cast<UAnimRequestPayload>(Payload);
	if(!AnimPayload)
	{
		UE_LOG(LogTemp,Error,TEXT("AnimPayloadIsInvalid"));
		return;
	}
	if (bIsPlaying)
	{
		StopCurrentMontage();
	}
	CurrentActionTag = AnimPayload->ActionTag;
	CurrentRequestId = AnimPayload->RequestId;
	bIsPlaying = true;

	UAnimMontage* MontageToPlay = AnimPayload->Montage;
	float PlayRate = AnimPayload->PlayRate;
	FName StartSectionName = AnimPayload->StartSectionName;
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(MontageToPlay,PlayRate);
		if(StartSectionName != NAME_None)
		{
			//如果需要跳转者通过这里直接跳转到对应的地方
			AnimInstance->Montage_JumpToSection(StartSectionName,MontageToPlay);
		}
	}
	
}

void UAnimationControllerComponent::StopCurrentMontage()
{
	if (!bIsPlaying || !AnimInstance)
	{
		return;
	}
	AnimInstance->Montage_Stop(0.2f);
}

float UAnimationControllerComponent::GetCurrentMontagePosition() const
{
	if (AnimInstance && bIsPlaying)
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			return AnimInstance->Montage_GetPosition(CurrentMontage);
		}
	}
	return 0.f;
		
	
}

void UAnimationControllerComponent::OnMontageEnded_CallBack(UAnimMontage* Montage,bool bInterrupted)
{
	HandleMontageEndedAndBroadcastResult(bInterrupted);
}

void UAnimationControllerComponent::HandleMontageEndedAndBroadcastResult(bool bInterrupted)
{
	if (!bIsPlaying)
	{
		return;
	}
	
	UAnimResultPayload* ResultPayload = UAnimResultPayload::Create(CurrentRequestId,bInterrupted ? EAnimCompletionResult::Interrupted : EAnimCompletionResult::Success,CurrentActionTag);
	
	if (!ResultPayload)
	{
		UE_LOG(LogTemp,Error,TEXT("Can Not Create ResultPayload"));
		return;
	}
	
	if (EventBusComponent)
	{
		EventBusComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Animation.MontageFinished"))),ResultPayload);
	}else
	{
		UE_LOG(LogTemp,Error,TEXT("EventBusComponent Is Invalid"));
	}
	CurrentActionTag = FGameplayTag();
	CurrentRequestId = FGuid();
	bIsPlaying = false;
	
}


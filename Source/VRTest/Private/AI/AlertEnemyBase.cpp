// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AlertEnemyBase.h"

#include "AI/Component/EventBusComponent.h"


AAlertEnemyBase::AAlertEnemyBase()
{
	EventBusComponent = CreateDefaultSubobject<UEventBusComponent>(TEXT("EventBusComponent"));
}


void AAlertEnemyBase::CheckEventBusComponent()
{
	if (EventBusComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("EventBusComponent is valid."));
	}
}

void AAlertEnemyBase::TestBoardcast()
{
	EventCount  = 0;
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Test")));

	FDelegateHandle Handle = EventBusComponent->RegisterNativeListener(Tag,FOnGameplayEventNative::FDelegate::CreateUObject(this, &AAlertEnemyBase::OnEventReceived));
	
	EventBusComponent->BroadcastEvent(Tag);

	UE_LOG(LogTemp, Warning, TEXT("TestBoardCast: Count = %d"), EventCount);

	EventBusComponent->BroadcastEvent(Tag);
	EventBusComponent->BroadcastEvent(Tag);
	EventBusComponent->BroadcastEvent(Tag);

	UE_LOG(LogTemp, Warning, TEXT("TestBoardCast: Count  = %d"), EventCount);
	
	EventBusComponent->UnregisterNativeListener(Tag,Handle);
	EventBusComponent->UnregisterAllListenersForEvent(Tag);
}

void AAlertEnemyBase::TestListen()
{
	EventCount  = 0;
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Test")));

	FDelegateHandle Handle = EventBusComponent->RegisterNativeListenerOnce(Tag,FOnGameplayEventNative::FDelegate::CreateUObject(this,&AAlertEnemyBase::OnEventReceived));

	EventBusComponent->BroadcastEvent(Tag);
	EventBusComponent->BroadcastEvent(Tag);

	UE_LOG(LogTemp, Warning, TEXT("TestListen: Count = %d"), EventCount);
	
}

void AAlertEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAlertEnemyBase::OnEventReceived(FGameplayTag Tag, UObject* Payload)
{
	EventCount++;
	UE_LOG(LogTemp, Warning, TEXT("EventReceived! Tag=%s"), *Tag.ToString());
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Component/SacraEnemyStatusUIComponent.h"

#include "AI/SacraEnemyAIControllerBase.h"
#include "AI/UI/SacraEnemyStatusWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

USacraEnemyStatusUIComponent::USacraEnemyStatusUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	StatusWidgetClass = USacraEnemyStatusWidget::StaticClass();
}

void USacraEnemyStatusUIComponent::BeginPlay()
{
	Super::BeginPlay();

	InitStatusUIComponent();
	SetStatusUIPaused(bIsStatusUIPaused);
}

void USacraEnemyStatusUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHatredDelegates();

	Super::EndPlay(EndPlayReason);
}

void USacraEnemyStatusUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsStatusUIPaused)
	{
		return;
	}

	if (!IsValid(CachedHatredComponent) || !IsValid(CachedStatusWidget))
	{
		InitStatusUIComponent();
	}

	UpdateWidgetVisibility();

	if (ShouldShowWidget())
	{
		FaceWidgetToPlayerCamera();
	}
}

void USacraEnemyStatusUIComponent::InitStatusUIComponent()
{
	CachedHatredComponent = ResolveHatredComponent();
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	CreateWidgetComponentIfNeeded();
	if (!bIsStatusUIPaused)
	{
		BindHatredDelegates();
		RefreshStatusUI();
	}
}

void USacraEnemyStatusUIComponent::RefreshStatusUI()
{
	if (bIsStatusUIPaused)
	{
		UpdateWidgetVisibility();
		return;
	}

	if (!IsValid(CachedHatredComponent))
	{
		CachedHatredComponent = ResolveHatredComponent();
	}

	if (!IsValid(CachedHatredComponent) || !IsValid(CachedStatusWidget))
	{
		return;
	}

	CachedStatusWidget->SetHatredState(CachedHatredComponent->GetCurrentHatredState());
	CachedStatusWidget->SetHatredPercent(GetHatredPercent());
	UpdateWidgetVisibility();
}

void USacraEnemyStatusUIComponent::ApplyConfigData(const FSacraEnemyStatusUIConfig& ConfigData)
{
	if (!ConfigData.bOverrideStatusUIConfig)
	{
		return;
	}

	StatusWidgetClass = ConfigData.StatusWidgetClass;
	WidgetRelativeLocation = ConfigData.WidgetRelativeLocation;
	DrawSize = ConfigData.DrawSize;
	bHideWhenIdle = ConfigData.bHideWhenIdle;
	MaxVisibleDistance = ConfigData.MaxVisibleDistance;

	if (IsValid(StatusWidgetComponent))
	{
		StatusWidgetComponent->SetDrawSize(DrawSize);
		StatusWidgetComponent->SetRelativeLocation(WidgetRelativeLocation);

		if (StatusWidgetClass)
		{
			StatusWidgetComponent->SetWidgetClass(StatusWidgetClass);
			StatusWidgetComponent->InitWidget();
			CachedStatusWidget = Cast<USacraEnemyStatusWidget>(StatusWidgetComponent->GetWidget());
		}
	}

	RefreshStatusUI();
}

void USacraEnemyStatusUIComponent::SetStatusUIPaused(bool bInPaused)
{
	if (bIsStatusUIPaused == bInPaused)
	{
		SetComponentTickEnabled(!bIsStatusUIPaused);
		UpdateWidgetVisibility();
		return;
	}

	bIsStatusUIPaused = bInPaused;
	SetComponentTickEnabled(!bIsStatusUIPaused);

	if (bIsStatusUIPaused)
	{
		UnbindHatredDelegates();
		UpdateWidgetVisibility();
		return;
	}

	if (!IsValid(CachedHatredComponent) || !IsValid(CachedStatusWidget))
	{
		InitStatusUIComponent();
	}

	BindHatredDelegates();
	RefreshStatusUI();
}

void USacraEnemyStatusUIComponent::HandleHatredStateChanged(EHatredState NewState)
{
	if (IsValid(CachedStatusWidget))
	{
		CachedStatusWidget->SetHatredState(NewState);
	}

	UpdateWidgetVisibility();
}

void USacraEnemyStatusUIComponent::HandleHatredValueChanged(float NewValue)
{
	if (IsValid(CachedStatusWidget))
	{
		CachedStatusWidget->SetHatredPercent(GetHatredPercent());
	}

	UpdateWidgetVisibility();
}

void USacraEnemyStatusUIComponent::CreateWidgetComponentIfNeeded()
{
	if (!IsValid(GetOwner()) || IsValid(StatusWidgetComponent))
	{
		return;
	}

	StatusWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("EnemyStatusWidgetComponent"));
	if (!IsValid(StatusWidgetComponent))
	{
		return;
	}

	StatusWidgetComponent->SetupAttachment(GetOwner()->GetRootComponent());
	StatusWidgetComponent->RegisterComponent();
	StatusWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	StatusWidgetComponent->SetDrawAtDesiredSize(false);
	StatusWidgetComponent->SetDrawSize(DrawSize);
	StatusWidgetComponent->SetRelativeLocation(WidgetRelativeLocation);
	StatusWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StatusWidgetComponent->SetGenerateOverlapEvents(false);
	StatusWidgetComponent->SetTwoSided(false);
	StatusWidgetComponent->SetOwnerPlayer(UGameplayStatics::GetPlayerController(this, 0) ? UGameplayStatics::GetPlayerController(this, 0)->GetLocalPlayer() : nullptr);
	StatusWidgetComponent->SetWidgetClass(StatusWidgetClass ? StatusWidgetClass.Get() : USacraEnemyStatusWidget::StaticClass());
	StatusWidgetComponent->InitWidget();

	CachedStatusWidget = Cast<USacraEnemyStatusWidget>(StatusWidgetComponent->GetWidget());
}

void USacraEnemyStatusUIComponent::BindHatredDelegates()
{
	UnbindHatredDelegates();

	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	CachedHatredComponent->OnHatredStateChanged.AddDynamic(this, &USacraEnemyStatusUIComponent::HandleHatredStateChanged);
	CachedHatredComponent->OnHatredValueChanged.AddDynamic(this, &USacraEnemyStatusUIComponent::HandleHatredValueChanged);
}

void USacraEnemyStatusUIComponent::UnbindHatredDelegates()
{
	if (!IsValid(CachedHatredComponent))
	{
		return;
	}

	CachedHatredComponent->OnHatredStateChanged.RemoveDynamic(this, &USacraEnemyStatusUIComponent::HandleHatredStateChanged);
	CachedHatredComponent->OnHatredValueChanged.RemoveDynamic(this, &USacraEnemyStatusUIComponent::HandleHatredValueChanged);
}

USacraEnemyHatredComponent* USacraEnemyStatusUIComponent::ResolveHatredComponent() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (ASacraEnemyAIControllerBase* EnemyController = Cast<ASacraEnemyAIControllerBase>(OwnerPawn->GetController()))
			{
				return EnemyController->FindComponentByClass<USacraEnemyHatredComponent>();
			}
		}

		return OwnerActor->FindComponentByClass<USacraEnemyHatredComponent>();
	}

	return nullptr;
}

float USacraEnemyStatusUIComponent::GetHatredPercent() const
{
	if (!IsValid(CachedHatredComponent))
	{
		return 0.0f;
	}

	const float CurrentValue = CachedHatredComponent->GetCurrentHatredValue();
	const float MaxValue = FMath::Max(CachedHatredComponent->GetMaxHatredValue(), 1.0f);
	return FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f);
}

bool USacraEnemyStatusUIComponent::ShouldShowWidget() const
{
	if (bIsStatusUIPaused || !IsValid(StatusWidgetComponent) || !IsValid(CachedHatredComponent) || !GetWorld())
	{
		return false;
	}

	if (bHideWhenIdle && CachedHatredComponent->GetCurrentHatredState() == EHatredState::Idle)
	{
		return false;
	}

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const float Distance = FVector::Dist(CameraManager->GetCameraLocation(), GetOwner()->GetActorLocation());
		return Distance <= MaxVisibleDistance;
	}

	return true;
}

void USacraEnemyStatusUIComponent::UpdateWidgetVisibility()
{
	if (!IsValid(StatusWidgetComponent) || !IsValid(CachedStatusWidget))
	{
		return;
	}

	const bool bShouldShow = ShouldShowWidget();
	StatusWidgetComponent->SetVisibility(bShouldShow);
	CachedStatusWidget->SetWidgetVisible(bShouldShow);
}

void USacraEnemyStatusUIComponent::FaceWidgetToPlayerCamera()
{
	if (!IsValid(StatusWidgetComponent))
	{
		return;
	}

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector CameraLocation = CameraManager->GetCameraLocation();
		const FVector WidgetLocation = StatusWidgetComponent->GetComponentLocation();
		const FRotator LookAtRotation = (CameraLocation - WidgetLocation).Rotation();

		StatusWidgetComponent->SetWorldRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
	}
}

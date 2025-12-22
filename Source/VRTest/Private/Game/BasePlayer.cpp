// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BasePlayer.h"

ABasePlayer::ABasePlayer()
{
	FallDamageComponent = CreateDefaultSubobject<UFallDamageComponent>(TEXT("FallDamageComponent"));
	AutoRecoverComponent = CreateDefaultSubobject<UAutoRecoverComponent>(TEXT("AutoRecoverComponent"));
}

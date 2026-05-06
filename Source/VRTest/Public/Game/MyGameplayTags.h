#pragma once

#include "NativeGameplayTags.h"

/**
 * 使用宏声明外部可访问的标签
 */
namespace MyProjectTags
{
	// NormalSound
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_PlayerInGrass);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_StarHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_StarShatter);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_EagleEye);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_GravityGloveDragBack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_BowStringTight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_ArrowShoot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_JarBreak);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NormalSound_HitNoise);

	// AI State
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_State_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_State_Warning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_State_Fight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_State_Idle_Guard);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_State_Idle_Patrol);

	// AI Behavior
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Idle_Guard);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Idle_Patrol);

	// AI Message
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Message_Hatred);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Message_Hatred_WarningExitToIdle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Message_Weapon_EquipFinished);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Message_Weapon_AttackFinished);
}

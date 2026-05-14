#include "Game/MyGameplayTags.h"

namespace MyProjectTags
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_PlayerInGrass, "NormalSound.PlayerInGrass");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_StarHit, "NormalSound.StarHit");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_StarShatter, "NormalSound.StarShatter");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_EagleEye, "NormalSound.EagleEye");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_GravityGloveDragBack, "NormalSound.GravityGloveDragBack");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_BowStringTight, "NormalSound.BowStringTight");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_ArrowShoot, "NormalSound.ArrowShoot");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_JarBreak, "NormalSound.JarBreak");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NormalSound_HitNoise, "NormalSound.HitNoise");

	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_State_Idle, "AI.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_State_Warning, "AI.State.Warning");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_State_Fight, "AI.State.Fight");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_State_Idle_Guard, "AI.State.Idle.Guard");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_State_Idle_Patrol, "AI.State.Idle.Patrol");

	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Behavior_Idle_Guard, "AI.Behavior.Idle.Guard");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Behavior_Idle_Patrol, "AI.Behavior.Idle.Patrol");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Behavior_Subtree_Idle, "AI.Behavior.Subtree.Idle");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Behavior_Subtree_Warning, "AI.Behavior.Subtree.Warning");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Behavior_Subtree_Fight, "AI.Behavior.Subtree.Fight");

	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Message_Hatred, "AI.Message.Hatred");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Message_Hatred_WarningExitToIdle, "AI.Message.Hatred.WarningExitToIdle");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Message_Weapon_EquipFinished, "AI.Message.Weapon.EquipFinished");
	UE_DEFINE_GAMEPLAY_TAG(TAG_AI_Message_Weapon_AttackFinished, "AI.Message.Weapon.AttackFinished");
}

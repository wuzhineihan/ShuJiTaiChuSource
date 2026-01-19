#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Central place for collision-related names.
 *
 * Motivation:
 * - Avoid using raw ECC_GameTraceChannelX values in code.
 * - Avoid typo-prone profile FName strings.
 *
 * IMPORTANT:
 * These mappings must match Project Settings -> Collision (stored in DefaultEngine.ini).
 */

// -------------------- Channels --------------------

/** Object Channel: Obj_PlayerHand (DefaultEngine.ini: ECC_GameTraceChannel1) */
#define OCC_PLAYER_HAND ECC_GameTraceChannel1

/** Trace Channel: Trace_Grab (DefaultEngine.ini: ECC_GameTraceChannel2) */
#define TCC_GRAB ECC_GameTraceChannel2

/** Trace Channel: Trace_Projectile (DefaultEngine.ini: ECC_GameTraceChannel3) */
#define TCC_PROJECTILE ECC_GameTraceChannel3

// -------------------- Collision Profiles --------------------
// Use these instead of raw strings like "Profile_PlayerHand".

#define CP_NO_COLLISION FName(TEXT("NoCollision"))

#define CP_PLAYER_CAPSULE FName(TEXT("Profile_PlayerCapsule"))
#define CP_PLAYER_HAND FName(TEXT("Profile_PlayerHand"))
#define CP_PLAYER_BACKPACK FName(TEXT("Profile_PlayerBackpack"))

#define CP_ENEMY_CAPSULE FName(TEXT("Profile_EnemyCapsule"))
#define CP_ENEMY_MESH_ALIVE FName(TEXT("Profile_EnemyMesh_Alive"))
#define CP_ENEMY_MESH_RAGDOLL FName(TEXT("Profile_EnemyMesh_Ragdoll"))

#define CP_GRABBABLE_PHYSICS FName(TEXT("Profile_Grabbable_Physics"))
#define CP_BOW_STRING_COLLISION FName(TEXT("Profile_BowStringCollision"))
#define CP_ARROW_STUCK FName(TEXT("Profile_Arrow_Stuck"))
#define CP_STASIS_POINT_FIRED FName(TEXT("Profile_StasisPoint_Fired"))

#define CP_STAR_FINGER FName(TEXT("Profile_Star_Finger"))
#define CP_STAR_OTHER FName(TEXT("Profile_Star_Other"))


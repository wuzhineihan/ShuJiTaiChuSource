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
}

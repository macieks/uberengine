#ifndef GX_ANIMATION_H
#define GX_ANIMATION_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueResource.h"
#include "Base/ueMath.h"

UE_DEFINE_RESOURCE_TYPE(gxAnimation, UE_BE_4BYTE_SYMBOL('s','k','e','l'));

void gxAnimation_SampleAt(gxAnimation* anim, f32 time, u32 boneIndex, ueMat44& transform);

void gxAnimationMgr_Startup(ueAllocator* allocator, const char* config);
void gxAnimationMgr_Shutdown();

// @}

#endif // GX_ANIMATION_H

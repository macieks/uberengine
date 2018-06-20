#ifndef PH_BOX2D_H
#define PH_BOX2D_H

#include "Base/ueBase.h"
#include "Box2D.h"

/**
 *	@addtogroup ph
 *	@{
 */

class b2DebugDraw;

//! Starts up Box2D physics library
void phBox2D_Startup(ueAllocator* allocator);
//! Shuts down Box2D physics library
void phBox2D_Shutdown();

//! Sets debug draw flags for Box2D library
void phBox2D_SetDebugDrawFlags(u32 flags);
//! Sets debug draw alpha / opaqueness value
void phBox2D_SetDebugDrawAlpha(f32 alpha);
//! Sets draw scale
void phBox2D_SetDebugDrawScale(f32 scale);
//! Gets default debug draw interface for Box2D
b2DebugDraw* phBox2D_GetDefaultDebugDraw();

UE_INLINE void phBox2D_FromBtVec(ueVec2& out, const b2Vec2& in)
{
	out.Set(in.x, in.y);
}

UE_INLINE void phBox2D_ToBtVec(b2Vec2& out, const ueVec2& in)
{
	out.x = in[0];
	out.y = in[1];
}

// @}

#endif // PH_BOX2D_H
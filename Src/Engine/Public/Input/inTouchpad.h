#ifndef IN_TOUCHPAD_H
#define IN_TOUCHPAD_H

#include "Base/ueShapes.h"

//! Touch point description
struct inTouch
{
	s32 m_x;
	s32 m_y;
	s32 m_prevX;
	s32 m_prevY;
	u32 m_id; //! Persistent (within swipe) touch id
};

void inTouchpad_Startup(ueBool multitouch = UE_TRUE, s32 dispFixRot = 2);
void inTouchpad_Shutdown();

void inTouchpad_PreSystemUpdate();
void inTouchpad_Update();

u32 inTouchpad_GetNumTouches();
const inTouch& inTouchpad_GetTouch(u32 index);

ueBool inTouchpad_IsDown();
ueBool inTouchpad_WasDown();
ueBool inTouchpad_WasPressed();
ueBool inTouchpad_WasReleased(f32* x = NULL, f32* y = NULL);

ueBool inTouchpad_IsDown(const ueRect& area);
ueBool inTouchpad_WasReleased(const ueRect& area);

#endif // IN_TOUCHPAD_H
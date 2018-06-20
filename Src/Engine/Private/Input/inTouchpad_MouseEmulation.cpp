#include "Input/inTouchpad.h"
#include "Input/inMouse.h"

static ueBool s_isDown;
static ueBool s_wasDown;
static inTouch s_touch;

void inTouchpad_Startup(ueBool multitouch, s32 dispFixRot)
{
	s_isDown = s_wasDown = UE_FALSE;
}

void inTouchpad_Shutdown()
{
}

void inTouchpad_PreSystemUpdate()
{
}

void inTouchpad_Update()
{
	s_wasDown = s_isDown = inMouse_IsDown(inMouseButton_Left);
}

u32 inTouchpad_GetNumTouches()
{
	return s_isDown ? 1 : 0;
}

const inTouch& inTouchpad_GetTouch(u32 index)
{
	UE_ASSERT(s_isDown);
	return s_touch;
}

ueBool inTouchpad_IsDown()
{
	return s_isDown;
}

ueBool inTouchpad_WasDown()
{
	return s_wasDown;
}

ueBool inTouchpad_WasPressed()
{
	return s_isDown && !s_wasDown;
}

ueBool inTouchpad_WasReleased(f32* x, f32* y)
{
	if (!s_isDown && s_wasDown)
	{
		if (x) *x = (f32) s_touch.m_x;
		if (y) *y = (f32) s_touch.m_y;
		return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool inTouchpad_IsDown(const ueRect& area)
{
	return inTouchpad_IsDown() ? area.Intersect((f32) s_touch.m_x, (f32) s_touch.m_y) : UE_FALSE;
}

ueBool inTouchpad_WasReleased(const ueRect& area)
{
	return inTouchpad_WasReleased() ? area.Intersect((f32) s_touch.m_x, (f32) s_touch.m_y) : UE_FALSE;
}
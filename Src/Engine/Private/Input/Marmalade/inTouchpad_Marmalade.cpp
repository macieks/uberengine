#include "Input/inTouchpad.h"
#include "Input/inAccelerometer.h"

struct inTouchpadData
{
	ueBool m_isInitialized;

	s32 m_dispFixRot;

	ueBool m_useMultiTouch;

	u32 m_numTouches;
	inTouch m_touches[S3E_POINTER_TOUCH_MAX];

	ueBool m_touchDownHistory[2];

	inTouchpadData() :
		m_isInitialized(UE_FALSE)
	{}
};

static inTouchpadData s_data;

void inTouchpad_SetTouchPos(inTouch& touch, s32 x, s32 y)
{
	touch.m_prevX = touch.m_x;
	touch.m_prevY = touch.m_y;

	if (inAccelerometer_IsFixedPositionInverted())
	{
		x = IwGxGetDisplayHeight() - x;
		y = IwGxGetDisplayWidth() - y;
	}

	touch.m_x = x;
	touch.m_y = y;
}

void inTouchpad_MultiTouchButtonCB(s3ePointerTouchEvent* ev)
{
	// Remove touch

	if (!ev->m_Pressed)
	{
		for (u32 i = 0; i < s_data.m_numTouches; i++)
			if (s_data.m_touches[i].m_id == ev->m_TouchID)
			{
				s_data.m_touches[i] = s_data.m_touches[--s_data.m_numTouches];
				break;
			}
		return;
	}

	// Add touch

	if (s_data.m_numTouches == UE_ARRAY_SIZE(s_data.m_touches))
		return;

	inTouch& touch = s_data.m_touches[s_data.m_numTouches++];
	touch.m_id = ev->m_TouchID;
	inTouchpad_SetTouchPos(touch, ev->m_x, ev->m_y);
	touch.m_prevX = touch.m_x;
	touch.m_prevY = touch.m_y;
}

void inTouchpad_MultiTouchMotionCB(s3ePointerTouchMotionEvent* ev)
{
	for (u32 i = 0; i < s_data.m_numTouches; i++)
		if (s_data.m_touches[i].m_id == ev->m_TouchID)
		{
			inTouch& touch = s_data.m_touches[i];
			inTouchpad_SetTouchPos(touch, ev->m_x, ev->m_y);
		}
}

void inTouchpad_SingleTouchButtonCB(s3ePointerEvent* e)
{
    s_data.m_numTouches = e->m_Pressed ? 1 : 0;
	inTouchpad_SetTouchPos(s_data.m_touches[0], e->m_x, e->m_y);
	s_data.m_touches[0].m_prevX = s_data.m_touches[0].m_x;
	s_data.m_touches[0].m_prevY = s_data.m_touches[0].m_y;
}

void inTouchpad_SingleTouchMotionCB(s3ePointerMotionEvent* e)
{
    inTouchpad_SetTouchPos(s_data.m_touches[0], e->m_x, e->m_y);
}

void inTouchpad_Startup(ueBool multitouch, s32 dispFixRot)
{
	ueAssert(!s_data.m_isInitialized);
	s_data.m_isInitialized = UE_TRUE;

	s_data.m_dispFixRot = dispFixRot;

	s_data.m_numTouches = 0;
	s_data.m_touchDownHistory[0] = s_data.m_touchDownHistory[1] = UE_FALSE;

    s_data.m_useMultiTouch = multitouch && s3ePointerGetInt(S3E_POINTER_MULTI_TOUCH_AVAILABLE);

    if (s_data.m_useMultiTouch)
    {
        s3ePointerRegister(S3E_POINTER_TOUCH_EVENT, (s3eCallback) inTouchpad_MultiTouchButtonCB, NULL);
        s3ePointerRegister(S3E_POINTER_TOUCH_MOTION_EVENT, (s3eCallback) inTouchpad_MultiTouchMotionCB, NULL);
    }
    else
    {
        s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback) inTouchpad_SingleTouchButtonCB, NULL);
        s3ePointerRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback) inTouchpad_SingleTouchMotionCB, NULL);
    }
}

void inTouchpad_Shutdown()
{
	ueAssert(s_data.m_isInitialized);
	s_data.m_isInitialized = UE_FALSE;

    if (s_data.m_useMultiTouch)
    {
        s3ePointerUnRegister(S3E_POINTER_TOUCH_EVENT, (s3eCallback) inTouchpad_MultiTouchButtonCB);
        s3ePointerUnRegister(S3E_POINTER_TOUCH_MOTION_EVENT, (s3eCallback) inTouchpad_MultiTouchMotionCB);
    }
    else
    {
        s3ePointerUnRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback) inTouchpad_SingleTouchButtonCB);
        s3ePointerUnRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback) inTouchpad_SingleTouchMotionCB);
    }
}

void inTouchpad_PreSystemUpdate()
{
	for (u32 i = 0; i < s_data.m_numTouches; i++)
	{
		inTouch& touch = s_data.m_touches[i];
		touch.m_prevX = touch.m_x;
		touch.m_prevY = touch.m_y;
	}
}

void inTouchpad_Update()
{
	s3ePointerUpdate();

	s_data.m_touchDownHistory[0] = s_data.m_touchDownHistory[1];
	s_data.m_touchDownHistory[1] = s_data.m_numTouches > 0;
}

ueBool inTouchpad_IsDown(const ueRect& area)
{
	ueRectI areaI;
	areaI.FromRectF(area);

	for (u32 i = 0; i < s_data.m_numTouches; i++)
	{
		const inTouch& touch = s_data.m_touches[i];
		if (areaI.Intersect(touch.m_x, touch.m_y))
			return UE_TRUE;
	}

	return UE_FALSE;
}

ueBool inTouchpad_IsDown()
{
	return s_data.m_touchDownHistory[1];
}

ueBool inTouchpad_WasDown()
{
	return s_data.m_touchDownHistory[0];
}

ueBool inTouchpad_WasPressed()
{
	return !s_data.m_touchDownHistory[0] && s_data.m_touchDownHistory[1];
}

ueBool inTouchpad_WasReleased(f32* x, f32* y)
{
	const ueBool wasReleased = s_data.m_touchDownHistory[0] && !s_data.m_touchDownHistory[1];
	if (x)
		*x = (f32) s_data.m_touches[0].m_x;
	if (y)
		*y = (f32) s_data.m_touches[0].m_y;
	return wasReleased;
}

ueBool inTouchpad_WasReleased(const ueRect& area)
{
	return inTouchpad_WasReleased() && area.Intersect((f32) s_data.m_touches[0].m_x, (f32) s_data.m_touches[0].m_y);
}

u32 inTouchpad_GetNumTouches()
{
	return s_data.m_numTouches;
}

const inTouch& inTouchpad_GetTouch(u32 index)
{
	ueAssert(index < s_data.m_numTouches);
	return s_data.m_touches[index];
}
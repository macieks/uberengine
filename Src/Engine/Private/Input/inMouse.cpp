#include "Input/inMouse.h"
#include "Input/inMouse_Private.h"
#include "Input/inSys_Private.h"

inMouseState s_mouseState;

ueBool inMouse_IsValid(u32 userId) { return userId == 0 && s_mouseState.m_isValid; }
ueBool inMouse_IsValidAny() { return s_mouseState.m_isValid; }

void inMouse_GetPos(u32 userId, s32& x, s32& y) { x = s_mouseState.m_pos[0]; y = s_mouseState.m_pos[1]; }
void inMouse_GetVector(u32 userId, s32& x, s32& y) { x = s_mouseState.m_vector[0]; y = s_mouseState.m_vector[1]; }
void inMouse_GetWheelDelta(u32 userId, s32& delta) { delta = s_mouseState.m_wheelDelta; }

ueBool inMouse_IsDown(u32 userId, inMouseButton button) { return !userId && (s_mouseState.m_buttons[button] & (inKeyStateFlags_IsDown_User0 << userId)) != 0; }
ueBool inMouse_WasDown(u32 userId, inMouseButton button) { return !userId && (s_mouseState.m_buttons[button] & (inKeyStateFlags_WasDown_User0 << userId)) != 0; }
ueBool inMouse_WasPressed(u32 userId, inMouseButton button) { return !userId && (s_mouseState.m_buttons[button] & (inKeyStateFlags_IsDown_User0 << userId)) != 0; }
ueBool inMouse_WasReleased(u32 userId, inMouseButton button) { return !userId && (s_mouseState.m_buttons[button] & (inKeyStateFlags_WasDown_User0 << userId)) != 0; }

void inMouse_GetPos(s32& x, s32& y) { x = s_mouseState.m_pos[0]; y = s_mouseState.m_pos[1]; }
void inMouse_GetVector(s32& x, s32& y) { x = s_mouseState.m_vector[0]; y = s_mouseState.m_vector[1]; }
void inMouse_GetWheelDelta(s32& delta) { delta = s_mouseState.m_wheelDelta; }
ueBool inMouse_IsDown(inMouseButton button) { return (s_mouseState.m_buttons[button] & inKeyStateFlags_IsDown_AllUsers) != 0; }
ueBool inMouse_WasDown(inMouseButton button) { return (s_mouseState.m_buttons[button] & inKeyStateFlags_WasDown_AllUsers) != 0; }
ueBool inMouse_WasPressed(inMouseButton button) { return s_mouseState.m_buttons[button] == inKeyStateFlags_IsDown_User0; }
ueBool inMouse_WasReleased(inMouseButton button) { return s_mouseState.m_buttons[button] == inKeyStateFlags_WasDown_User0; }

const char* inMouse_GetButtonName(inMouseButton button)
{
	switch (button)
	{
		case inMouseButton_Left: return "LMB";
		case inMouseButton_Middle: return "MMB";
		case inMouseButton_Right: return "RMB";
		UE_INVALID_CASE(button);
	}
	return NULL;
}

// Private

void inMouse_ResetState()
{
	s_mouseState.m_isValid = UE_FALSE;

	for (u32 i = 0; i < inMouseButton_MAX; ++i)
		s_mouseState.m_buttons[i] = 0;

	s_mouseState.m_vector[0] = 0;
	s_mouseState.m_vector[1] = 0;

	s_mouseState.m_wheelDelta = 0;
	s_mouseState.m_wheelDeltaAcc = 0;
}
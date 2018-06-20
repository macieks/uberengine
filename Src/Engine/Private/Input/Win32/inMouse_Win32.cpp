#include "Input/inMouse.h"
#include "Input/inMouse_Private.h"
#include "Input/inSys_Private.h"
#include "Base/ueWindow.h"

void inMouse_Startup()
{
	inMouse_ResetState();

	POINT pos;
	GetCursorPos(&pos);
	if (GetCursorPos(&pos))
	{
		s_mouseState.m_initialPos[0] = pos.x;
		s_mouseState.m_initialPos[1] = pos.y;
		s_mouseState.m_hasInitialPos = UE_TRUE;
	}
}

void inMouse_Shutdown()
{
	if (s_mouseState.m_hasInitialPos)
		SetCursorPos(s_mouseState.m_initialPos[0], s_mouseState.m_initialPos[1]);
}

void inMouse_Update(ueBool centerCursor)
{
	// Store previous buttons state

	for (u32 i = 0; i < inMouseButton_MAX; ++i)
		if (s_mouseState.m_buttons[i] & inKeyStateFlags_IsDown_User0)
			s_mouseState.m_buttons[i] |= inKeyStateFlags_WasDown_User0;
		else
			s_mouseState.m_buttons[i] &= ~inKeyStateFlags_WasDown_User0;

	// Update buttons

#define UPDATE_BUTTON(buttonName, vkCode) \
	if (GetAsyncKeyState(vkCode) & 0x8000) \
		s_mouseState.m_buttons[inMouseButton_##buttonName] |= inKeyStateFlags_IsDown_User0; \
	else \
		s_mouseState.m_buttons[inMouseButton_##buttonName] &= ~inKeyStateFlags_IsDown_User0;

	UPDATE_BUTTON(Left, VK_LBUTTON);
	UPDATE_BUTTON(Middle, VK_MBUTTON);
	UPDATE_BUTTON(Right, VK_RBUTTON);

	// Update movement vector

	ueWindow* window = ueWindow_GetMain();
	UE_ASSERT(window);
	const ueRectI& windowRect = ueWindow_GetWorkArea(window);

	POINT pos;
	if (!GetCursorPos(&pos))
	{
		pos.x = windowRect.CenterX();
		pos.y = windowRect.CenterY();

		s_mouseState.m_isValid = UE_FALSE;
	}
	else
		s_mouseState.m_isValid = UE_TRUE;

	s_mouseState.m_pos[0] = pos.x;
	s_mouseState.m_pos[1] = pos.y;

	if (centerCursor)
	{
		SetCursorPos(windowRect.CenterX(), windowRect.CenterY());

		s_mouseState.m_vector[0] = s_mouseState.m_pos[0] - windowRect.CenterX();
		s_mouseState.m_vector[1] = s_mouseState.m_pos[1] - windowRect.CenterY();
	}
	else
	{
		s_mouseState.m_vector[0] = s_mouseState.m_pos[0] - s_mouseState.m_prevPos[0];
		s_mouseState.m_vector[1] = s_mouseState.m_pos[1] - s_mouseState.m_prevPos[1];

		s_mouseState.m_prevPos[0] = s_mouseState.m_pos[0];
		s_mouseState.m_prevPos[1] = s_mouseState.m_pos[1];
	}

	// Mouse wheel delta acc gets updated from within the window update

	s_mouseState.m_wheelDelta = s_mouseState.m_wheelDeltaAcc;
	s_mouseState.m_wheelDeltaAcc = 0;
}
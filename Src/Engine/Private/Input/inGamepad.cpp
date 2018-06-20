#include "Input/inGamepad.h"
#include "Input/inGamepad_Private.h"

inGamepadData s_data;

void inGamepad_UpdateMergedState()
{
	inGamepad& gamepad = s_data.m_gamepads[IN_MAX_USERS];
	gamepad.m_isValid = UE_FALSE;

	for (u32 i = 0; i < inGamepadButton_MAX; i++)
		gamepad.m_buttons[i] = (gamepad.m_buttons[i] & inGamepadButtonFlag_IsDown) ? inGamepadButtonFlag_WasDown : 0;
	for (u32 j = 0; j < inGamepadAnalog_MAX; j++)
		gamepad.m_analogs[j]  = 0.0f;

	for (u32 i = 0; i < IN_MAX_USERS; i++)
	{
		if (!s_data.m_gamepads[i].m_isValid)
			continue;

		gamepad.m_isValid = UE_TRUE;

		for (u32 j = 0; j < inGamepadButton_MAX; j++)
			if (s_data.m_gamepads[i].m_buttons[j] & inGamepadButtonFlag_IsDown)
				gamepad.m_buttons[j] |= inGamepadButtonFlag_IsDown;

		for (u32 j = 0; j < inGamepadAnalog_MAX; j++)
			gamepad.m_analogs[j] += s_data.m_gamepads[i].m_analogs[j];
	}

	for (u32 j = 0; j < inGamepadAnalog_MAX; j++)
		gamepad.m_analogs[j] = ueClamp(gamepad.m_analogs[j], -1.0f, 1.0f);
}

const char* inGamepad_GetButtonName(inGamepadButton button)
{
	switch (button)
	{
		case inGamepadButton_DPad_Up: return "dpad-up";
		case inGamepadButton_DPad_Right: return "dpad-right";
		case inGamepadButton_DPad_Down: return "dpad-down";
		case inGamepadButton_DPad_Left: return "dpad-left";
		case inGamepadButton_Y: return "gamepad-Y";
		case inGamepadButton_B: return "gamepad-B";
		case inGamepadButton_A: return "gamepad-A";
		case inGamepadButton_X: return "gamepad-X";
		case inGamepadButton_Shoulder_L1: return "shoulder-l1";
		case inGamepadButton_Shoulder_R1: return "shoulder-r1";
		case inGamepadButton_Shoulder_L2: return "shoulder-l2";
		case inGamepadButton_Shoulder_R2: return "shoulder-r2";
		case inGamepadButton_Start: return "gamepad-start";
		case inGamepadButton_Back: return "gamepad-back";
		case inGamepadButton_LeftStick: return "left-stick";
		case inGamepadButton_RightStick: return "right-stick";

		// Meta inGamepadButtons (translation from analogs)

		case inGamepadButton_ThumbStick_Left: return "thumb-stick-left";
		case inGamepadButton_ThumbStick_Right: return "thumb-stick-right";

		case inGamepadButton_LeftStick_Up: return "left-stick-up";
		case inGamepadButton_LeftStick_Down: return "left-stick-down";
		case inGamepadButton_LeftStick_Left: return "left-stick-left";
		case inGamepadButton_LeftStick_Right: return "left-stick-right";

		case inGamepadButton_RightStick_Up: return "right-stick-up";
		case inGamepadButton_RightStick_Down: return "right-stick-down";
		case inGamepadButton_RightStick_Left: return "right-stick-left";
		case inGamepadButton_RightStick_Right: return "right-stick-right";

		UE_INVALID_CASE(button);
	}
	return NULL;
}

// State

ueBool inGamepad_IsValid(u32 userId) { UE_ASSERT(userId < IN_MAX_USERS); return s_data.m_gamepads[userId].m_isValid; }
ueBool inGamepad_IsDown(u32 userId, inGamepadButton button) { return s_data.m_gamepads[userId].m_buttons[button] & inGamepadButtonFlag_IsDown; }
ueBool inGamepad_WasPressed(u32 userId, inGamepadButton button) { return s_data.m_gamepads[userId].m_buttons[button] == inGamepadButtonFlag_IsDown; }
ueBool inGamepad_WasReleased(u32 userId, inGamepadButton button) { return s_data.m_gamepads[userId].m_buttons[button] == inGamepadButtonFlag_WasDown; }
f32 inGamepad_GetAnalog(u32 userId, inGamepadAnalog analog) { return s_data.m_gamepads[userId].m_analogs[analog]; }

// Merged gamepad

ueBool inGamepad_IsValidAny() { return s_data.m_gamepads[IN_MAX_USERS].m_isValid; }
ueBool inGamepad_IsDown(inGamepadButton button) { return s_data.m_gamepads[IN_MAX_USERS].m_buttons[button] & inGamepadButtonFlag_IsDown; }
ueBool inGamepad_WasPressed(inGamepadButton button) { return s_data.m_gamepads[IN_MAX_USERS].m_buttons[button] == inGamepadButtonFlag_IsDown; }
ueBool inGamepad_WasReleased(inGamepadButton button) { return s_data.m_gamepads[IN_MAX_USERS].m_buttons[button] == inGamepadButtonFlag_WasDown; }
f32 inGamepad_GetAnalog(inGamepadAnalog analog) { return s_data.m_gamepads[IN_MAX_USERS].m_analogs[analog]; }

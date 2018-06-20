#include "Input/inGamepad.h"
#include "Input/inGamepad_Private.h"

#if defined(UE_WIN32)
	#include <xinput.h>
	#pragma comment(lib, "xinput.lib")
#endif // defined(UE_WIN32)

f32 inGamepad_StickToAnalog(s32 value, s32 deadzone)
{
	if (value < -deadzone)
		return (f32) (value + deadzone) / (f32) (32768 - deadzone);
	else if (value > deadzone)
		return (f32) (value - deadzone) / (f32) (32768 - deadzone);
	return 0.0f;
}

f32 inGamepad_TriggerToAnalog(s32 value, s32 deadzone)
{
	if (value < deadzone)
		return 0.0f;
	return (f32) (value - deadzone) / (f32) (255 - deadzone);
}

void inGamepad_Startup()
{
	inGamepad_Update();
}

void inGamepad_Shutdown()
{
}

void inGamepad_Update()
{
	for (u32 id = 0; id < IN_MAX_USERS; ++id)
	{
		inGamepad& gamepad = s_data.m_gamepads[id];

		// Store old button states

		for (u32 button = 0; button < inGamepadButton_MAX; button++)
			gamepad.m_buttons[button] = (gamepad.m_buttons[button] & inGamepadButtonFlag_IsDown) ? inGamepadButtonFlag_WasDown : 0;

		gamepad.m_isValid = UE_FALSE;

		// Grab current pad state

		XINPUT_STATE xInputState;
		if (XInputGetState(id, &xInputState) == ERROR_SUCCESS)
		{
			gamepad.m_isValid = UE_TRUE;
			
#define SET_BUTTON(engineCode, xFlag) \
			if (xInputState.Gamepad.wButtons & xFlag) \
				gamepad.m_buttons[inGamepad##engineCode] |= inGamepadButtonFlag_IsDown;

			SET_BUTTON(Button_DPad_Up, XINPUT_GAMEPAD_DPAD_UP);
			SET_BUTTON(Button_DPad_Up, XINPUT_GAMEPAD_DPAD_UP);
			SET_BUTTON(Button_DPad_Right, XINPUT_GAMEPAD_DPAD_RIGHT);
			SET_BUTTON(Button_DPad_Down, XINPUT_GAMEPAD_DPAD_DOWN);
			SET_BUTTON(Button_DPad_Left, XINPUT_GAMEPAD_DPAD_LEFT);
			SET_BUTTON(Button_Y, XINPUT_GAMEPAD_Y);
			SET_BUTTON(Button_B, XINPUT_GAMEPAD_B);
			SET_BUTTON(Button_A, XINPUT_GAMEPAD_A);
			SET_BUTTON(Button_X, XINPUT_GAMEPAD_X);
			SET_BUTTON(Button_Shoulder_L1, XINPUT_GAMEPAD_LEFT_SHOULDER);
			SET_BUTTON(Button_Shoulder_R1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
			SET_BUTTON(Button_Start, XINPUT_GAMEPAD_START);
			SET_BUTTON(Button_Back, XINPUT_GAMEPAD_BACK);
			SET_BUTTON(Button_ThumbStick_Left, XINPUT_GAMEPAD_LEFT_THUMB);
			SET_BUTTON(Button_ThumbStick_Right, XINPUT_GAMEPAD_RIGHT_THUMB);
			
			gamepad.m_analogs[inGamepadAnalog_LeftStick_X] = inGamepad_StickToAnalog(xInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * 2);
			gamepad.m_analogs[inGamepadAnalog_LeftStick_Y] = inGamepad_StickToAnalog(xInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * 2);
			gamepad.m_analogs[inGamepadAnalog_RightStick_X] = inGamepad_StickToAnalog(xInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2);
			gamepad.m_analogs[inGamepadAnalog_RightStick_Y] = inGamepad_StickToAnalog(xInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2);

			gamepad.m_analogs[inGamepadAnalog_Shoulder_L2] = inGamepad_TriggerToAnalog(xInputState.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD * 2);
			gamepad.m_analogs[inGamepadAnalog_Shoulder_R2] = inGamepad_TriggerToAnalog(xInputState.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD * 2);

#define SET_BUTTON_FROM_ANALOG(engineCode, condition) \
	if (condition) \
		gamepad.m_buttons[inGamepad##engineCode] |= inGamepadButtonFlag_IsDown;

			SET_BUTTON_FROM_ANALOG(Button_Shoulder_L2, (gamepad.m_analogs[inGamepadAnalog_Shoulder_L2] > 0.5f));
			SET_BUTTON_FROM_ANALOG(Button_Shoulder_R2, (gamepad.m_analogs[inGamepadAnalog_Shoulder_R2] > 0.5f));

			SET_BUTTON_FROM_ANALOG(Button_LeftStick_Up, (gamepad.m_analogs[inGamepadAnalog_LeftStick_Y] > 0.5f));
			SET_BUTTON_FROM_ANALOG(Button_LeftStick_Down, (gamepad.m_analogs[inGamepadAnalog_LeftStick_Y] < -0.5f));
			SET_BUTTON_FROM_ANALOG(Button_LeftStick_Right, (gamepad.m_analogs[inGamepadAnalog_LeftStick_X] > 0.5f));
			SET_BUTTON_FROM_ANALOG(Button_LeftStick_Left, (gamepad.m_analogs[inGamepadAnalog_LeftStick_X] < -0.5f));

			SET_BUTTON_FROM_ANALOG(Button_RightStick_Up, (gamepad.m_analogs[inGamepadAnalog_RightStick_Y] > 0.5f));
			SET_BUTTON_FROM_ANALOG(Button_RightStick_Down, (gamepad.m_analogs[inGamepadAnalog_RightStick_Y] < -0.5f));
			SET_BUTTON_FROM_ANALOG(Button_RightStick_Right, (gamepad.m_analogs[inGamepadAnalog_RightStick_X] > 0.5f));
			SET_BUTTON_FROM_ANALOG(Button_RightStick_Left, (gamepad.m_analogs[inGamepadAnalog_RightStick_X] < -0.5f));
		}
		else
			ueMemSet(&gamepad, 0, sizeof(inGamepad));

		// Update force feedback

		if (gamepad.m_forceFeedback.m_isStarted &&
			ueClock_GetSecsSince(gamepad.m_forceFeedback.m_startTime) >= gamepad.m_forceFeedback.m_time)
			inGamepad_StopForceFeedback(id);
	}

	inGamepad_UpdateMergedState();
}


void inGamepad_EnableForceFeedback(u32 id, ueBool enable)
{
	s_data.m_gamepads[id].m_forceFeedback.m_enable = enable;
	if (!enable)
		inGamepad_StopForceFeedback(id);
}

ueBool inGamepad_IsForceFeedbackEnabled(u32 id)
{
	return s_data.m_gamepads[id].m_forceFeedback.m_enable;
}

void inGamepad_TriggerPadForceFeedback(u32 id, f32 leftStrength, f32 rightStrength, f32 time)
{
	if (!s_data.m_gamepads[id].m_forceFeedback.m_enable)
		return;

	XINPUT_VIBRATION padState;
	padState.wLeftMotorSpeed = (WORD) ueClamp((int) (leftStrength * 65535.0f), 0, 65535);
	padState.wRightMotorSpeed = (WORD) ueClamp((int) (rightStrength * 65535.0f), 0, 65535);
	XInputSetState(id, &padState);

	s_data.m_gamepads[id].m_forceFeedback.m_isStarted = UE_TRUE;
	s_data.m_gamepads[id].m_forceFeedback.m_startTime = ueClock_GetCurrent();
	s_data.m_gamepads[id].m_forceFeedback.m_time = time;
}

void inGamepad_StopForceFeedback(u32 id)
{
	XINPUT_VIBRATION padState;
	padState.wLeftMotorSpeed = 0;
	padState.wRightMotorSpeed = 0;
	XInputSetState(id, &padState);

	s_data.m_gamepads[id].m_forceFeedback.m_isStarted = UE_FALSE;
}
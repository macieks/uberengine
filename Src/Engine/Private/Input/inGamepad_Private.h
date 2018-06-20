#ifndef IN_GAMEPAD_PRIVATE_H
#define IN_GAMEPAD_PRIVATE_H

#include "Input/inSys.h"

//! Possible button state flags
enum inGamepadButtonFlag
{
	inGamepadButtonFlag_IsDown		= UE_POW2(0),
	inGamepadButtonFlag_WasDown		= UE_POW2(1)
};

struct inGamepadForceFeedback
{
	ueBool m_enable;
	ueBool m_isStarted;
	ueTime m_startTime;
	f32 m_time;

	inGamepadForceFeedback() : m_enable(UE_TRUE), m_isStarted(UE_FALSE) {}
};

//! Gamepad state
struct inGamepad
{
	ueBool m_isValid;
	f32 m_analogs[inGamepadAnalog_MAX];
	u8 m_buttons[inGamepadButton_MAX];

	inGamepadForceFeedback m_forceFeedback;

	inGamepad() : m_isValid(UE_FALSE) {}
};

struct inGamepadData
{
	inGamepad m_gamepads[IN_MAX_USERS + 1]; //!< Current state of all gamepads (including merged state)
};

extern inGamepadData s_data;

void inGamepad_UpdateMergedState();

#endif // IN_GAMEPAD_PRIVATE_H

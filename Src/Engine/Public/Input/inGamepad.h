#ifndef IN_GAMEPAD_H
#define IN_GAMEPAD_H

#include "Base/ueBase.h"

/**
 *	@addtogroup in
 *	@{
 */

//! Available gamepad buttons
enum inGamepadButton
{
	inGamepadButton_DPad_Up,
	inGamepadButton_DPad_Right,
	inGamepadButton_DPad_Down,
	inGamepadButton_DPad_Left,
	inGamepadButton_Y,
	inGamepadButton_B,
	inGamepadButton_A,
	inGamepadButton_X,
	inGamepadButton_Shoulder_L1,
	inGamepadButton_Shoulder_R1,
	inGamepadButton_Shoulder_L2,
	inGamepadButton_Shoulder_R2,
	inGamepadButton_Start,
	inGamepadButton_Back,
	inGamepadButton_LeftStick, //!< Press down left stick
	inGamepadButton_RightStick, //!< Press down right stick

	// Meta inGamepadButtons (translation from analogs)

	inGamepadButton_ThumbStick_Left,
	inGamepadButton_ThumbStick_Right,

	inGamepadButton_LeftStick_Up,
	inGamepadButton_LeftStick_Down,
	inGamepadButton_LeftStick_Left,
	inGamepadButton_LeftStick_Right,

	inGamepadButton_RightStick_Up,
	inGamepadButton_RightStick_Down,
	inGamepadButton_RightStick_Left,
	inGamepadButton_RightStick_Right,

	inGamepadButton_MAX,
};

//! Available analog input types
enum inGamepadAnalog
{
	inGamepadAnalog_LeftStick_X,	//!< Left stick X axis
	inGamepadAnalog_LeftStick_Y,	//!< Left stick Y axis
	inGamepadAnalog_RightStick_X,	//!< Right stick X axis
	inGamepadAnalog_RightStick_Y,	//!< Right stick Y axis
	inGamepadAnalog_Shoulder_L2,	//!< Shoulder left
	inGamepadAnalog_Shoulder_R2,	//!< Shoulder right

	inGamepadAnalog_MAX
};

//! Starts up gamepads
void inGamepad_Startup();
//! Shuts down gamepads
void inGamepad_Shutdown();
//! Updates state of all gamepads
void inGamepad_Update();
//! Gets gamepad button name
const char* inGamepad_GetButtonName(inGamepadButton button);

// State

//! Tells whether gamepad at given index is valid
ueBool inGamepad_IsValid(u32 userId);
//! Tells whether gamepad button is down
ueBool inGamepad_IsDown(u32 userId, inGamepadButton button);
//! Tells whether gamepad button was just pressed
ueBool inGamepad_WasPressed(u32 userId, inGamepadButton button);
//! Tells whether gamepad button was just released
ueBool inGamepad_WasReleased(u32 userId, inGamepadButton button);
//! Gets gamepad analog value
f32 inGamepad_GetAnalog(u32 userId, inGamepadAnalog analog);
//! Toggles force feedback effect of the gamepad
void inGamepad_EnableForceFeedback(u32 userId, ueBool enable);
//! Tells whether force feedback is enabled
ueBool inGamepad_IsForceFeedbackEnabled(u32 userId);
//! Triggers force feedback effect of the gamepad
void inGamepad_TriggerPadForceFeedback(u32 userId, f32 leftStrength, f32 rightStrength, f32 time);
//! Stops force feedback effect of the gamepad
void inGamepad_StopForceFeedback(u32 userId);

// Merged state

//! Tells whether any gamepad is valid
ueBool inGamepad_IsValidAny();
//! Tells whether any gamepad's button is down
ueBool inGamepad_IsDown(inGamepadButton button);
//! Tells whether any gamepad's button was just pressed
ueBool inGamepad_WasPressed(inGamepadButton button);
//! Tells whether any gamepad's button was just released
ueBool inGamepad_WasReleased(inGamepadButton button);
//! Gets max value of given analog among all gamepads
f32 inGamepad_GetAnalog(inGamepadAnalog analog);

// @}

#endif // IN_GAMEPAD_H

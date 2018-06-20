#ifndef IN_MOUSE_H
#define IN_MOUSE_H

#include "Base/ueBase.h"

/**
 *	@addtogroup in
 *	@{
 */

//! Available mouse buttons
enum inMouseButton
{
	inMouseButton_Left = 0,		//!< Left mouse button
	inMouseButton_Middle,		//!< Middle mouse button
	inMouseButton_Right,		//!< Right mouse button

	inMouseButton_MAX
};

//! Starts up all mouse support
void	inMouse_Startup();
//! Shuts down all mouse support
void	inMouse_Shutdown();
//! Updates mouse state
void	inMouse_Update(ueBool centerCursor = UE_TRUE);
//! Gets mouse button name
const char* inMouse_GetButtonName(inMouseButton button);

// Per user state

//! Tells whether mouse of given index is currently valid
ueBool	inMouse_IsValid(u32 userId);
//! Gets mouse position
void	inMouse_GetPos(u32 userId, s32& x, s32& y);
//! Gets mouse vector
void	inMouse_GetVector(u32 userId, s32& x, s32& y);
//! Gets mouse wheel delta
void	inMouse_GetWheelDelta(u32 userId, s32& delta);
//! Tells whether given mouse button is down
ueBool	inMouse_IsDown(u32 userId, inMouseButton button);
//! Tells whether given mouse button was down last frame
ueBool	inMouse_WasDown(u32 userId, inMouseButton button);
//! Tells whether given mouse button was just pressed
ueBool	inMouse_WasPressed(u32 userId, inMouseButton button);
//! Tells whether given mouse button was just released
ueBool	inMouse_WasReleased(u32 userId, inMouseButton button);

// Merged state (from all users)

//! Tells whether any mouse is currently valid
ueBool	inMouse_IsValidAny();
//! Gets mouse position
void	inMouse_GetPos(s32& x, s32& y);
//! Gets mouse movement vector
void	inMouse_GetVector(s32& x, s32& y);
//! Gets mouse wheel delta
void	inMouse_GetWheelDelta(s32& delta);
//! Tells whether given mouse button is down
ueBool	inMouse_IsDown(inMouseButton button);
//! Tells whether given mouse button was down last frame
ueBool	inMouse_WasDown(inMouseButton button);
//! Tells whether given mouse button was just pressed
ueBool	inMouse_WasPressed(inMouseButton button);
//! Tells whether given mouse button was just released
ueBool	inMouse_WasReleased(inMouseButton button);

// @}

#endif // IN_MOUSE_H

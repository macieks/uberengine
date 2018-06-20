#ifndef IN_MOUSE_PRIVATE_H
#define IN_MOUSE_PRIVATE_H

#include "Input/inMouse.h"

struct inMouseState
{
	ueBool m_isValid;
	ueBool m_hasInitialPos;	//!< Whether initial mouse position has been captured
	s32 m_initialPos[2];	//!< Initial mouse position restored when app gets closed (on some platforms)
	s32 m_vector[2];		//!< Mouse movement vector (since last update)
	s32 m_wheelDelta;		//!< Wheel delta
	s32 m_wheelDeltaAcc;	//!< Wheel delta accumulator
	s32 m_pos[2];			//!< Current mouse position
	s32 m_prevPos[2];		//!< Previous mouse position
	u16 m_buttons[inMouseButton_MAX]; //!< State of all mouse buttons for all users
};

void inMouse_ResetState();

extern inMouseState s_mouseState;

#endif // IN_MOUSE_PRIVATE_H

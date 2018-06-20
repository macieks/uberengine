#ifndef IN_KEYBOARD_PRIVATE_H
#define IN_KEYBOARD_PRIVATE_H

#include "Input/inSys.h"

#define IN_MAX_MAPPED_KEY   255
#define IN_UNMAPPED_KEY     ((u32) U32_MAX) //!< Indicates whether mapping for a key was done successfully (used for debugging only)

//! Possible key state flags
enum inKeyStateFlag
{
	inKeyStateFlag_IsDown		= UE_POW2(0),
	inKeyStateFlag_WasDown		= UE_POW2(1)
};

//! Internal scan code information
struct inKeyboardScanCode_Internal : inKeyboardScanCode
{
	u8 m_state[IN_MAX_USERS + 1]; //!< State for all keyboards (plus merged state)

	// State

	UE_INLINE ueBool IsDown(u32 userId) const { return m_state[userId] & inKeyStateFlag_IsDown; }
	UE_INLINE ueBool WasPressed(u32 userId) const { return m_state[userId] == inKeyStateFlag_IsDown; }
	UE_INLINE ueBool WasReleased(u32 userId) const { return m_state[userId] == inKeyStateFlag_WasDown; }

	// Merged state

	UE_INLINE ueBool IsDown() const { return m_state[IN_MAX_USERS] & inKeyStateFlag_IsDown; }
	UE_INLINE ueBool WasPressed() const { return m_state[IN_MAX_USERS] == inKeyStateFlag_IsDown; }
	UE_INLINE ueBool WasReleased() const { return m_state[IN_MAX_USERS] == inKeyStateFlag_WasDown; }
};

//! Keyboard state
struct inKeyboardState
{
	ueBool m_isValid;
	ueBool m_wasAutoRepeat;
	u8 m_keys[inKey_MAX];

	inKeyboardState() : m_isValid(UE_FALSE) {}
};

struct inKeyboardData
{
    ueBool m_isInitialized;

    inKeyboardState m_states[IN_MAX_USERS + 1];     //!< Current state of all keyboard devices (plus merged state)

    u32 m_numScanCodes;                             //!< Number of scan codes
    inKeyboardScanCode_Internal m_scanCodes[256];	//!< Scan codes

    const char* m_keyNames[inKey_MAX];              //!< Names of all keys
    u32 m_keyMapping[IN_MAX_MAPPED_KEY];            //!< Mapping between engine and platform specific keys (mapping direction depends on platform)

    inKeyboardData() : m_isInitialized(UE_FALSE) {}
};

extern inKeyboardData s_data;

// Private functionality

void inKeyboard_InitKeyNames();
void inKeyboard_ResetState(u32 userId, ueBool resetPreviousState = UE_TRUE, ueBool resetCurrentState = UE_TRUE);
void inKeyboard_ResetState();

void inKeyboard_InitDefaultScanCodes();
void inKeyboard_UpdateScanCodes(u32 userId);
void inKeyboard_UpdateMergedState();

#endif // IN_KEYBOARD_PRIVATE_H

#ifndef IN_KEYBOARD_H
#define IN_KEYBOARD_H

#include "Base/ueBase.h"

/**
 *	@addtogroup in
 *	@{
 */

//! All available keys; some of them possibly unsupported on some platforms or keyboard devices
enum inKey
{
	inKey_0 = 0,
	inKey_1,
	inKey_2,
	inKey_3,
	inKey_4,
	inKey_5,
	inKey_6,
	inKey_7,
	inKey_8,
	inKey_9,

	inKey_A,
	inKey_B,
	inKey_C,
	inKey_D,
	inKey_E,
	inKey_F,
	inKey_G,
	inKey_H,
	inKey_I,
	inKey_J,
	inKey_K,
	inKey_L,
	inKey_M,
	inKey_N,
	inKey_O,
	inKey_P,
	inKey_Q,
	inKey_R,
	inKey_S,
	inKey_T,
	inKey_U,
	inKey_V,
	inKey_W,
	inKey_X,
	inKey_Y,
	inKey_Z,

	inKey_F1,
	inKey_F2,
	inKey_F3,
	inKey_F4,
	inKey_F5,
	inKey_F6,
	inKey_F7,
	inKey_F8,
	inKey_F9,
	inKey_F10,
	inKey_F11,
	inKey_F12,
	inKey_F13,			//!<                     (NEC PC98)
	inKey_F14,			//!<                     (NEC PC98)
	inKey_F15,			//!<                     (NEC PC98)

	inKey_Numpad0,
	inKey_Numpad1,
	inKey_Numpad2,
	inKey_Numpad3,
	inKey_Numpad4,
	inKey_Numpad5,
	inKey_Numpad6,
	inKey_Numpad7,
	inKey_Numpad8,
	inKey_Numpad9,

	inKey_Escape,
	inKey_Minus,			//!< - (minus) on main keyboard
	inKey_Back,				//!< Backspace
	inKey_Tab,
	inKey_Return,			//!< Enter on main keyboard
	inKey_LeftControl,
	inKey_LeftShift,
	inKey_LeftBracket,
	inKey_RightBracket,
	inKey_Comma,
	inKey_Slash,
	inKey_Backslash,
	inKey_Period,			//!< . (full stop) on main keyboard
	inKey_RightShift,
	inKey_Multiply,			//!< * (star) on numeric keypad
	inKey_LeftAlt,			//!< Left Alt
	inKey_Space,
	inKey_Capital,
	inKey_Numlock,
	inKey_Scroll,			//!< Scroll Lock
	inKey_Subtract,			//!< - (minus) on numeric keypad
	inKey_Add,				//!< + (plus) on numeric keypad
	inKey_Decimal,			//!< . (decimal point) on numeric keypad
	inKey_Kana,				//!< (Japanese keyboard)
	inKey_Convert,			//!< (Japanese keyboard)
	inKey_NoConvert,		//!< (Japanese keyboard)
	inKey_NumpadEquals,		//!< = on numeric keypad (NEC PC98)
	inKey_Kanji,			//!< (Japanese keyboard)
	inKey_RightControl,
	inKey_Divide,			//!< / on numeric keypad
	inKey_RightAlt,			//!< Right Alt
	inKey_Home,				//!< Home on arrow keypad
	inKey_Pause,
	inKey_Up,				//!< UpArrow on arrow keypad
	inKey_Prior,			//!< PgUp on arrow keypad
	inKey_Left,				//!< LeftArrow on arrow keypad
	inKey_Right,			//!< RightArrow on arrow keypad
	inKey_End,				//!< End on arrow keypad
	inKey_Down,				//!< DownArrow on arrow keypad
	inKey_Next,				//!< PgDn on arrow keypad
	inKey_Insert,			//!< Insert on arrow keypad
	inKey_Delete,			//!< ueDelete on arrow keypad
	inKey_LeftWindows,		//!< Left Windows key
	inKey_RightWindows,		//!< Right Windows key
	inKey_AppMenu,			//!< AppMenu key
	inKey_Sleep,
	inKey_Tilde,			//!< `
	inKey_Equal,			//!< = on main keyboard
	inKey_PageUp,
	inKey_PageDown,
	inKey_Quotation,
	inKey_Semicolon,
	inKey_Shift,
	inKey_Control,
	inKey_Alt,

	inKey_MAX
};

//! Special key flags
enum inExtraKeyFlag
{
	inExtraKeyFlag_LeftShift	= UE_POW2(0),	//!< Left shift
	inExtraKeyFlag_RightShift	= UE_POW2(1),	//!< Right shift
	inExtraKeyFlag_LeftControl	= UE_POW2(2),	//!< Left control
	inExtraKeyFlag_RightControl	= UE_POW2(3),	//!< Right control
	inExtraKeyFlag_LeftAlt		= UE_POW2(4),	//!< Left alt
	inExtraKeyFlag_RightAlt		= UE_POW2(5),	//!< Right alt

	inExtraKeyFlag_Shift		= inExtraKeyFlag_LeftShift | inExtraKeyFlag_RightShift,		//!< Any shift
	inExtraKeyFlag_Control		= inExtraKeyFlag_LeftControl | inExtraKeyFlag_RightControl,	//!< Any control
	inExtraKeyFlag_Alt			= inExtraKeyFlag_LeftAlt | inExtraKeyFlag_RightAlt			//!< Any alt
};

//! Scan code information
struct inKeyboardScanCode
{
	inKey m_key;		//!< Key that must pressed
	u32 m_extraKeys;	//!< Special keys that must pressed @see enum inExtraKeyFlag
	char m_scanCode;	//!< Scan code value
};

//! Starts up keyboard support
void inKeyboard_Startup();
//! Shuts down keyboard support
void inKeyboard_Shutdown();
//! Updates keyboard
ueBool inKeyboard_Update();

// Per user state

//! Tells whether keyboard of given index is currently valid
ueBool inKeyboard_IsValid(u32 userId);
//! Tells whether given key for for given user is down
ueBool inKeyboard_IsDown(u32 userId, inKey key);
//! Tells whether given key for for given user has just been pressed
ueBool inKeyboard_WasPressed(u32 userId, inKey key);
//! Tells whether given key for for given user has just been released
ueBool inKeyboard_WasReleased(u32 userId, inKey key);

// Merged state (from all users)

//! Tells whether any keyboard is currently valid
ueBool inKeyboard_IsValidAny();
//! Tells whether given key is down for any user
ueBool inKeyboard_IsDown(inKey key);
//! Tells whether given key has just been pressed for any user
ueBool inKeyboard_WasPressed(inKey key);
//! Tells whether given key has just been released for any user
ueBool inKeyboard_WasReleased(inKey key);
//! Gets whether any key is being pressed for any user
ueBool inKeyboard_IsDownAnyKey();

// Scan codes

//! Gets number of scan codes
u32		inKeyboard_GetNumScanCodes();
//! Gets scan code description
const	inKeyboardScanCode& inKeyboard_GetScanCode(u32 index);
//! Tells whether given scan code is down
ueBool	inKeyboard_IsDownScanCode(u32 userId, u32 index);
//! Tells whether given scan code was just pressed
ueBool	inKeyboard_WasPressedScanCode(u32 userId, u32 index);
//! Tells whether given scan code was just released
ueBool	inKeyboard_WasReleasedScanCode(u32 userId, u32 index);
//! Tells whether given scan code is down (by any user)
ueBool	inKeyboard_IsDownScanCode(u32 index);
//! Tells whether given scan code has just been pressed (by any user)
ueBool	inKeyboard_WasPressedScanCode(u32 index);
//! Tells whether given scan code has just been released (by any user)
ueBool	inKeyboard_WasReleasedScanCode(u32 index);

// Misc.

//! Gets string name of the key
const char* inKeyboard_GetKeyName(inKey key);

// @}

#endif // IN_KEYBOARD_H

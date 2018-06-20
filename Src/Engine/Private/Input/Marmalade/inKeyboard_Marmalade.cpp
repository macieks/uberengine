#include "Input/inKeyboard.h"
#include "Input/inKeyboard_Private.h"

void inKeyboard_Startup()
{
	ueAssert(!s_data.m_isInitialized);

	// Set up key names

	inKeyboard_InitKeyNames();

	// Set up scan codes mapping

	inKeyboard_InitDefaultScanCodes();

	// Reset mapping for all keys

	for (u32 i = 0; i < inKey_MAX; ++i)
		s_data.m_keyMapping[i] = IN_UNMAPPED_KEY;

#define MAP_KEY(engineKeyName, win32KeyName) \
	ueAssert(win32KeyName < IN_MAX_MAPPED_KEY); \
	s_data.m_keyMapping[inKey_##engineKeyName] = win32KeyName;

	// Set up mapping for all keys

	for (u32 digitKey = inKey_0; digitKey <= inKey_9; ++digitKey)
		s_data.m_keyMapping[digitKey] = '0' + digitKey - inKey_0;

	for (u32 letterKey = inKey_A; letterKey <= inKey_Z; ++letterKey)
		s_data.m_keyMapping[letterKey] = 'A' + letterKey - inKey_A;

	MAP_KEY(Escape, s3eKeyEsc)
	MAP_KEY(Minus, s3eKeyMinus)
	MAP_KEY(Back, s3eKeyBack)
	MAP_KEY(Tab, s3eKeyTab)
	MAP_KEY(Return, s3eKeyEnter)
	MAP_KEY(LeftControl, s3eKeyLeftControl)
	MAP_KEY(LeftShift, s3eKeyLeftShift)
	MAP_KEY(Comma, s3eKeyComma)
	MAP_KEY(Period, s3eKeyPeriod)
	MAP_KEY(RightShift, s3eKeyRightShift)
	MAP_KEY(Multiply, s3eKeyStar)
	MAP_KEY(LeftAlt, s3eKeyLeftAlt)
	MAP_KEY(Space, s3eKeySpace)
	MAP_KEY(Capital, s3eKeyCapsLock)
	MAP_KEY(Numlock, s3eKeyNumLock)
	MAP_KEY(RightControl, s3eKeyRightControl)
	MAP_KEY(RightAlt, s3eKeyRightAlt)
	MAP_KEY(Home, s3eKeyHome)
	MAP_KEY(Pause, s3eKeyPause)
	MAP_KEY(Up, s3eKeyUp)
	MAP_KEY(Left, s3eKeyLeft)
	MAP_KEY(Right, s3eKeyRight)
	MAP_KEY(End, s3eKeyEnd)
	MAP_KEY(Down, s3eKeyDown)
	MAP_KEY(Insert, s3eKeyInsert)
	MAP_KEY(Delete, s3eKeyDelete)
	MAP_KEY(AppMenu, s3eKeyMenu)
	MAP_KEY(Equal, s3eKeyEquals)
	MAP_KEY(PageUp, s3eKeyPageUp)
	MAP_KEY(PageDown, s3eKeyPageDown)
	MAP_KEY(Semicolon, s3eKeySemicolon)
	MAP_KEY(Slash, s3eKeySlash)
	MAP_KEY(LeftBracket, s3eKeyLeftBracket);
	MAP_KEY(Backslash, s3eKeyBackSlash);
	MAP_KEY(RightBracket, s3eKeyRightBracket);

#undef MAP_KEY

	inKeyboard_ResetState();

	s_data.m_isInitialized = UE_TRUE;
}

void inKeyboard_Shutdown()
{
	ueAssert(s_data.m_isInitialized);
	s_data.m_isInitialized = UE_FALSE;
}

ueBool inKeyboard_Update()
{
	ueAssert(s_data.m_isInitialized);

	s3eKeyboardUpdate();

	for (u32 id = 0; id < IN_MAX_USERS; id++)
	{
		inKeyboardState& state = s_data.m_states[id];

		state.m_isValid = (id == 0);
		if (!state.m_isValid)
			continue;

		// Store old button states

		for (u32 key = 0; key < inKey_MAX; ++key)
			state.m_keys[key] = (state.m_keys[key] & inKeyStateFlag_IsDown) ? inKeyStateFlag_WasDown : 0;

		// Get current state

		for (u32 key = 0; key < inKey_MAX; ++key)
			if (s_data.m_keyMapping[key] != IN_UNMAPPED_KEY)
			{
				if (s3eKeyboardGetState((s3eKey) s_data.m_keyMapping[key]) & S3E_KEY_STATE_DOWN)
					state.m_keys[key] |= inKeyStateFlag_IsDown;
			}

		// Handle shift / control / alt

#define CASE_SPECIAL_KEY(key, refKey0, refKey1) \
	case key: \
		if ((state.m_keys[refKey0] & inKeyStateFlag_IsDown) || (state.m_keys[refKey1] & inKeyStateFlag_IsDown)) \
			state.m_keys[key] |= inKeyStateFlag_IsDown; \
		break;
		
		for (u32 key = 0; key < inKey_MAX; ++key)
			switch (key)
			{
				CASE_SPECIAL_KEY(inKey_Shift, inKey_LeftShift, inKey_RightShift)
				CASE_SPECIAL_KEY(inKey_Alt, inKey_LeftAlt, inKey_RightAlt)
				CASE_SPECIAL_KEY(inKey_Control, inKey_LeftControl, inKey_RightControl)
			}

		// Update scan codes

		inKeyboard_UpdateScanCodes(id);
	}

	inKeyboard_UpdateMergedState();
	return UE_TRUE;
}

ueBool inKeyboard_IsDownAnyKey()
{
	for (u32 i = 0; i < inKey_MAX; i++)
		if (inKeyboard_IsDown((inKey) i))
			return UE_TRUE;
	return UE_FALSE;
}
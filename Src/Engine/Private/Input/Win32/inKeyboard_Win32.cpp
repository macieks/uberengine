#include "Input/inKeyboard.h"
#include "Input/inKeyboard_Private.h"

void inKeyboard_Startup()
{
	UE_ASSERT(!s_data.m_isInitialized);

	// Set up key names

	inKeyboard_InitKeyNames();

	// Set up scan codes mapping

	inKeyboard_InitDefaultScanCodes();

	// Reset mapping for all keys

	for (u32 i = 0; i < inKey_MAX; ++i)
		s_data.m_keyMapping[i] = IN_UNMAPPED_KEY;

#define MAP_KEY(engineKeyName, win32KeyName) \
	UE_ASSERT(win32KeyName < IN_MAX_MAPPED_KEY); \
	s_data.m_keyMapping[inKey_##engineKeyName] = win32KeyName;

	// Set up mapping for all keys

	for (u32 digitKey = inKey_0; digitKey <= inKey_9; ++digitKey)
		s_data.m_keyMapping[digitKey] = '0' + digitKey - inKey_0;

	for (u32 letterKey = inKey_A; letterKey <= inKey_Z; ++letterKey)
		s_data.m_keyMapping[letterKey] = 'A' + letterKey - inKey_A;

	for (u32 functionKey = inKey_F1; functionKey <= inKey_F15; ++functionKey)
		s_data.m_keyMapping[functionKey] = VK_F1 + functionKey - inKey_F1;

	for (u32 numPadKey = inKey_Numpad0; numPadKey <= inKey_Numpad9; ++numPadKey)
		s_data.m_keyMapping[numPadKey] = VK_NUMPAD0 + numPadKey - inKey_Numpad0;

	MAP_KEY(Escape, VK_ESCAPE)
	MAP_KEY(Minus, VK_OEM_MINUS)
	MAP_KEY(Back, VK_BACK)
	MAP_KEY(Tab, VK_TAB)
	MAP_KEY(Return, VK_RETURN)
	MAP_KEY(LeftControl, VK_LCONTROL)
	MAP_KEY(LeftShift, VK_LSHIFT)
	MAP_KEY(Comma, VK_OEM_COMMA)
	MAP_KEY(Period, VK_OEM_PERIOD)
	MAP_KEY(RightShift, VK_RSHIFT)
	MAP_KEY(Multiply, VK_MULTIPLY)
	MAP_KEY(LeftAlt, VK_LMENU)
	MAP_KEY(Space, VK_SPACE)
	MAP_KEY(Capital, VK_CAPITAL)
	MAP_KEY(Numlock, VK_NUMLOCK)
	MAP_KEY(Scroll, VK_SCROLL)
	MAP_KEY(Subtract, VK_SUBTRACT)
	MAP_KEY(Add, VK_ADD)
	MAP_KEY(Decimal, VK_DECIMAL)
	MAP_KEY(Kana, VK_KANA)
	MAP_KEY(Convert, VK_CONVERT)
	MAP_KEY(NoConvert, VK_NONCONVERT)
	MAP_KEY(NumpadEquals, VK_OEM_NEC_EQUAL)
	MAP_KEY(Kanji, VK_KANJI)
	MAP_KEY(RightControl, VK_RCONTROL)
	MAP_KEY(Divide, VK_DIVIDE)
	MAP_KEY(RightAlt, VK_RMENU)
	MAP_KEY(Home, VK_HOME)
	MAP_KEY(Pause, VK_PAUSE)
	MAP_KEY(Up, VK_UP)
	MAP_KEY(Prior, VK_PRIOR)
	MAP_KEY(Left, VK_LEFT)
	MAP_KEY(Right, VK_RIGHT)
	MAP_KEY(End, VK_END)
	MAP_KEY(Down, VK_DOWN)
	MAP_KEY(Next, VK_NEXT)
	MAP_KEY(Insert, VK_INSERT)
	MAP_KEY(Delete, VK_DELETE)
	MAP_KEY(LeftWindows, VK_LWIN)
	MAP_KEY(RightWindows, VK_RWIN)
	MAP_KEY(AppMenu, VK_APPS)
	MAP_KEY(Sleep, VK_SLEEP)
	MAP_KEY(Equal, VK_OEM_PLUS)
	MAP_KEY(PageUp, VK_PRIOR)
	MAP_KEY(PageDown, VK_NEXT)
	MAP_KEY(Semicolon, VK_OEM_1)
	MAP_KEY(Slash, VK_OEM_2)
	MAP_KEY(Tilde, VK_OEM_3)
	MAP_KEY(LeftBracket, VK_OEM_4);
	MAP_KEY(Backslash, VK_OEM_5);
	MAP_KEY(RightBracket, VK_OEM_6);
	MAP_KEY(Quotation, VK_OEM_7)

#undef MAP_KEY

	inKeyboard_ResetState();

	s_data.m_isInitialized = UE_TRUE;
}

void inKeyboard_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	s_data.m_isInitialized = UE_FALSE;
}

ueBool inKeyboard_Update()
{
	UE_ASSERT(s_data.m_isInitialized);
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
				if (GetAsyncKeyState( s_data.m_keyMapping[key] ) & 0x8000)
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
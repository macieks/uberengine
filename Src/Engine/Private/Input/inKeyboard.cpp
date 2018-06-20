#include "Input/inKeyboard.h"
#include "Input/inKeyboard_Private.h"

inKeyboardData s_data;

ueBool inKeyboard_IsValid(u32 userId) { return s_data.m_states[userId].m_isValid; }
ueBool inKeyboard_IsDown(u32 userId, inKey key) { return s_data.m_states[userId].m_keys[key] & inKeyStateFlag_IsDown; }
ueBool inKeyboard_WasPressed(u32 userId, inKey key) { return s_data.m_states[userId].m_keys[key] == inKeyStateFlag_IsDown; }
ueBool inKeyboard_WasReleased(u32 userId, inKey key) { return s_data.m_states[userId].m_keys[key] == inKeyStateFlag_WasDown; }

// Merged state

ueBool inKeyboard_IsValidAny() { return s_data.m_states[IN_MAX_USERS].m_isValid; }
ueBool inKeyboard_IsDown(inKey key) { return s_data.m_states[IN_MAX_USERS].m_keys[key] & inKeyStateFlag_IsDown; }
ueBool inKeyboard_WasPressed(inKey key) { return s_data.m_states[IN_MAX_USERS].m_keys[key] == inKeyStateFlag_IsDown; }
ueBool inKeyboard_WasReleased(inKey key) { return s_data.m_states[IN_MAX_USERS].m_keys[key] == inKeyStateFlag_WasDown; }

// Misc.

const char* inKeyboard_GetKeyName(inKey key) { return s_data.m_keyNames[key]; }

// Private

void inKeyboard_InitKeyNames()
{
#define SET_KEY_NAME(engineKeyName) s_data.m_keyNames[inKey_##engineKeyName] = (const char*) #engineKeyName;

	SET_KEY_NAME(0);
	SET_KEY_NAME(1);
	SET_KEY_NAME(2);
	SET_KEY_NAME(3);
	SET_KEY_NAME(4);
	SET_KEY_NAME(5);
	SET_KEY_NAME(6);
	SET_KEY_NAME(7);
	SET_KEY_NAME(8);
	SET_KEY_NAME(9);

	SET_KEY_NAME(A);
	SET_KEY_NAME(B);
	SET_KEY_NAME(C);
	SET_KEY_NAME(D);
	SET_KEY_NAME(E);
	SET_KEY_NAME(F);
	SET_KEY_NAME(G);
	SET_KEY_NAME(H);
	SET_KEY_NAME(I);
	SET_KEY_NAME(J);
	SET_KEY_NAME(K);
	SET_KEY_NAME(L);
	SET_KEY_NAME(M);
	SET_KEY_NAME(N);
	SET_KEY_NAME(O);
	SET_KEY_NAME(P);
	SET_KEY_NAME(Q);
	SET_KEY_NAME(R);
	SET_KEY_NAME(S);
	SET_KEY_NAME(T);
	SET_KEY_NAME(U);
	SET_KEY_NAME(V);
	SET_KEY_NAME(W);
	SET_KEY_NAME(X);
	SET_KEY_NAME(Y);
	SET_KEY_NAME(Z);

	SET_KEY_NAME(F1);
	SET_KEY_NAME(F2);
	SET_KEY_NAME(F3);
	SET_KEY_NAME(F4);
	SET_KEY_NAME(F5);
	SET_KEY_NAME(F6);
	SET_KEY_NAME(F7);
	SET_KEY_NAME(F8);
	SET_KEY_NAME(F9);
	SET_KEY_NAME(F10);
	SET_KEY_NAME(F11);
	SET_KEY_NAME(F12);
	SET_KEY_NAME(F13);			//                     (NEC PC98)
	SET_KEY_NAME(F14);			//                     (NEC PC98)
	SET_KEY_NAME(F15);			//                     (NEC PC98)

	SET_KEY_NAME(Numpad0);
	SET_KEY_NAME(Numpad1);
	SET_KEY_NAME(Numpad2);
	SET_KEY_NAME(Numpad3);
	SET_KEY_NAME(Numpad4);
	SET_KEY_NAME(Numpad5);
	SET_KEY_NAME(Numpad6);
	SET_KEY_NAME(Numpad7);
	SET_KEY_NAME(Numpad8);
	SET_KEY_NAME(Numpad9);

	SET_KEY_NAME(Escape);
	SET_KEY_NAME(Minus);			//!< - (minus) on main keyboard
	SET_KEY_NAME(Back);			//!< Backspace
	SET_KEY_NAME(Tab);
	SET_KEY_NAME(Return);			//!< Enter on main keyboard
	SET_KEY_NAME(LeftControl);
	SET_KEY_NAME(LeftShift);
	SET_KEY_NAME(Comma);
	SET_KEY_NAME(Slash);
	SET_KEY_NAME(Backslash);
	SET_KEY_NAME(Period);			//!< . (full stop) on main keyboard
	SET_KEY_NAME(RightShift);
	SET_KEY_NAME(Multiply);		//!< * (star) on numeric keypad
	SET_KEY_NAME(LeftAlt);		//!< Left Alt
	SET_KEY_NAME(Space);
	SET_KEY_NAME(Capital);
	SET_KEY_NAME(Numlock);
	SET_KEY_NAME(Scroll);			//!< Scroll Lock
	SET_KEY_NAME(Subtract);		//!< - (minus) on numeric keypad
	SET_KEY_NAME(Add);			//!< + (plus) on numeric keypad
	SET_KEY_NAME(Decimal);		//!< . (decimal point) on numeric keypad
	SET_KEY_NAME(Kana);			// (Japanese keyboard)
	SET_KEY_NAME(Convert);		// (Japanese keyboard)
	SET_KEY_NAME(NoConvert);		// (Japanese keyboard)
	SET_KEY_NAME(NumpadEquals);	// = on numeric keypad (NEC PC98)
	SET_KEY_NAME(Kanji);			// (Japanese keyboard)
	SET_KEY_NAME(RightControl);
	SET_KEY_NAME(Divide);			// / on numeric keypad
	SET_KEY_NAME(RightAlt);		// Right Alt
	SET_KEY_NAME(Home);			// Home on arrow keypad
	SET_KEY_NAME(Pause);
	SET_KEY_NAME(Up);				// UpArrow on arrow keypad
	SET_KEY_NAME(Prior);			// PgUp on arrow keypad
	SET_KEY_NAME(Left);			// LeftArrow on arrow keypad
	SET_KEY_NAME(Right);			// RightArrow on arrow keypad
	SET_KEY_NAME(End);			// End on arrow keypad
	SET_KEY_NAME(Down);			// DownArrow on arrow keypad
	SET_KEY_NAME(Next);			// PgDn on arrow keypad
	SET_KEY_NAME(Insert);			// Insert on arrow keypad
	SET_KEY_NAME(Delete);			// Delete on arrow keypad
	SET_KEY_NAME(LeftWindows);	// Left Windows key
	SET_KEY_NAME(RightWindows);	// Right Windows key
	SET_KEY_NAME(AppMenu);		// AppMenu key
	SET_KEY_NAME(Sleep);
	SET_KEY_NAME(Tilde);			//!< `
	SET_KEY_NAME(Equal);			// = on main keyboard
	SET_KEY_NAME(PageUp);
	SET_KEY_NAME(PageDown);
	SET_KEY_NAME(Quotation);
	SET_KEY_NAME(Semicolon);
}

void inKeyboard_ResetState()
{
	for (u32 userId = 0; userId < IN_MAX_USERS + 1; ++userId)
		inKeyboard_ResetState(userId);
}

void inKeyboard_ResetState(u32 userId, ueBool resetPreviousState, ueBool resetCurrentState)
{
	if (resetPreviousState)
	{
		for (u32 i = 0; i < inKey_MAX; ++i)
			s_data.m_states[userId].m_keys[i] &= ~inKeyStateFlag_WasDown;

		for (u32 i = 0; i < s_data.m_numScanCodes; i++)
			s_data.m_scanCodes[i].m_state[userId] &= ~inKeyStateFlag_WasDown;
	}

	if (resetCurrentState)
	{
		for (u32 i = 0; i < inKey_MAX; ++i)
			s_data.m_states[userId].m_keys[i] &= ~inKeyStateFlag_IsDown;

		for (u32 i = 0; i < s_data.m_numScanCodes; i++)
			s_data.m_scanCodes[i].m_state[userId] &= ~inKeyStateFlag_IsDown;
	}
}

void inKeyboard_InitDefaultScanCodes()
{
	s_data.m_numScanCodes = 0;

#define INIT_SCAN_CODE(scanCode, key, extraKeys) \
	s_data.m_scanCodes[s_data.m_numScanCodes].m_scanCode = scanCode; \
	s_data.m_scanCodes[s_data.m_numScanCodes].m_key = key; \
	s_data.m_scanCodes[s_data.m_numScanCodes].m_extraKeys = extraKeys; \
	s_data.m_numScanCodes++;

	for (u32 key = inKey_A; key <= inKey_Z; key++)
	{
		INIT_SCAN_CODE(key - inKey_A + 'a', (inKey) key, 0);
		INIT_SCAN_CODE(key - inKey_A + 'A', (inKey) key, inExtraKeyFlag_Shift);
	}

	for (u32 key = inKey_0; key <= inKey_9; key++)
	{
		INIT_SCAN_CODE(key - inKey_0 + '0', (inKey) key, 0);
	}

	INIT_SCAN_CODE('!', inKey_1, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('@', inKey_2, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('#', inKey_3, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('$', inKey_4, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('%', inKey_5, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('^', inKey_6, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('&', inKey_7, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('*', inKey_8, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('(', inKey_9, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE(')', inKey_0, inExtraKeyFlag_Shift);

	INIT_SCAN_CODE(',', inKey_Comma, 0);
	INIT_SCAN_CODE('.', inKey_Period, 0);
	INIT_SCAN_CODE('/', inKey_Slash, 0);
	INIT_SCAN_CODE('\'', inKey_Quotation, 0);
	INIT_SCAN_CODE('-', inKey_Minus, 0);
	INIT_SCAN_CODE('=', inKey_Equal, 0);
	INIT_SCAN_CODE(';', inKey_Semicolon, 0);
	INIT_SCAN_CODE('[', inKey_LeftBracket, 0);
	INIT_SCAN_CODE(']', inKey_RightBracket, 0);

	INIT_SCAN_CODE('<', inKey_Comma, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('>', inKey_Period, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('?', inKey_Slash, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('"', inKey_Quotation, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('_', inKey_Minus, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('+', inKey_Equal, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE(':', inKey_Semicolon, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('|', inKey_Backslash, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('{', inKey_LeftBracket, inExtraKeyFlag_Shift);
	INIT_SCAN_CODE('}', inKey_RightBracket, inExtraKeyFlag_Shift);

	INIT_SCAN_CODE(' ', inKey_Space, 0);
}

void inKeyboard_UpdateScanCodes(u32 userId)
{
	for (u32 code = 0; code < s_data.m_numScanCodes; ++code)
	{
		inKeyboardScanCode_Internal& scanCode = s_data.m_scanCodes[code];

		const ueBool wasDown = scanCode.IsDown(userId);
		scanCode.m_state[userId] = (scanCode.m_state[userId] & inKeyStateFlag_IsDown) ? inKeyStateFlag_WasDown : 0;

		ueBool isExtraDown = UE_TRUE;
		if ((scanCode.m_extraKeys & inExtraKeyFlag_Shift) == inExtraKeyFlag_Shift)
			isExtraDown = isExtraDown && inKeyboard_IsDown(userId, inKey_Shift);
		else
		{
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_LeftShift) != 0) == inKeyboard_IsDown(userId, inKey_LeftShift);
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_RightShift) != 0) == inKeyboard_IsDown(userId, inKey_RightShift);
		}

		if ((scanCode.m_extraKeys & inExtraKeyFlag_Control) == inExtraKeyFlag_Control)
			isExtraDown = isExtraDown && inKeyboard_IsDown(userId, inKey_Control);
		else
		{
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_LeftControl) != 0) == inKeyboard_IsDown(userId, inKey_LeftControl);
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_RightControl) != 0) == inKeyboard_IsDown(userId, inKey_RightControl);
		}

		if ((scanCode.m_extraKeys & inExtraKeyFlag_Alt) == inExtraKeyFlag_Alt)
			isExtraDown = isExtraDown && inKeyboard_IsDown(userId, inKey_Alt);
		else
		{
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_LeftAlt) != 0) == inKeyboard_IsDown(userId, inKey_LeftAlt);
			isExtraDown = isExtraDown && ((scanCode.m_extraKeys & inExtraKeyFlag_RightAlt) != 0) == inKeyboard_IsDown(userId, inKey_RightAlt);
		}

		if (isExtraDown && inKeyboard_IsDown(userId, scanCode.m_key) && (inKeyboard_WasPressed(userId, scanCode.m_key) || wasDown))
			scanCode.m_state[userId] |= inKeyStateFlag_IsDown;
	}
}

void inKeyboard_UpdateMergedState()
{
	inKeyboardState& mergedState = s_data.m_states[IN_MAX_USERS];

	// Store old button states

	for (u32 key = 0; key < inKey_MAX; ++key)
		mergedState.m_keys[key] = (mergedState.m_keys[key] & inKeyStateFlag_IsDown) ? inKeyStateFlag_WasDown : 0;

	// Get current state

	mergedState.m_isValid = UE_FALSE;

	for (u32 i = 0; i < IN_MAX_USERS; i++)
	{
	    inKeyboardState& state = s_data.m_states[i];
		if (state.m_isValid)
		{
			mergedState.m_isValid = UE_TRUE;
			for (u32 key = 0; key < inKey_MAX; ++key)
				if (state.m_keys[key] & inKeyStateFlag_IsDown)
					mergedState.m_keys[key] |= inKeyStateFlag_IsDown;
		}
	}

	inKeyboard_UpdateScanCodes(IN_MAX_USERS);
}

u32		inKeyboard_GetNumScanCodes() { return s_data.m_numScanCodes; }
const	inKeyboardScanCode& inKeyboard_GetScanCode(u32 index) { UE_ASSERT(index < s_data.m_numScanCodes); return s_data.m_scanCodes[index]; }

ueBool	inKeyboard_IsDownScanCode(u32 userId, u32 index) { return s_data.m_scanCodes[index].IsDown(userId); }
ueBool	inKeyboard_WasPressedScanCode(u32 userId, u32 index) { return s_data.m_scanCodes[index].WasPressed(userId); }
ueBool	inKeyboard_WasReleasedScanCode(u32 userId, u32 index) { return s_data.m_scanCodes[index].WasReleased(userId); }

ueBool	inKeyboard_IsDownScanCode(u32 index) { return s_data.m_scanCodes[index].IsDown(IN_MAX_USERS); }
ueBool	inKeyboard_WasPressedScanCode(u32 index) { return s_data.m_scanCodes[index].WasPressed(IN_MAX_USERS); }
ueBool	inKeyboard_WasReleasedScanCode(u32 index) { return s_data.m_scanCodes[index].WasReleased(IN_MAX_USERS); }
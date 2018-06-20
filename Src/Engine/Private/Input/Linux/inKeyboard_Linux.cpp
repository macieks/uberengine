#include "Input/inKeyboard.h"
#include "Input/inKeyboard_Private.h"
#include "Utils/Window.h"

#include <X11/keysym.h>
#include <X11/Xutil.h>

// X11 stuff
//Window s_window;
//Display* s_display;

void inKeyboard_Startup()
{
	ueAssert(!s_data.m_isInitialized);

	// Set up key names

	inKeyboard_InitKeyNames();

	// Set up scan codes mapping

	inKeyboard_InitDefaultScanCodes();

	// Create mapping for X11 keys

	for (u32 i = 0; i < inKey_MAX; ++i)
		s_data.m_keyMapping[i] = IN_UNMAPPED_KEY;

#define MAP_KEY(dstKey, srcKey) \
    ueAssert(srcKey < IN_MAX_MAPPED_KEY); \
    s_data.m_keyMapping[srcKey] = inKey_##dstKey;

#define MAP_KEY_RANGE(dstKeyStart, dstKeyEnd, srcKeyStart, srcKeyEnd) \
    ueAssert(srcKeyEnd < IN_MAX_MAPPED_KEY); \
    ueAssert(dstKeyEnd < IN_MAX_MAPPED_KEY); \
    ueAssert(srcKeyEnd - srcKeyStart == dstKeyEnd - dstKeyStart); \
    for (u32 srcKey = srcKeyStart, dstKey = dstKeyStart; srcKey <= srcKeyEnd; srcKey++, dstKey++) \
        s_data.m_keyMapping[srcKey] = dstKey;

	MAP_KEY_RANGE(inKey_0, inKey_9, XK_0, XK_9)
	MAP_KEY_RANGE(inKey_A, inKey_Z, XK_a, XK_z)
	MAP_KEY_RANGE(inKey_F1, inKey_F15, XK_F1, XK_F15)
	MAP_KEY_RANGE(inKey_Numpad0, inKey_Numpad9, XK_KP_0, XK_KP_9)

	MAP_KEY(Escape, XK_Escape)
	MAP_KEY(Minus, XK_minus)
	MAP_KEY(Back, XK_BackSpace)
	MAP_KEY(Tab, XK_Tab)
	MAP_KEY(Return, XK_Return)
	MAP_KEY(LeftControl, XK_Control_L)
	MAP_KEY(LeftShift, XK_Shift_L)
	MAP_KEY(Comma, XK_comma)
	MAP_KEY(Period, XK_period)
	MAP_KEY(RightShift, XK_Shift_R)
	MAP_KEY(Multiply, XK_multiply)
	MAP_KEY(LeftAlt, XK_Alt_L)
	MAP_KEY(Space, XK_space)
	MAP_KEY(Capital, XK_Caps_Lock)
	MAP_KEY(Numlock, XK_Num_Lock)
	MAP_KEY(Scroll, XK_Scroll_Lock)
	MAP_KEY(Subtract, XK_minus)
	MAP_KEY(Add, XK_plus)
	MAP_KEY(RightControl, XK_Control_R)
	MAP_KEY(Divide, XK_slash)
	MAP_KEY(RightAlt, XK_Alt_R)
	MAP_KEY(Home, XK_Home)
	MAP_KEY(Pause, XK_Pause)
	MAP_KEY(Up, XK_Up)
	MAP_KEY(Left, XK_Left)
	MAP_KEY(Right, XK_Right)
	MAP_KEY(End, XK_End)
	MAP_KEY(Down, XK_Down)
	MAP_KEY(Insert, XK_Insert)
	MAP_KEY(Delete, XK_Delete)
	MAP_KEY(LeftWindows, XK_Super_L)
	MAP_KEY(RightWindows, XK_Super_R)
	MAP_KEY(AppMenu, XK_Menu)
	MAP_KEY(Slash, XK_slash)
	MAP_KEY(Equal, XK_equal)
	MAP_KEY(PageUp, XK_Page_Up)
	MAP_KEY(PageDown, XK_Page_Down)

#undef MAP_KEY

	inKeyboard_ResetState();

    // Initialize X11 keyboard

    ueWindow* mainWindow = ueWindow_GetMain();

    Display* display = ueWindow_GetXDisplay();
    Window window = ueWindow_GetXWindow(mainWindow);

	int res = XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
	ueAssert(res != BadWindow);

	XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);

	// Make sure ueto-repeat is disabled

	XKeyboardState old;
    XGetKeyboardControl(display, &old);
    s_data.m_states[0].m_wasAutoRepeat = (old.global_auto_repeat == AutoRepeatModeOn);
    if (s_data.m_states[0].m_wasAutoRepeat)
        XAutoRepeatOff(display);

	s_data.m_isInitialized = UE_TRUE;
}

void inKeyboard_Shutdown()
{
	ueAssert(s_data.m_isInitialized);

    Display* display = ueWindow_GetXDisplay();

	if (s_data.m_states[0].m_wasAutoRepeat)
        XAutoRepeatOn(display);

	XUngrabKeyboard(display, CurrentTime);

	s_data.m_isInitialized = UE_FALSE;
}

ueBool inKeyboard_Update()
{
	ueAssert(s_data.m_isInitialized);

	Display* display = ueWindow_GetXDisplay();

	for (u32 id = 0; id < IN_MAX_USERS; id++)
	{
	    inKeyboardState& state = s_data.m_states[id];

		state.m_isValid = (id == 0);
		if (!state.m_isValid)
			continue;

		// Store old button states

		for (u32 key = 0; key < inKey_MAX; ++key)
			state.m_keys[key] = (state.m_keys[key] & inKeyStateFlag_IsDown) ? inKeyStateFlag_WasDown : 0;

		// Process events

        KeySym key;
        XEvent event;

        while (XPending(display) > 0)
        {
            XNextEvent(display, &event);

            if (event.xkey.keycode >= IN_MAX_MAPPED_KEY)
                continue;
            const u32 key = s_data.m_keyMapping[event.xkey.keycode];
            if (key == IN_UNMAPPED_KEY)
                continue;

            if (KeyPress == event.type)
                state.m_keys[key] |= inKeyStateFlag_IsDown;
            else if (KeyRelease == event.type)
                state.m_keys[key] &= ~inKeyStateFlag_IsDown;
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

	// Update merged state

	inKeyboard_UpdateMergedState();

	return UE_TRUE;
}

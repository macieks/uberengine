#ifndef UE_WINDOW_H
#define UE_WINDOW_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ue
 *	@{
 */

#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC)

#define UE_HAS_WINDOWS

#if defined(UE_LINUX)
    #include <X11/keysym.h>
    #include <X11/Xutil.h>
#endif

#include "Base/ueShapes.h"

/**
 *	@struct ueWindow
 *	@brief Native operating system window
 */
struct ueWindow;

//! User supplied callback function for when window is resized
typedef void (*ueWindowResizeCallback)(ueWindow* window, const ueRectI& workArea);

//! Native OS window creation parameters
struct ueWindowParams
{
	ueAllocator* m_allocator;	//!< Allocator to be used for this window

	ueRectI m_rect;		//!< Working area (i.e. excluding borders, title bar etc)

	u32 m_bitsPerPixel;	//!< Bits per pixel (typically 32)
	u32 m_frequency;	//!< Refresh rate

	u32 m_minWidth;		//!< Min. width
	u32 m_minHeight;	//!< Min. height
	u32 m_maxWidth;		//!< Min. width
	u32 m_maxHeight;	//!< Max. height

	ueBool m_fullscreen;//!< Fullscreen on or off

	char m_name[64];	//!< Name

	ueBool m_hideCursorWhenActive;	//!< Hide cursor when window is active

	ueBool m_resizeable;	//!< Window is resizeable
	ueBool m_maximizable;	//!< Window is maximizable
	ueBool m_minimizable;	//!< Window is minimizable

	ueBool m_visible;		//!< Window is visible

	ueWindowResizeCallback m_resizeCallback;	//!< Resize callback

#if defined(UE_WIN32)
	HINSTANCE m_hInstance;	//!< Win32 HINSTANCE
	char m_className[64];	//!< Win32 class name
	const char* m_icon;		//!< Win32 icon resource ID
	const char* m_smallIcon;		//!< Win32 small icon resource ID
#endif

	ueWindowParams() :
		m_allocator(NULL),
		m_rect(64, 64, 256, 256),
		m_bitsPerPixel(32),
		m_frequency(60),
		m_minWidth(64), m_minHeight(64),
		m_maxWidth(2048), m_maxHeight(2048),
		m_fullscreen(UE_FALSE),
		m_hideCursorWhenActive(UE_FALSE),
		m_resizeable(UE_FALSE),
		m_maximizable(UE_FALSE),
		m_minimizable(UE_TRUE),
		m_visible(UE_TRUE),
		m_resizeCallback(NULL)
#if defined(UE_WIN32)
		,m_hInstance(NULL),
		m_icon(IDI_APPLICATION),
		m_smallIcon(IDI_APPLICATION)
#endif
	{}
};

//! Creates window
ueWindow*		ueWindow_Create(ueWindowParams* params);
//! Destroys window
void			ueWindow_Destroy(ueWindow* window);

//! Gets whether window has focus
ueBool			ueWindow_HasFocus(ueWindow* window);
//! Gets total window area (including all borders and title bar)
const ueRectI&	ueWindow_GetTotalArea(ueWindow* window);
//! Gets window working area (excluding all borders and title bar)
const ueRectI&	ueWindow_GetWorkArea(ueWindow* window);
//! Gets window parameters
const ueWindowParams* ueWindow_GetParams(ueWindow* window);

//! Sets window name
void			ueWindow_SetName(ueWindow* window, const char* name);
//! Changes window mode (fullscreen, dimensions, frequency)
ueBool			ueWindow_ChangeMode(ueWindow* window, ueBool fullscreen, u32 width = 0, u32 height = 0, u32 frequency = 0);
//! Minimizes window
void			ueWindow_Minimize(ueWindow* window);

//! Adds window resize callback
void			ueWindow_AddResizeCallback(ueWindow* window, ueWindowResizeCallback callback);
//! Removes window resize callback
void			ueWindow_RemoveResizeCallback(ueWindow* window, ueWindowResizeCallback callback);

//! Sets main window
void			ueWindow_SetMain(ueWindow* window);
//! Gets main window
ueWindow*		ueWindow_GetMain();

#if defined(UE_WIN32)

//! Gets window Win32 handle
HWND			ueWindow_GetHWND(ueWindow* window);

#elif defined(UE_LINUX)

//! Gets Linux window
Window			ueWindow_GetXWindow(ueWindow* window);
//! Gets Linux display
Display*		ueWindow_GetXDisplay();

#endif // defined(UE_<SKU>)

//! Handles native OS messages for all created windows
void					ueWindowUtils_HandleOSMessages();

#else // defined(UE_HAS_WINDOWS)

struct ueWindow;

#endif // defined(UE_HAS_WINDOWS)

//! Native OS message box
enum ueWindowMsgBoxType
{
	ueWindowMsgBoxType_Info = 0,			//!< Information
	ueWindowMsgBoxType_QuestionOkCancel,	//!< Question with 2 buttons: ok and cancel
	ueWindowMsgBoxType_QuestionYesNo,		//!< Question with 2 buttons: yes and no
	ueWindowMsgBoxType_QuestionYesNoCancel,	//!< Question with 3 buttons: yes, no and cancel
	ueWindowMsgBoxType_Warning,				//!< Warning
	ueWindowMsgBoxType_Error,				//!< Error

	ueWindowMsgBoxType_MAX
};

//! Native OS message box result
enum ueWindowMsgBoxResult
{
	ueWindowMsgBoxResult_Ok = 0,	//!< Ok
	ueWindowMsgBoxResult_Cancel,	//!< Cancel
	ueWindowMsgBoxResult_Yes,		//!< Yes
	ueWindowMsgBoxResult_No,		//!< No

	ueWindowMsgBoxResult_MAX
};

//! Shows native OS message box on top of given window
ueWindowMsgBoxResult	ueWindowUtils_ShowMessageBox(ueWindow* window, const char* message, const char* title, ueWindowMsgBoxType type = ueWindowMsgBoxType_Info);

// @}

#endif // UE_WINDOW_H
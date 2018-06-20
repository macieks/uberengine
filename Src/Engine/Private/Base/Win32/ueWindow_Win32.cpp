#include "Base/Containers/ueList.h"
#include "Base/ueWindow.h"
#include "Input/inMouse_Private.h"

struct ueWindow : ueList<ueWindow>::Node
{
	HWND m_hwnd;
	ueBool m_hasFocus;
	ueWindowParams m_params;
	ueRectI m_rect;
	ueWindowResizeCallback m_extraResizeCallback;

	ueWindow() :
		m_hwnd(NULL),
		m_hasFocus(UE_FALSE),
		m_extraResizeCallback(NULL)
	{}
};

static ueList<ueWindow> s_windows;
static ueWindow* s_mainWindow = NULL;

void ueWindow_UpdateRects(ueWindow* window)
{
	WINDOWINFO info;
	if (GetWindowInfo(window->m_hwnd, &info))
	{
		window->m_params.m_rect.m_left = info.rcClient.left;
		window->m_params.m_rect.m_width = info.rcClient.right - info.rcClient.left;
		window->m_params.m_rect.m_top = info.rcClient.top;
		window->m_params.m_rect.m_height = info.rcClient.bottom - info.rcClient.top;

//		ueLogD("NEW CLIENT RECT %d,%d -> %d,%d", window->m_params.m_rect.m_left, window->m_params.m_rect.m_top, window->m_params.m_rect.m_width, window->m_params.m_rect.m_height);

		window->m_rect.m_left = info.rcWindow.left;
		window->m_rect.m_width = info.rcWindow.right - info.rcWindow.left;
		window->m_rect.m_top = info.rcWindow.top;
		window->m_rect.m_height = info.rcWindow.bottom - info.rcWindow.top;

//		ueLogD("NEW FULL RECT %d,%d -> %d,%d", window->m_rect.m_left, window->m_rect.m_top, window->m_rect.m_width, window->m_rect.m_height);
	}
}

LRESULT CALLBACK ueWindow_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ueWindow* window = NULL;

	if (msg == WM_CREATE)
	{
		CREATESTRUCT* create = (CREATESTRUCT*) lParam;
		window = (ueWindow*) create->lpCreateParams;
		UE_ASSERT(window);
		window->m_hwnd = hwnd;
	}
	else
	{
		window = s_windows.Front();
		while (window && window->m_hwnd != hwnd)
			window = window->Next();
	}

	if (window)
		switch (msg)
		{
		case WM_ACTIVATE:
			window->m_hasFocus = (LOWORD(wParam) == WA_ACTIVE);
			if (window->m_params.m_hideCursorWhenActive)
			{
				// Note: This is a bit crap... but Quake does the same thing ;-)
				if (window->m_hasFocus)
					while (ShowCursor(FALSE) >= 0) {}
				else
					while (ShowCursor(TRUE) < 0) {}
			}
			break;
		case WM_SIZE:
		{
			ueWindow_UpdateRects(window);
			if (window->m_params.m_resizeCallback)
				window->m_params.m_resizeCallback(window, window->m_params.m_rect);
			if (window->m_extraResizeCallback)
				window->m_extraResizeCallback(window, window->m_params.m_rect);
			return 0;
		}
		case WM_MOVE:
			ueWindow_UpdateRects(window);
			return 0;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO& minMaxInfo = *(MINMAXINFO*) lParam;

			minMaxInfo.ptMinTrackSize.x = window->m_params.m_minWidth;
			minMaxInfo.ptMinTrackSize.y = window->m_params.m_minHeight;
			minMaxInfo.ptMaxTrackSize.x = window->m_params.m_maxWidth;
			minMaxInfo.ptMaxTrackSize.y = window->m_params.m_maxHeight;
			break;
		}
		case WM_CLOSE:
			break;
		case WM_MOUSEHWHEEL:
			s_mouseState.m_wheelDeltaAcc += ((short) HIWORD(wParam)) / -WHEEL_DELTA;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ueWindow_GetDesiredWindowStylesAndRect(ueWindow* window, u32& style, u32& extStyle, ueRectI& rect)
{
	// Get monitor rectangle

	POINT leftTop;
	leftTop.x = window->m_rect.m_left;
	leftTop.y = window->m_rect.m_top;
	HMONITOR monitor =
		window->m_hwnd ?
		MonitorFromWindow(window->m_hwnd, MONITOR_DEFAULTTONEAREST) :
		MonitorFromPoint(leftTop, MONITOR_DEFAULTTONEAREST);

	MONITORINFO monitorInfo;
	ueMemZeroS(monitorInfo);
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);

	// Set up rectangle & styles

	if (window->m_params.m_fullscreen)
	{
		extStyle = WS_EX_TOPMOST;
		style = WS_POPUP | (window->m_params.m_visible ? WS_VISIBLE : 0);

		// Set (left, top) based on monitor's left top

		rect.m_left = monitorInfo.rcMonitor.left;
		rect.m_top = monitorInfo.rcMonitor.top;
		rect.m_width = window->m_params.m_rect.m_width;
		rect.m_height = window->m_params.m_rect.m_height;
	}
	else
	{
		extStyle = 0;
		style = WS_OVERLAPPED | WS_CLIPCHILDREN | WS_BORDER | WS_CAPTION | (window->m_params.m_visible ? WS_VISIBLE : 0);
		if (window->m_params.m_resizeable)
			style |= WS_SIZEBOX;
		if (window->m_params.m_maximizable)
			style |= WS_MAXIMIZEBOX;
		if (window->m_params.m_minimizable)
			style |= WS_MINIMIZEBOX;

		// Determine total required width & height (including menu bars, borders etc.)

		RECT rc;
		SetRect(&rc, 0, 0, window->m_params.m_rect.m_width, window->m_params.m_rect.m_height);
		AdjustWindowRectEx(&rc, style, FALSE, extStyle);

		rect.m_width = rc.right - rc.left;
		rect.m_height = rc.bottom - rc.top;

		// Center the window if smaller than monitor

		const u32 monitorWidth = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
		const u32 monitorHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

		rect.m_left = rect.m_width < (s32) monitorWidth ? (((s32) monitorWidth - rect.m_width) / 2) : 0;
		rect.m_top = rect.m_height < (s32) monitorHeight ? (((s32) monitorHeight - rect.m_height) / 2) : 0;
	}
}

ueWindow* ueWindow_Create(ueWindowParams* params)
{
	ueWindow* window = new(params->m_allocator) ueWindow();
	UE_ASSERT(window);
	s_windows.PushBack(window);

	window->m_params = *params;
	window->m_hasFocus = UE_FALSE;

	// Create window class

	WNDCLASSEX wc;
	ueMemZeroS(wc);
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = ueWindow_WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = window->m_params.m_hInstance;
	wc.hIcon         = LoadIcon(NULL, window->m_params.m_icon);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = 0;//(HBRUSH)(COLOR_MENUTEXT);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = window->m_params.m_className;
	wc.hIconSm       = LoadIcon(NULL, window->m_params.m_smallIcon);

	UE_ASSERT_FUNC(RegisterClassEx(&wc));

#if defined(GL_OPENGL)

	// If fullscreen: create windowed, then change to fullscreen mode

	const ueBool isFullscreen = window->m_params.m_fullscreen;
	window->m_params.m_fullscreen = UE_FALSE;

#endif // defined(GL_OPENGL)

	// Get window style & rect

	u32 style, extStyle;
	ueRectI rect;
	ueWindow_GetDesiredWindowStylesAndRect(window, style, extStyle, rect);

	// Create the window

	window->m_hwnd = NULL; // Handle will be set in window msg handler
	CreateWindowEx(
		extStyle,
		window->m_params.m_className,
		window->m_params.m_name,
		style,
		rect.m_left, rect.m_top, rect.m_width, rect.m_height,
		NULL,
		NULL,
		window->m_params.m_hInstance,
		window);
	UE_ASSERT(window->m_hwnd);

#if defined(GL_OPENGL)

	// Change to fullscreen

	if (isFullscreen) 
		ueWindow_ChangeMode(window, UE_TRUE, 0, 0, 0);

#endif

	// Get work area dimensions

	ueWindow_UpdateRects(window);

	// Show the window

	ShowWindow(window->m_hwnd, window->m_params.m_visible ? SW_NORMAL : SW_HIDE);
	SetForegroundWindow(window->m_hwnd);
	UpdateWindow(window->m_hwnd);

	return window;
}

ueBool ueWindow_ChangeMode(ueWindow* window, ueBool fullscreen, u32 width, u32 height, u32 frequency)
{
	if (!width)
		width = window->m_params.m_rect.m_width;
	if (!height)
		height = window->m_params.m_rect.m_height;
	if (!frequency)
		frequency = window->m_params.m_frequency;

	if (window->m_params.m_fullscreen == fullscreen &&
		window->m_params.m_rect.m_width == width &&
		window->m_params.m_rect.m_height == height &&
		window->m_params.m_frequency == frequency)
		return UE_TRUE;

	const u32 prevWidth = window->m_rect.m_width;
	const u32 prevHeight = window->m_rect.m_height;

	window->m_params.m_fullscreen = fullscreen;
	window->m_params.m_rect.m_width = width;
	window->m_params.m_rect.m_height = height;
	window->m_params.m_frequency = frequency;

	u32 style, extStyle;
	ueRectI rect;
	ueWindow_GetDesiredWindowStylesAndRect(window, style, extStyle, rect);

	if (fullscreen)
	{
		// Set up display mode

		DEVMODE displayDeviceMode;
		ueMemZeroS(displayDeviceMode);

		displayDeviceMode.dmSize = sizeof(DEVMODE);
		displayDeviceMode.dmBitsPerPel = window->m_params.m_bitsPerPixel;
		displayDeviceMode.dmPelsWidth = window->m_params.m_rect.m_width;
		displayDeviceMode.dmPelsHeight = window->m_params.m_rect.m_height;
		displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		displayDeviceMode.dmDisplayFrequency = window->m_params.m_frequency;
		if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN | CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL)
			displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

		// Set style and rectangle
#if defined(WIN_HACK_FIXME)
		window->m_params.m_fullscreen = UE_FALSE;
		u32 windowedStyle, windowedExtStyle;
		ueWindow_GetDesiredWindowStylesAndRect(window, windowedStyle, windowedExtStyle, rect);
		RECT windowedRect;
		SetRect(&windowedRect, 0, 0, width, height);
		AdjustWindowRect(&windowedRect, windowedStyle, FALSE);
		window->m_params.m_fullscreen = UE_TRUE;
#endif

		SetWindowLong(window->m_hwnd, GWL_STYLE, style);
		SetWindowPos(window->m_hwnd, HWND_TOPMOST,
#if defined(WIN_HACK_FIXME)
			windowedRect.left, windowedRect.top, windowedRect.right - windowedRect.left, windowedRect.bottom - windowedRect.top, // WTF !!! FIXME
#else
			0, 0, prevWidth, prevHeight,
			//rect.m_left, rect.m_top, rect.m_width, rect.m_height,
#endif
			SWP_NOACTIVATE);

		// Change to fullscreen

		if (ChangeDisplaySettingsEx(NULL, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
		{
			ueLogW("ChangeDisplaySettings failed when switching to fullscreen (%ux%u)", width, height);
			return UE_FALSE;
		}

		SetWindowPos(window->m_hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);
	}
	else
	{
		// Switch off fullscreen

		if (ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL) != DISP_CHANGE_SUCCESSFUL)
		{
			ueLogW("ChangeDisplaySettings failed when switching to windows mode (%ux%u)", width, height);
			return UE_FALSE;
		}

		// Set rectangle and style

		SetWindowLong(window->m_hwnd, GWL_STYLE, style);
		SetWindowPos(window->m_hwnd, HWND_NOTOPMOST, rect.m_left, rect.m_top, rect.m_width, rect.m_height, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);
	}

//	ueLogD("CHANGE TO %s %d,%d -> %d,%d", fullscreen ? "FULLSCREEN" : "WINDOWED", rect.m_left, rect.m_top, rect.m_width, rect.m_height);
	return UE_TRUE;
}

void ueWindow_Minimize(ueWindow* window)
{
	ShowWindow(window->m_hwnd, SW_SHOWMINIMIZED);
}

void ueWindow_AddResizeCallback(ueWindow* window, ueWindowResizeCallback callback)
{
	UE_ASSERT(!window->m_extraResizeCallback);
	window->m_extraResizeCallback = callback;
}

void ueWindow_RemoveResizeCallback(ueWindow* window, ueWindowResizeCallback callback)
{
	UE_ASSERT(callback && window->m_extraResizeCallback == callback);
	window->m_extraResizeCallback = NULL;
}

void ueWindow_SetName(ueWindow* window, const char* name)
{
	SetWindowText(window->m_hwnd, name);
}

ueBool ueWindow_HasFocus(ueWindow* window)
{
	return window->m_hasFocus;
}

const ueRectI& ueWindow_GetWorkArea(ueWindow* window)
{
	return window->m_params.m_rect;
}

const ueRectI& ueWindow_GetTotalArea(ueWindow* window)
{
	return window->m_rect;
}

const ueWindowParams* ueWindow_GetParams(ueWindow* window)
{
	return &window->m_params;
}

void ueWindow_SetMain(ueWindow* window)
{
	s_mainWindow = window;
}

ueWindow* ueWindow_GetMain()
{
	return s_mainWindow;
}

HWND ueWindow_GetHWND(ueWindow* window)
{
	return window->m_hwnd;
}

void ueWindow_Destroy(ueWindow* window)
{
	if (window == s_mainWindow)
		s_mainWindow = NULL;
	if (window->m_hwnd)
	{
		DestroyWindow(window->m_hwnd);
		window->m_hwnd = 0;
		UnregisterClass(window->m_params.m_className, window->m_params.m_hInstance);
	}
	s_windows.Remove(window);
	window->m_params.m_allocator->Free(window);
}

void ueWindowUtils_HandleOSMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		ueWindow_WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}
}

ueWindowMsgBoxResult ueWindowUtils_ShowMessageBox(ueWindow* window, const char* message, const char* title, ueWindowMsgBoxType type)
{
	INT mbType;
	switch (type)
	{
		case ueWindowMsgBoxType_Info: mbType = MB_ICONINFORMATION; break;
		case ueWindowMsgBoxType_QuestionOkCancel: mbType = MB_ICONQUESTION | MB_OKCANCEL; break;
		case ueWindowMsgBoxType_QuestionYesNo: mbType = MB_ICONQUESTION | MB_YESNO; break;
		case ueWindowMsgBoxType_QuestionYesNoCancel: mbType = MB_ICONQUESTION | MB_YESNOCANCEL; break;
		case ueWindowMsgBoxType_Warning: mbType = MB_ICONWARNING; break;
		case ueWindowMsgBoxType_Error: mbType = MB_ICONERROR; break;
		UE_INVALID_CASE(type);
	}

	INT result = MessageBox(window ? window->m_hwnd : 0, message, title, mbType);
	switch (result)
	{
		case IDOK: return ueWindowMsgBoxResult_Ok;
		case IDYES: return ueWindowMsgBoxResult_Yes;
		case IDNO: return ueWindowMsgBoxResult_No;
		case IDCANCEL: return ueWindowMsgBoxResult_Cancel;
		default: return ueWindowMsgBoxResult_Ok;
	}
}
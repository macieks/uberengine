#include "GraphicsExt/gxConsole.h"
#include "GraphicsExt/gxDebugWindow.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxFont.h"
#include "Input/inSys.h"
#include "Base/ueEnv.h"
#include "Base/Containers/ueRingBuffer.h"

struct gxConsoleData
{
	ueAllocator* m_allocator;

	ueVar* m_show;

	gxDebugWindow* m_window;

	ueRingBuffer m_linesBuffer;
	char m_lineStart;
	char m_line[256];
	u32 m_cursorPos;
	f32 m_cursorTimer;
	u32 m_lineIndex;

	ueRingBuffer m_historyBuffer;
	u32 m_historyIndex;

	gxConsoleData() : m_allocator(NULL) {}
};

static gxConsoleData s_data;

void gxConsole_DrawFunc(glCtx* ctx, gxDebugWindow* window, void* userData);
void gxConsole_LoggerCallback(const ueLogContext& context, const char* buffer, void* userData);

void gxConsole_Startup(gxConsoleStartupParams* params)
{
	UE_ASSERT(!s_data.m_allocator);

	s_data.m_allocator = params->m_stackAllocator;

	s_data.m_show = ueVar_CreateBool("debug.show_console", UE_FALSE);
	gxDebugMenuItem* debugMenu = gxDebugMenu_GetItem("Debug");
	gxDebugMenu_CreateVar(debugMenu, s_data.m_show);

	gxDebugWindowDesc windowDesc;
	windowDesc.m_name = "Console";
	windowDesc.m_autoSize = UE_FALSE;
	windowDesc.m_drawFunc = gxConsole_DrawFunc;
	windowDesc.m_rect.SetLeftTopAndSize(5, 5, 790, 350);
	s_data.m_window = gxDebugWindow_Create(&windowDesc);
	UE_ASSERT(s_data.m_window);

	s_data.m_linesBuffer.Init(s_data.m_allocator, params->m_maxTextLines, (ueSize) params->m_maxTextSize);
	s_data.m_linesBuffer.EnableWrapping(UE_FALSE);
	s_data.m_lineIndex = 0;

	s_data.m_historyBuffer.Init(s_data.m_allocator, params->m_maxHistoryTextLines, (ueSize) params->m_maxHistoryTextSize);
	s_data.m_historyBuffer.EnableWrapping(UE_FALSE);
	s_data.m_historyIndex = U32_MAX;

	s_data.m_lineStart = '>';
	s_data.m_line[0] = 0;
	s_data.m_cursorPos = 0;
	s_data.m_cursorTimer = 0.0f;

	ueLogger_RegisterOutputListener(gxConsole_LoggerCallback, NULL);

	ueLogI("Console started. Type in 'help' for help.");
}

void gxConsole_Shutdown()
{
	UE_ASSERT(s_data.m_allocator);
	ueVar_Destroy(s_data.m_show);
	ueLogger_UnregisterOutputListener(gxConsole_LoggerCallback, NULL);
	s_data.m_historyBuffer.Deinit();
	s_data.m_linesBuffer.Deinit();
	gxDebugWindow_Destroy(s_data.m_window);
	s_data.m_allocator = NULL;
}

void gxConsole_Update(f32 dt)
{
	UE_ASSERT(s_data.m_allocator);

	gxDebugWindow_SetVisible(s_data.m_window, ueVar_GetBool(s_data.m_show));

	s_data.m_cursorTimer = ueMod(s_data.m_cursorTimer + dt, 1.0f);

	const u32 lineLength = ueStrLen(s_data.m_line);

	// Handle up/down scrolling

	if (inKeyboard_WasPressed(inKey_PageUp))
	{
		const u32 numRows = inKeyboard_IsDown(inKey_Shift) ? 10 : 1;
		if (s_data.m_lineIndex + 1 < s_data.m_linesBuffer.Count())
			s_data.m_lineIndex = ueMin(s_data.m_lineIndex + numRows, s_data.m_linesBuffer.Count() - 1);
	}

	else if (inKeyboard_WasPressed(inKey_PageDown))
	{
		const u32 numRows = inKeyboard_IsDown(inKey_Shift) ? 10 : 1;
		if (s_data.m_lineIndex > 0)
			s_data.m_lineIndex = (u32) ueMax((s32) s_data.m_lineIndex - (s32) numRows, 0);
	}

	else if (inKeyboard_IsDown(inKey_Shift) && inKeyboard_WasPressed(inKey_Home))
	{
		s_data.m_lineIndex = s_data.m_linesBuffer.Count() - 1;
	}

	else if (inKeyboard_IsDown(inKey_Shift) && inKeyboard_WasPressed(inKey_End))
	{
		s_data.m_lineIndex = 0;
	}

	// Handle history up/down

	else if (inKeyboard_WasPressed(inKey_Up))
	{
		if (s_data.m_historyIndex + 1 < s_data.m_historyBuffer.Count())
			s_data.m_historyIndex++;

		if (s_data.m_historyIndex == U32_MAX)
			s_data.m_line[0] = 0;
		else
		{
			void* historyData = NULL;
			s_data.m_historyBuffer.GetBackData(s_data.m_historyIndex, &historyData);
			ueStrCpyS(s_data.m_line, (const char*) historyData);
		}

		s_data.m_cursorPos = ueStrLen(s_data.m_line);
	}

	else if (inKeyboard_WasPressed(inKey_Down))
	{
		if (s_data.m_historyIndex != U32_MAX)
			s_data.m_historyIndex--;

		if (s_data.m_historyIndex == U32_MAX)
			s_data.m_line[0] = 0;
		else
		{
			void* historyData = NULL;
			s_data.m_historyBuffer.GetBackData(s_data.m_historyIndex, &historyData);
			ueStrCpyS(s_data.m_line, (const char*) historyData);
		}
		s_data.m_cursorPos = ueStrLen(s_data.m_line);
	}

	// Handle home/end

	else if (inKeyboard_WasPressed(inKey_Home))
		s_data.m_cursorPos = 0;
	else if (inKeyboard_WasPressed(inKey_End))
		s_data.m_cursorPos = lineLength;

	// Handle left/right

	else if (inKeyboard_WasPressed(inKey_Left))
	{
		if (s_data.m_cursorPos > 0)
		{
			if (inKeyboard_IsDown(inKey_Control))
			{
				while (s_data.m_cursorPos > 0 && s_data.m_line[s_data.m_cursorPos - 1] == ' ')
					s_data.m_cursorPos--;
				while (s_data.m_cursorPos > 0 && s_data.m_line[s_data.m_cursorPos - 1] != ' ')
					s_data.m_cursorPos--;
			}
			else
				s_data.m_cursorPos--;
		}
	}
	else if (inKeyboard_WasPressed(inKey_Right))
	{
		if (s_data.m_cursorPos < lineLength)
		{
			if (inKeyboard_IsDown(inKey_Control))
			{
				while (s_data.m_cursorPos < lineLength && s_data.m_line[s_data.m_cursorPos] != ' ')
					s_data.m_cursorPos++;
				while (s_data.m_cursorPos < lineLength && s_data.m_line[s_data.m_cursorPos] == ' ')
					s_data.m_cursorPos++;
			}
			else
				s_data.m_cursorPos++;
		}
	}

	// Handle delete/backspace

	else if (inKeyboard_WasPressed(inKey_Delete))
	{
		if (s_data.m_cursorPos < lineLength)
			ueMemMove(&s_data.m_line[s_data.m_cursorPos], &s_data.m_line[s_data.m_cursorPos + 1], lineLength - s_data.m_cursorPos + 1);
	}
	else if (inKeyboard_WasPressed(inKey_Back))
	{
		if (s_data.m_cursorPos > 0)
		{
			s_data.m_cursorPos--;
			ueMemMove(&s_data.m_line[s_data.m_cursorPos], &s_data.m_line[s_data.m_cursorPos + 1], lineLength - s_data.m_cursorPos + 1);
		}
	}

	// Handle copy from clipboard via CTRL+V

	else if (inKeyboard_IsDown(inKey_Control) && inKeyboard_WasPressed(inKey_V))
	{
#if defined(UE_WIN32)
		if (OpenClipboard(NULL))
		{
			const char* clip = (const char*) GetClipboardData(CF_TEXT);
			if (clip)
			{
				const u32 clipLength = ueStrLen(clip);
				if (lineLength + clipLength + 1 < UE_ARRAY_SIZE(s_data.m_line))
				{
					ueMemMove(&s_data.m_line[s_data.m_cursorPos + clipLength], &s_data.m_line[s_data.m_cursorPos], lineLength - s_data.m_cursorPos + 1);
					ueMemCpy(&s_data.m_line[s_data.m_cursorPos], clip, clipLength);
				}
			}
			CloseClipboard();
		}
#endif
	}

	// Handle tab auto-completion

	else if (inKeyboard_WasPressed(inKey_Tab) &&
		!inKeyboard_IsDown(inKey_Control) && !inKeyboard_IsDown(inKey_Alt)) // Exclude alt-tabbing
	{
		ueEnv_AutoComplete(s_data.m_line, UE_ARRAY_SIZE(s_data.m_line), !inKeyboard_IsDown(inKey_Shift));
		s_data.m_cursorPos = ueStrLen(s_data.m_line);
	}

	// Handle enter

	else if (inKeyboard_WasPressed(inKey_Return))
	{
		// Do not execute empty commands

		ueBool isAnyChar = UE_FALSE;
		for (u32 i = 0; i < lineLength; i++)
			if (s_data.m_line[i] != ' ')
			{
				isAnyChar = UE_TRUE;
				break;
			}

		if (isAnyChar)
		{
			// Add entry to history ring buffer

			s_data.m_historyBuffer.PushBack(s_data.m_line, lineLength + 1, UE_TRUE);
			if (s_data.m_historyIndex != U32_MAX)
				s_data.m_historyIndex = ueMin(s_data.m_historyIndex, s_data.m_historyBuffer.Count() - 1);

			// Output command line

			ueLogI("> %s", s_data.m_line);

			// Execute command

			ueEnv_DoString(s_data.m_line);

			// Reset console command line

			s_data.m_line[0] = 0;
			s_data.m_cursorPos = 0;
			s_data.m_lineIndex = 0;
			s_data.m_historyIndex = U32_MAX;
		}
	}

	// Handle regular scan codes

	else
	{
		const u32 numScanCodes = inKeyboard_GetNumScanCodes();
		for (u32 i = 0; i < numScanCodes; i++)
			if (inKeyboard_WasPressedScanCode(i))
			{
				if (lineLength + 1 == UE_ARRAY_SIZE(s_data.m_line))
					return;

				const inKeyboardScanCode& scanCode = inKeyboard_GetScanCode(i);
				ueMemMove(&s_data.m_line[s_data.m_cursorPos + 1], &s_data.m_line[s_data.m_cursorPos], lineLength - s_data.m_cursorPos + 1);
				s_data.m_line[s_data.m_cursorPos] = scanCode.m_scanCode;

				s_data.m_cursorPos++;

				s_data.m_lineIndex = 0;
				s_data.m_historyIndex = U32_MAX;
				return;
			}
	}
}

void gxConsole_DrawFunc(glCtx* ctx, gxDebugWindow* window, void* userData)
{
	// Get draw params

	gxFont* font = gxTextRenderer_GetDefaultFont();
	const f32 fontWidth = (f32) gxFont_GetWidthInPixels(font);
	const f32 fontHeight = (f32) gxFont_GetSizeInPixels(font);

	ueRect rect;
	gxDebugWindow_GetDrawRect(rect, window);

	// Draw background

	gxDebugWindow_DrawBackground(ctx, window);

	// Draw edited line

	const u32 lineLength = ueStrLen(s_data.m_line);

	gxText text;
	text.m_x = rect.m_left;
	text.m_y = rect.m_bottom - fontHeight;
	text.m_utf8Buffer = &s_data.m_lineStart;
	text.m_color = ueColor32::White;

	gxTextRenderer_Draw(ctx, &text);

	// Draw cursor

	if (s_data.m_cursorTimer > 0.2f)
	{
		text.m_x = rect.m_left + (s_data.m_cursorPos + 0.6f) * fontWidth;
		text.m_utf8Buffer = "|";

		gxTextRenderer_Draw(ctx, &text);
	}

	text.m_y -= fontHeight;
	text.m_x = rect.m_left;

	// Draw visible lines

	u32 indexFromTop = s_data.m_lineIndex;
	while (rect.m_top <= text.m_y && indexFromTop < s_data.m_linesBuffer.Count())
	{
		void* lineData = NULL;
		s_data.m_linesBuffer.GetBackData(indexFromTop, &lineData);
		text.m_utf8Buffer = (const char*) lineData;
		gxTextRenderer_Draw(ctx, &text);

		text.m_y -= fontHeight;
		indexFromTop++;
	}
}

void gxConsole_LoggerCallback(const ueLogContext& context, const char* buffer, void* userData)
{
	s_data.m_linesBuffer.PushBack(buffer, ueStrLen(buffer) + 1, UE_TRUE);
	s_data.m_lineIndex = ueMin(s_data.m_lineIndex, s_data.m_linesBuffer.Count() - 1);
}
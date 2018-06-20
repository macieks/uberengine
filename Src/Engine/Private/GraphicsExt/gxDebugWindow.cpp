#include "Base/Containers/ueList.h"
#include "Input/inSys.h"
#include "IO/ioXml.h"
#include "GraphicsExt/gxDebugWindow.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTexture.h"

#define GX_DEBUG_WINDOW_SPACING 3.0f
#define GX_DEBUG_WINDOW_BUTTON_DRAWABLE_SCALE 0.8f
#define GX_DEBUG_WINDOW_MIN_WIDTH 80.0f
#define GX_DEBUG_WINDOW_MIN_HEIGHT 30.0f

struct gxDebugWindow : ueList<gxDebugWindow>::Node, gxDebugWindowDesc
{
	u32 m_contentLength;
	u32 m_contentBufferSize;
};

struct gxDebugWindowSysData
{
	ueAllocator* m_allocator;

	ueBool m_active;
	ueBool m_drawCursor;

	ueList<gxDebugWindow> m_windows;

	ueVec2 m_cursorPos;
	gxDebugWindow* m_draggedWindow;
	ueVec2 m_draggingOffset;

	gxDebugWindow* m_resizedWindow;

	gxDebugWindow* m_switchWindow;

	gxDebugWindow* m_hoverWindow;

	ueResourceHandle<gxTexture> m_cursorTexture;
};

static gxDebugWindowSysData* s_data = NULL;

void gxDebugWindowSys_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxDebugWindowSysData;
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_active = UE_FALSE;
	s_data->m_drawCursor = UE_TRUE;

	s_data->m_cursorPos.Set(100, 100);
	s_data->m_draggedWindow = NULL;

	s_data->m_resizedWindow = NULL;

	s_data->m_switchWindow = NULL;
}

void gxDebugWindowSys_StartupResources()
{
	UE_ASSERT(s_data);
	s_data->m_cursorTexture.SetByName("common/cursor_texture");
}

void gxDebugWindowSys_ShutdownResources()
{
	UE_ASSERT(s_data);
	s_data->m_cursorTexture = NULL;
}

void gxDebugWindowSys_Shutdown()
{
	UE_ASSERT(s_data);
	UE_ASSERT(s_data->m_windows.IsEmpty());
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool gxDebugWindowSys_IsInitialized()
{
	return s_data != NULL;
}

gxDebugWindow* gxDebugWindow_FindWindow(const char* name)
{
	for (gxDebugWindow* window = s_data->m_windows.Front(); window; window = window->Next())
		if (!ueStrCmp(name, window->m_name))
			return window;
	return NULL;
}

ueBool gxDebugWindowSys_LoadLayout(const char* layoutFileName)
{
	uePath layoutFilePath;
	ueStrFormatS(layoutFilePath, "%s.layout.xml", layoutFileName);
	ioXmlDoc* doc = ioXmlDoc_Load(s_data->m_allocator, layoutFilePath);
	if (!doc)
		return UE_FALSE;
	ioXmlDocScopedDestructor docDestructor(doc);

	if (ioXmlNode* layoutNode = ioXmlDoc_GetFirstNode(doc, "layout"))
		for (ioXmlNode* windowNode = ioXmlNode_GetFirstNode(layoutNode, "window"); windowNode; windowNode = ioXmlNode_GetNext(windowNode, "window"))
		{
			const char* name = ioXmlNode_GetAttrValue(windowNode, "name");
			gxDebugWindow* window = gxDebugWindow_FindWindow(name);
			if (!window)
				continue;

			if (!ioXmlNode_GetAttrValueBool(windowNode, "show", window->m_isVisible) ||
				!ioXmlNode_GetAttrValueF32(windowNode, "left", window->m_rect.m_left) ||
				!ioXmlNode_GetAttrValueF32(windowNode, "top", window->m_rect.m_top) ||
				!ioXmlNode_GetAttrValueF32(windowNode, "right", window->m_rect.m_right) ||
				!ioXmlNode_GetAttrValueF32(windowNode, "bottom", window->m_rect.m_bottom))
				continue;

			if (window->m_rect.m_left >= window->m_rect.m_right)
				window->m_rect.m_right = window->m_rect.m_left + 100;
			if (window->m_rect.m_top >= window->m_rect.m_bottom)
				window->m_rect.m_bottom = window->m_rect.m_top + 50;
		}
	else
		return UE_FALSE;

	return UE_TRUE;
}

ueBool gxDebugWindowSys_SaveLayout(const char* layoutFileName)
{
	ioXmlDoc* doc = ioXmlDoc_Create(s_data->m_allocator);
	if (!doc)
		return UE_FALSE;
	ioXmlDocScopedDestructor docDestructor(doc);

	ioXmlNode* layoutNode = ioXmlDoc_AddNode(doc, "layout");
	for (gxDebugWindow* window = s_data->m_windows.Front(); window; window = window->Next())
	{
		ioXmlNode* windowNode = ioXmlNode_AddNode(layoutNode, "window");
		ioXmlNode_AddAttr(windowNode, "name", window->m_name);
		ioXmlNode_AddAttrBool(windowNode, "show", window->m_isVisible);
		ioXmlNode_AddAttrF32(windowNode, "left", window->m_rect.m_left);
		ioXmlNode_AddAttrF32(windowNode, "top", window->m_rect.m_top);
		ioXmlNode_AddAttrF32(windowNode, "right", window->m_rect.m_right);
		ioXmlNode_AddAttrF32(windowNode, "bottom", window->m_rect.m_bottom);
	}

	uePath layoutFilePath;
	ueStrFormatS(layoutFilePath, "%s.layout.xml", layoutFileName);
	return ioXmlDoc_Save(doc, layoutFilePath);
}

gxDebugWindow* gxDebugWindow_Create(gxDebugWindowDesc* desc)
{
	// Verify unique window name

	for (gxDebugWindow* window = s_data->m_windows.Front(); window; window = window->Next())
		if (!ueStrCmp(window->m_name, desc->m_name))
		{
			ueLogE("Window with name = '%s' already exists", desc->m_name);
			return NULL;
		}

	// Allocate memory for window and its name

	const u32 windowNameLength = ueStrLen(desc->m_name) + 1;

	const ueSize memorySize = sizeof(gxDebugWindow) + (ueSize) windowNameLength;
	void* memory = s_data->m_allocator->Alloc(memorySize);
	if (!memory)
		return NULL;

	// Set up the window

	gxDebugWindow* window = new(memory) gxDebugWindow();
	*(gxDebugWindowDesc*) window = *desc;

	window->m_name = (char*) (window + 1);
	ueMemCpy(const_cast<char*>(window->m_name), desc->m_name, windowNameLength);

	window->m_content = NULL;
	window->m_contentLength = 0;
	window->m_contentBufferSize = 0;
	gxDebugWindow_SetContent(window, desc->m_content);

	// Add window to front

	s_data->m_windows.PushFront(window);

	return window;
}

void gxDebugWindow_Destroy(gxDebugWindow* window)
{
	s_data->m_windows.Remove(window);
	if (window->m_content)
		s_data->m_allocator->Free(const_cast<char*>(window->m_content));
	s_data->m_allocator->Free(window);

	if (s_data->m_draggedWindow == window)
		s_data->m_draggedWindow = NULL;
}

void gxDebugWindow_SetContent(gxDebugWindow* window, const char* content)
{
	gxDebugWindow_ClearContent(window);
	gxDebugWindow_AppendContent(window, content);
}

void gxDebugWindow_ClearContent(gxDebugWindow* window)
{
	if (!window->m_content)
		return;

	const_cast<char*>(window->m_content)[0] = 0;
	window->m_contentLength = 0;
}

void gxDebugWindow_AppendContent(gxDebugWindow* window, const char* content)
{
	if (!content || !content[0])
		return;

	const u32 appendLength = ueStrLen(content);
	u32 minNewContentBufferSize = window->m_contentLength + appendLength + 1;

	// Resize if needed

	if (minNewContentBufferSize >= window->m_contentBufferSize)
	{
		minNewContentBufferSize = ueAlignPow2<u32>(minNewContentBufferSize, 32);

		const char* newContentBuffer = (char*) s_data->m_allocator->Realloc(const_cast<char*>(window->m_content), minNewContentBufferSize);
		if (!newContentBuffer)
			return; // Failure

		window->m_content = newContentBuffer;
		window->m_contentBufferSize = minNewContentBufferSize;
	}

	// Copy content

	ueMemCpy(const_cast<char*>(window->m_content) + window->m_contentLength, content, appendLength + 1);
	window->m_contentLength += appendLength;
}

void gxDebugWindow_SetRect(gxDebugWindow* window, const ueRect& rect)
{
	window->m_rect = rect;
}

void gxDebugWindow_SetSize(gxDebugWindow* window, f32 width, f32 height)
{
	window->m_rect.SetDims(width, height);
}

const ueVec2& gxDebugWindow_GetPos(gxDebugWindow* window)
{
	return window->m_rect.GetLeftTop();
}

f32 gxDebugWindow_GetTitleBarHeight()
{
	return (f32) gxFont_GetSizeInPixels(gxTextRenderer_GetDefaultFont()) + GX_DEBUG_WINDOW_SPACING;
}

f32 gxDebugWindow_GetTitleBarHeight(gxDebugWindow* window)
{
	return window->m_showName ? gxDebugWindow_GetTitleBarHeight() : 0;
}

void gxDebugWindow_SetDrawSize(gxDebugWindow* window, f32 width, f32 height)
{
	window->m_rect.m_right = window->m_rect.m_left + width + 2 * GX_DEBUG_WINDOW_SPACING;
	window->m_rect.m_bottom = window->m_rect.m_top + gxDebugWindow_GetTitleBarHeight(window) + height + 2 * GX_DEBUG_WINDOW_SPACING;
}

void gxDebugWindow_GetDrawPos(ueVec2& out, gxDebugWindow* window)
{
	out.Set(window->m_rect.m_left + GX_DEBUG_WINDOW_SPACING, window->m_rect.m_top + gxDebugWindow_GetTitleBarHeight(window) + GX_DEBUG_WINDOW_SPACING);
}

void gxDebugWindow_GetDrawRect(ueRect& out, gxDebugWindow* window)
{
	out.m_left = window->m_rect.m_left + GX_DEBUG_WINDOW_SPACING;
	out.m_top = window->m_rect.m_top + gxDebugWindow_GetTitleBarHeight(window) + GX_DEBUG_WINDOW_SPACING;
	out.m_right = window->m_rect.m_right - GX_DEBUG_WINDOW_SPACING * 2;
	out.m_bottom = window->m_rect.m_bottom - GX_DEBUG_WINDOW_SPACING;
}

f32 gxDebugWindow_GetTextTransparency(gxDebugWindow* window)
{
	return window->m_textTransparency;
}

void gxDebugWindow_BringToFront(gxDebugWindow* window)
{
	s_data->m_windows.Remove(window);
	s_data->m_windows.PushFront(window);
}

void gxDebugWindow_SetTransparency(gxDebugWindow* window, f32 transparency)
{
	window->m_transparency = transparency;
}

void gxDebugWindow_SetTextTransparency(gxDebugWindow* window, f32 textTransparency)
{
	window->m_textTransparency = textTransparency;
}

void gxDebugWindow_SetVisible(gxDebugWindow* window, ueBool isVisible)
{
	window->m_isVisible = isVisible;
	gxDebugWindow_BringToFront(window);
}

ueBool gxDebugWindow_IsVisible(gxDebugWindow* window)
{
	return window->m_isVisible;
}

void gxDebugWindow_GetCursorPos(ueVec2& pos)
{
	pos = s_data->m_cursorPos;
}

void gxDebugWindow_SetUserData(gxDebugWindow* window, void* userData)
{
	window->m_userData = userData;
}

gxDebugWindow* gxDebugWindow_GetPreviousVisible(gxDebugWindow* window)
{
	if (window)
	{
		window = window->Previous();
		while (window && !window->m_isVisible)
			window = window->Previous();
	}

	if (!window)
	{
		window = s_data->m_windows.Back();
		while (window && !window->m_isVisible)
			window = window->Previous();
	}

	return window;
}

gxDebugWindow* gxDebugWindow_GetNextVisible(gxDebugWindow* window)
{
	if (window)
	{
		window = window->Next();
		while (window && !window->m_isVisible)
			window = window->Next();
	}

	if (!window)
	{
		window = s_data->m_windows.Front();
		while (window && !window->m_isVisible)
			window = window->Next();
	}

	return window;
}

ueRect gxDebugWindow_GetCloseButtonRect(gxDebugWindow* window, ueBool drawable)
{
	const f32 buttonAreaSize = gxDebugWindow_GetTitleBarHeight();
	const f32 buttonSize = buttonAreaSize * GX_DEBUG_WINDOW_BUTTON_DRAWABLE_SCALE;
	const f32 buttonSpacing = (buttonAreaSize - buttonSize) * 0.5f;

	ueRect rect;
	rect.SetLeftTopAndSize(
		window->m_rect.m_right - buttonAreaSize + buttonSpacing,
		window->m_rect.m_top + buttonSpacing,
		buttonSize,
		buttonSize);
	if (drawable)
		rect.ScaleCentered(GX_DEBUG_WINDOW_BUTTON_DRAWABLE_SCALE);
	return rect;
}

ueRect gxDebugWindow_GetResizeButtonRect(gxDebugWindow* window, ueBool drawable)
{
	const f32 buttonAreaSize = gxDebugWindow_GetTitleBarHeight();
	const f32 buttonSize = buttonAreaSize * GX_DEBUG_WINDOW_BUTTON_DRAWABLE_SCALE;
	const f32 buttonSpacing = (buttonAreaSize - buttonSize) * 0.5f;

	ueRect rect;
	rect.SetLeftTopAndSize(
		window->m_rect.m_right - buttonAreaSize + buttonSpacing,
		window->m_rect.m_bottom - buttonAreaSize + buttonSpacing,
		buttonSize,
		buttonSize);
	if (drawable)
		rect.ScaleCentered(GX_DEBUG_WINDOW_BUTTON_DRAWABLE_SCALE);
	return rect;
}

void gxDebugWindowSys_SetActive(ueBool active)
{
	s_data->m_active = active;
}

void gxDebugWindowSys_Update(f32 dt)
{
	if (!s_data->m_active)
		return;

	// Update cursor

	const ueVec2 oldCursorPos = s_data->m_cursorPos;

	s32 xOffset, yOffset;
	inMouse_GetVector(xOffset, yOffset);
	ueVec2 offset((f32) xOffset, (f32) yOffset);
	s_data->m_cursorPos += offset;

	const glDeviceStartupParams* devParams = glDevice_GetStartupParams();
	s_data->m_cursorPos.Set(
		ueClamp(s_data->m_cursorPos[0], 0.0f, (f32) devParams->m_width - 1.0f),
		ueClamp(s_data->m_cursorPos[1], 0.0f, (f32) devParams->m_height - 1.0f));

	const ueVec2 actualOffset = s_data->m_cursorPos - oldCursorPos;

	// Determine "hover" window

	s_data->m_hoverWindow = NULL;
	for (gxDebugWindow* window = s_data->m_windows.Front(); window; window = window->Next())
	{
		if (!window->m_isVisible)
			continue;

		if (!window->m_rect.Intersect(s_data->m_cursorPos))
			continue;

		s_data->m_hoverWindow = window;
		break;
	}

	// Handle bring-to-fronting and dragging / moving / closing

	if (s_data->m_draggedWindow)
	{
		s_data->m_draggedWindow->m_rect.Translate(actualOffset);
		s_data->m_hoverWindow = s_data->m_draggedWindow;

		if (inMouse_WasReleased(inMouseButton_Left))
			s_data->m_draggedWindow = NULL;
	}
	else if (s_data->m_resizedWindow)
	{
		// Determine new right-bottom corner

		const ueVec2 newRightBottom = s_data->m_resizedWindow->m_rect.GetRightBottom() + actualOffset;

		// Resize the window

		s_data->m_resizedWindow->m_rect.SetRightBottom(newRightBottom);
		s_data->m_resizedWindow->m_rect.m_right = ueMax(s_data->m_resizedWindow->m_rect.m_left + GX_DEBUG_WINDOW_MIN_WIDTH, s_data->m_resizedWindow->m_rect.m_right);
		s_data->m_resizedWindow->m_rect.m_bottom = ueMax(s_data->m_resizedWindow->m_rect.m_top + GX_DEBUG_WINDOW_MIN_HEIGHT, s_data->m_resizedWindow->m_rect.m_bottom);

		s_data->m_hoverWindow = s_data->m_resizedWindow;

		// Stop resizing?

		if (inMouse_WasReleased(inMouseButton_Left))
			s_data->m_resizedWindow = NULL;
	}
	else if (inMouse_WasPressed(inMouseButton_Left) && s_data->m_hoverWindow)
	{
		// Close

		if (s_data->m_hoverWindow->m_hasCloseButton &&
			gxDebugWindow_GetCloseButtonRect(s_data->m_hoverWindow, UE_TRUE).Intersect(s_data->m_cursorPos))
		{
			gxDebugWindow_SetVisible(s_data->m_hoverWindow, UE_FALSE);
		}

		// Resize

		else if (s_data->m_hoverWindow->m_hasResizeButton &&
			!s_data->m_hoverWindow->m_autoSize &&
			gxDebugWindow_GetResizeButtonRect(s_data->m_hoverWindow, UE_TRUE).Intersect(s_data->m_cursorPos))
		{
			gxDebugWindow_BringToFront(s_data->m_hoverWindow);
			s_data->m_resizedWindow = s_data->m_hoverWindow;
		}

		// Move

		else
		{
			gxDebugWindow_BringToFront(s_data->m_hoverWindow);
			s_data->m_draggedWindow = s_data->m_hoverWindow;
		}
	}

	// Handle switching between windows CTRL+TAB and SHIFT+CTRL+TAB

	if (inKeyboard_IsDown(inKey_Control))
	{
		if (inKeyboard_WasPressed(inKey_Tab))
		{
			if (inKeyboard_IsDown(inKey_Shift))
				s_data->m_switchWindow = gxDebugWindow_GetPreviousVisible(s_data->m_switchWindow);
			else
				s_data->m_switchWindow = gxDebugWindow_GetNextVisible(s_data->m_switchWindow);
		}
	}
	else if (s_data->m_switchWindow)
	{
		gxDebugWindow_BringToFront(s_data->m_switchWindow);
		s_data->m_switchWindow = NULL;
	}
}

void gxDebugWindow_DrawBackground(glCtx* ctx, gxDebugWindow* window)
{
	gxShape_Rect rect;
	gxShape_Line2D line;

	// Get debug font

	gxText text;
	text.m_font = gxTextRenderer_GetDefaultFont();

	// Get misc. constants

	const f32 buttonAreaSize = gxDebugWindow_GetTitleBarHeight(window);
	const f32 minWindowWidth = 100.0f;
	const f32 minWindowHeight = buttonAreaSize * 2.0f;

	// Begin drawing debug shapes

	const glDeviceStartupParams* devParams = glDevice_GetStartupParams();

	gxShapeDrawParams drawParams;
	drawParams.m_2DCanvas.Set(0, 0, (f32) devParams->m_width, (f32) devParams->m_height);
	gxShapeDraw_SetDrawParams(&drawParams);
	gxShapeDraw_Begin(ctx);

	// Update window rectangle

	rect.m_rect = window->m_rect;
	if (window->m_autoSize)
	{
		f32 titleWidth, titleHeight;
		if (window->m_showName)
		{
			text.m_utf8Buffer = window->m_name;
			gxTextRenderer_CalcDimensions(&text, titleWidth, titleHeight);
		}
		else
			titleWidth = titleHeight = 0.0f;

		f32 contentWidth = 0, contentHeight = 0;
		if (window->m_content)
		{
			text.m_utf8Buffer = window->m_content;
			gxTextRenderer_CalcDimensions(&text, contentWidth, contentHeight);
		}

		rect.m_rect.m_right = rect.m_rect.m_left + ueMax(minWindowWidth, ueMax(titleWidth + buttonAreaSize, contentWidth)) + 6;
		rect.m_rect.m_bottom = rect.m_rect.m_top + ueMax(minWindowHeight, titleHeight + contentHeight) + 6;
		window->m_rect = rect.m_rect;
	}

	// Draw background

	rect.m_wireFrame = UE_FALSE;
	rect.m_color.SetRGB(20, 20, 20);
	rect.m_color.SetAlphaF(window->m_transparency);
	gxShapeDraw_DrawRect(rect);
	gxShapeDraw_Flush();

	// Draw title separation line

	const ueColor32 borderColor = window == s_data->m_switchWindow ? ueColor32::White : ueColor32(128, 128, 128, 255);

	if (window->m_showName)
	{
		const ueVec2& leftTop = rect.m_rect.GetLeftTop();
		const ueVec2 rightTop = rect.m_rect.GetRightTop();
		const ueVec2 buttonSizeHighVec(0, buttonAreaSize);

		line.m_color.SetRGB(128, 128, 128);
		line.m_color.SetAlphaF(window->m_textTransparency);
		line.m_a = leftTop + buttonSizeHighVec;
		line.m_b = rightTop + buttonSizeHighVec;
		gxShapeDraw_DrawLine(line);
	}

	// Draw buttons

	if (window == s_data->m_hoverWindow)
	{
		gxShape_Rect buttonRect;
		buttonRect.m_wireFrame = UE_FALSE;

		if (window->m_hasCloseButton)
		{
			buttonRect.m_color = ueColor32::Red;
			buttonRect.m_rect = gxDebugWindow_GetCloseButtonRect(window, UE_TRUE);
			gxShapeDraw_DrawRect(buttonRect);
		}

		if (window->m_hasResizeButton && !window->m_autoSize)
		{
			buttonRect.m_color = ueColor32::Green;
			buttonRect.m_rect = gxDebugWindow_GetResizeButtonRect(window, UE_TRUE);
			gxShapeDraw_DrawRect(buttonRect);
		}
	}

	// Draw outline

	rect.m_wireFrame = UE_TRUE;
	rect.m_color = borderColor;
	rect.m_color.SetAlphaF(window->m_textTransparency);
	gxShapeDraw_DrawRect(rect);

	gxShapeDraw_End();

	// Draw the title

	if (window->m_showName)
	{
		text.m_color = ueColor32::White;
		text.m_color.SetAlphaF(window->m_textTransparency);
		text.m_x = (f32) window->m_rect.m_left;
		text.m_y = (f32) window->m_rect.m_top;
		text.m_utf8Buffer = window->m_name;
		gxTextRenderer_Draw(ctx, &text);
	}
}

void gxDebugWindow_Draw(glCtx* ctx, gxDebugWindow* window)
{
	if (!window->m_isVisible)
		return;

#if defined(UE_ENABLE_PROFILER)
	static char profiledScopeName[] = "Draw Window: ?????????????????????????????????????";
	ueStrCpy((char*) profiledScopeName + 13, UE_ARRAY_SIZE(profiledScopeName) - 13, window->m_name);
	UE_PROF_SCOPE_DYNAMIC(profiledScopeName);
#endif

	if (window->m_drawFunc)
	{
		window->m_drawFunc(ctx, window, window->m_userData);
		return;
	}

	// Draw background

	gxDebugWindow_DrawBackground(ctx, window);

	// Draw the content

	if (window->m_content)
	{
		gxText text;
		text.m_color.SetAlphaF(window->m_textTransparency);
		text.m_x = window->m_rect.m_left;
		text.m_y = window->m_rect.m_top + (window->m_showName ? gxFont_GetSizeInPixels(gxTextRenderer_GetDefaultFont()) : 0);
		text.m_utf8Buffer = window->m_content;

		gxTextRenderer_Draw(ctx, &text);
	}
}

void gxDebugWindowSys_Draw(glCtx* ctx)
{
	UE_PROF_SCOPE("Draw Debug Windows");

	// Draw all (visible) windows

	for (gxDebugWindow* window = s_data->m_windows.Back(); window; window = window->Previous())
		if (window != s_data->m_switchWindow)
			gxDebugWindow_Draw(ctx, window);
	if (s_data->m_switchWindow)
		gxDebugWindow_Draw(ctx, s_data->m_switchWindow);

	if (s_data->m_active)
	{
		// Draw the cursor

		const glDeviceStartupParams* devParams = glDevice_GetStartupParams();

		if (s_data->m_drawCursor)
		{
			gxShapeDrawParams drawParams;
			drawParams.m_2DCanvas.Set(0, 0, (f32) devParams->m_width, (f32) devParams->m_height);
			gxShapeDraw_SetDrawParams(&drawParams);
			gxShapeDraw_Begin(ctx);

			glTextureBuffer* cursorTexture = gxTexture_GetBuffer(*s_data->m_cursorTexture);
			const glTextureBufferDesc* cursorTextureDesc = glTextureBuffer_GetDesc(cursorTexture);

			gxShape_TexturedRect rect;
			rect.m_colorMap = cursorTexture;
			rect.m_colorMapSampler = &glSamplerParams::DefaultClamp;
			rect.m_pos.SetLeftTopAndSize(s_data->m_cursorPos[0], s_data->m_cursorPos[1], (f32) cursorTextureDesc->m_width, (f32) cursorTextureDesc->m_height);
			rect.m_enableBlending = UE_TRUE;
			gxShapeDraw_DrawTexturedRect(rect);

			gxShapeDraw_End();
		}

		// Draw switched windows info

		if (s_data->m_switchWindow)
		{
			char buffer[1024];
			buffer[0] = 0;
			char* currBufferPtr = buffer;
			for (gxDebugWindow* window = s_data->m_windows.Front(); window; window = window->Next())
			{
				if (!window->m_isVisible)
					continue;

				if (window == s_data->m_switchWindow)
					ueStrCat2(currBufferPtr, "* ");
				ueStrCat2(currBufferPtr, window->m_name);

				if (window->Next())
					ueStrCat2(currBufferPtr, "\n");
			}

			gxDebugWindow tempWindow;
			tempWindow.m_showName = UE_TRUE;
			tempWindow.m_autoSize = UE_TRUE;
			tempWindow.m_isVisible = UE_TRUE;
			tempWindow.m_content = buffer;
			tempWindow.m_name = "Toggle Debug Windows";
			tempWindow.m_rect.m_left = 256;
			tempWindow.m_rect.m_top = 256;

			gxDebugWindow_Draw(ctx, &tempWindow);
		}
	}
}
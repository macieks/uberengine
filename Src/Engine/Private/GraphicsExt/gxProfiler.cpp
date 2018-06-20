#include "GraphicsExt/gxProfiler.h"
#include "Base/ueEnv.h"
#include "Base/ueProfilerCapture.h"
#include "Utils/utWarningSys.h"
#include "Base/Containers/ueVector.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "GraphicsExt/gxDebugWindow.h"
#include "GraphicsExt/gxShapeDraw.h"

enum gxProfilerView
{
	gxProfilerView_Threads = 0,
	gxProfilerView_Tree
};

struct gxProfilerData
{
	ueAllocator* m_allocator;

	ueBool m_isEnabled;
	ueBool m_isInputEnabled;

	inConsumerId m_inputConsumerId;
	inEventId m_inputLeft;
	inEventId m_inputRight;
	inEventId m_inputUp;
	inEventId m_inputDown;

	ueAppTimer m_timer;

	ueVar* m_showFPS;
	gxDebugWindow* m_fpsWindow;

	ueVar* m_showMem;
	gxDebugWindow* m_memWindow;

	ueVar* m_showWarnings;
	gxDebugWindow* m_warningsWindow;

	ueVar* m_showProfiler;
	gxDebugWindow* m_profilerWindow;

	ueVar* m_showProfilerGraph;
	gxDebugWindow* m_profilerGraphWindow;

	ueVar* m_profilerCaptureFreq;
	ueVar* m_profilerCapturePause;

	ueVar* m_showResources;
	gxDebugWindow* m_resourcesWindow;

	// Profiled allocators

	ueVector<ueAllocator*> m_allocators;

	// Profiler

	const ueProfilerCapture* m_profCapture;
	gxProfilerView m_profView;
	u32 m_profCaptureTreeIndex;
	char m_profCapturePath[1024];
	u32 m_numProfCapturePathSegments;

	u32 m_graphStart;
	u32 m_graphLength;
	f32 m_graph[512]; // Stores values in microseconds

	gxProfilerData() :
		m_allocator(NULL),
		m_isEnabled(UE_TRUE),
		m_isInputEnabled(UE_TRUE),
		m_showFPS(NULL),
		m_profCapture(NULL),
		m_profView(gxProfilerView_Tree),
		m_profCaptureTreeIndex(0),
		m_numProfCapturePathSegments(0),
		m_graphLength(0)
	{}
};

void gxProfiler_DrawMemFunc(glCtx* ctx, gxDebugWindow* window, void* userData);
void gxProfiler_DrawProfilerFunc(glCtx* ctx, gxDebugWindow* window, void* userData);
void gxProfiler_DrawProfilerGraphFunc(glCtx* ctx, gxDebugWindow* window, void* userData);

static gxProfilerData* s_data = NULL;

void gxProfiler_StartupEnv(ueAllocator* allocator)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxProfilerData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	UE_ASSERT(!s_data->m_showFPS);
	s_data->m_showFPS = ueVar_CreateBool("stats.show_fps", UE_FALSE);
	s_data->m_showMem = ueVar_CreateBool("stats.show_mem", UE_FALSE);
	s_data->m_showWarnings = ueVar_CreateBool("stats.show_warnings", UE_FALSE);
	s_data->m_showProfiler = ueVar_CreateBool("stats.show_profiler", UE_FALSE);
	s_data->m_showProfilerGraph = ueVar_CreateBool("stats.show_profiler_graph", UE_FALSE);
	s_data->m_profilerCaptureFreq = ueVar_CreateF32("stats.profiler_capture_freq", 1.0f, 0.01f, 10.0f, 0.1f);
	s_data->m_profilerCapturePause = ueVar_CreateBool("stats.profiler_capture_pause", UE_FALSE);
	s_data->m_showResources = ueVar_CreateBool("stats.show_resources", UE_FALSE);
}

void gxProfiler_Startup(gxProfilerStartupParams* params)
{
	UE_ASSERT_MSG(s_data, "Must call gxProfiler_StartupEnv() before.");

	s_data->m_timer.Init();

	UE_ASSERT_FUNC(s_data->m_allocators.Init(s_data->m_allocator, params->m_maxAllocators));

	gxDebugMenuItem* statsMenu = gxDebugMenu_GetItem("Statistics");
	UE_ASSERT(statsMenu);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showFPS);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showMem);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showWarnings);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showProfiler);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showProfilerGraph);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_profilerCaptureFreq);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_profilerCapturePause);
	gxDebugMenu_CreateVar(statsMenu, s_data->m_showResources);

	s_data->m_profCapture = NULL;
	s_data->m_graphLength = 0;

	s_data->m_inputConsumerId = inSys_RegisterConsumer("profiler", params->m_inputPriority);
	inSys_EnableConsumer(s_data->m_inputConsumerId, UE_FALSE);
	s_data->m_inputLeft = inSys_RegisterEvent(s_data->m_inputConsumerId, "left", TempPtr(inBinding(inDev_Keyboard, inKey_Left)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Left)));
	s_data->m_inputRight = inSys_RegisterEvent(s_data->m_inputConsumerId, "right", TempPtr(inBinding(inDev_Keyboard, inKey_Right)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Right)));
	s_data->m_inputUp = inSys_RegisterEvent(s_data->m_inputConsumerId, "up", TempPtr(inBinding(inDev_Keyboard, inKey_Up)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Up)));
	s_data->m_inputDown = inSys_RegisterEvent(s_data->m_inputConsumerId, "down", TempPtr(inBinding(inDev_Keyboard, inKey_Down)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Down)));

	gxDebugWindowDesc windowDesc;
	windowDesc.m_showName = UE_FALSE;
	windowDesc.m_autoSize = UE_FALSE;
	windowDesc.m_name = "FPS";
	windowDesc.m_content = "FPS: 000.00";
	windowDesc.m_rect.SetLeftTopAndSize(650, 50, 70, 20);
	windowDesc.m_hasResizeButton = UE_FALSE;
	s_data->m_fpsWindow = gxDebugWindow_Create(&windowDesc);

	windowDesc.m_showName = UE_TRUE;
	windowDesc.m_autoSize = UE_TRUE;
	windowDesc.m_name = "Runtime Warnings";
	windowDesc.m_rect.SetLeftTopAndSize(100, 10, 600, 100);
	windowDesc.m_hasResizeButton = UE_TRUE;
	s_data->m_warningsWindow = gxDebugWindow_Create(&windowDesc);

	windowDesc.m_autoSize = UE_FALSE;

	windowDesc.m_name = "Profiler";
	windowDesc.m_rect.SetLeftTopAndSize(30, 65, 70, 20);
	windowDesc.m_drawFunc = gxProfiler_DrawProfilerFunc;
	s_data->m_profilerWindow = gxDebugWindow_Create(&windowDesc);

	windowDesc.m_name = "Profiler Event History";
	windowDesc.m_rect.SetLeftTopAndSize(70, 300, 300, 100);
	windowDesc.m_drawFunc = gxProfiler_DrawProfilerGraphFunc;
	s_data->m_profilerGraphWindow = gxDebugWindow_Create(&windowDesc);

	windowDesc.m_name = "Memory Statistics";
	windowDesc.m_rect.SetLeftTopAndSize(50, 450, 700, 100);
	windowDesc.m_drawFunc = gxProfiler_DrawMemFunc;
	windowDesc.m_hasResizeButton = UE_FALSE;
	s_data->m_memWindow = gxDebugWindow_Create(&windowDesc);
}

void gxProfiler_Shutdown()
{
	UE_ASSERT(s_data);

	inSys_UnregisterConsumer(s_data->m_inputConsumerId);

	gxDebugWindow_Destroy(s_data->m_fpsWindow);
	gxDebugWindow_Destroy(s_data->m_profilerWindow);
	gxDebugWindow_Destroy(s_data->m_profilerGraphWindow);
	gxDebugWindow_Destroy(s_data->m_memWindow);
	gxDebugWindow_Destroy(s_data->m_warningsWindow);

	ueVar_Destroy(s_data->m_showFPS);
	ueVar_Destroy(s_data->m_showMem);
	ueVar_Destroy(s_data->m_showWarnings);
	ueVar_Destroy(s_data->m_showProfiler);
	ueVar_Destroy(s_data->m_showProfilerGraph);
	ueVar_Destroy(s_data->m_profilerCaptureFreq);
	ueVar_Destroy(s_data->m_profilerCapturePause);
	ueVar_Destroy(s_data->m_showResources);

	s_data->m_allocators.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxProfiler_Update(f32 dt)
{
	UE_ASSERT(s_data);

	if (!s_data->m_isEnabled)
		return;

	UE_PROF_SCOPE("gxProfiler_Update");

	if (gxDebugWindow_IsVisible(s_data->m_fpsWindow))
	{
		char buffer[64];
		ueStrFormatS(buffer, "FPS %.1f", s_data->m_timer.FPS());
		gxDebugWindow_SetContent(s_data->m_fpsWindow, buffer);
	}

	if (gxDebugWindow_IsVisible(s_data->m_warningsWindow))
	{
		gxDebugWindow_ClearContent(s_data->m_warningsWindow);

#if defined(UE_ENABLE_WARNINGS)

		const u32 numWarnings = ueMin(utWarningSys_GetNumWarnings(), (u32) 6);
		for (u32 i = 0; i < numWarnings; i++)
		{
			if (i > 0)
				gxDebugWindow_AppendContent(s_data->m_warningsWindow, "\n");
			gxDebugWindow_AppendContent(s_data->m_warningsWindow, utWarningSys_GetWarning(i));
		}

#endif // defined(UE_ENABLE_WARNINGS)

	}

	s_data->m_timer.Tick();
}

void gxProfiler_DrawMemFunc(glCtx* ctx, gxDebugWindow* window, void* userData)
{
	const char columnsText[] = "Allocator                 |    Free     |    Used     |    Total    |   Peak Used |   Count  ";

	gxFont* font = gxTextRenderer_GetDefaultFont();

	// Update rectangle

	const f32 fontWidth = (f32) gxFont_GetWidthInPixels(font);
	const f32 fontHeight = (f32) gxFont_GetSizeInPixels(font);

	gxDebugWindow_SetDrawSize(window, (f32) (sizeof(columnsText) - 1) * fontWidth, (s_data->m_allocators.Size() + 1) * fontHeight);

	// Draw background

	gxDebugWindow_DrawBackground(ctx, window);

	// Draw text

	ueVec2 pos;
	gxDebugWindow_GetDrawPos(pos, window);

	char buffer[256];

	gxText text;
	text.m_x = pos[0];
	text.m_utf8Buffer = buffer;

	// Draw column names

	ueStrCpyS(buffer, columnsText);
	const s32 numberStart = ueStrFirstIndexOf(buffer, '|');

	text.m_y = pos[1];
	text.m_color = ueColor32::Red;
	gxTextRenderer_Draw(ctx, &text);

	// Draw stats for each allocator

	text.m_color = ueColor32::Green;

	for (u32 i = 0; i < s_data->m_allocators.Size(); i++)
	{
		ueAllocator* allocator = s_data->m_allocators[i];
		ueAllocatorStats stats;
		allocator->GetStats(&stats);

		ueSize free = stats.m_total - stats.m_used;
		if (!(stats.m_flags & ueAllocatorStatsFlags_HasTotal))
		{
			stats.m_total = 0;
			free = 0;
		}
		if (!(stats.m_flags & ueAllocatorStatsFlags_HasUsed))
			stats.m_used = 0;
		if (!(stats.m_flags & ueAllocatorStatsFlags_HasUsedPeak))
			stats.m_usedPeak = 0;

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
		const u32 allocNameLength = ueStrLen(allocator->GetName());
		ueMemCpy(buffer, allocator->GetName(), allocNameLength);
		ueMemSet(buffer + allocNameLength, ' ', numberStart - allocNameLength);
#else
		const u32 allocNameLength = ueStrLen("<unknown>");
		ueMemCpy(buffer, "<unknown>", allocNameLength);
		ueMemSet(buffer + allocNameLength, ' ', numberStart - allocNameLength);
#endif

		u32 offset = numberStart;

#define FORMAT_BYTES(s, num1024s) \
		buffer[offset++] = '|'; \
		buffer[offset++] = ' '; \
		ueStrFormatBytes(buffer + offset, UE_ARRAY_SIZE(buffer) - offset, (u32) s, num1024s); \
		offset += 3 * 4 - 1; \
		buffer[offset++] = ' ';

		FORMAT_BYTES(free, 3);
		FORMAT_BYTES(stats.m_used, 3);
		FORMAT_BYTES(stats.m_total, 3);
		FORMAT_BYTES(stats.m_usedPeak, 3);
		FORMAT_BYTES(stats.m_numAllocs, 2);
		buffer[offset++] = 0;

		text.m_y += fontHeight;
		gxTextRenderer_Draw(ctx, &text);
	}
}

void gxProfiler_DrawProfiler_Threads(glCtx* ctx, gxDebugWindow* window)
{
	// TODO
}

void gxProfiler_DrawProfilerGraphFunc(glCtx* ctx, gxDebugWindow* window, void* userData)
{
	// Draw background

	gxDebugWindow_DrawBackground(ctx, window);

	// Draw graph

	if (s_data->m_graphLength < 2)
		return;

	const glDeviceStartupParams* devParams = glDevice_GetStartupParams();
	gxShapeDrawParams drawParams;
	drawParams.m_2DCanvas.Set(0, 0, (f32) devParams->m_width, (f32) devParams->m_height);
	gxShapeDraw_SetDrawParams(&drawParams);
	gxShapeDraw_Begin(ctx);

	ueRect rect;
	gxDebugWindow_GetDrawRect(rect, window);

	f32 x = rect.m_left;

	const u32 sizeModulo = UE_ARRAY_SIZE(s_data->m_graph) - 1;

	const u32 startPointIndex = s_data->m_graphLength <= (u32) rect.GetWidth() ? 0 : (s_data->m_graphLength - (u32) rect.GetWidth());

	f32 maxValue = s_data->m_graph[startPointIndex];
	f32 minValue = s_data->m_graph[startPointIndex];
	for (u32 i = startPointIndex; i < s_data->m_graphLength; i++)
	{
		const f32 v = s_data->m_graph[(s_data->m_graphStart + i) & sizeModulo];
		maxValue = ueMax(maxValue, v);
		minValue = ueMin(minValue, v);
	}

	const f32 yScale = rect.GetHeight() / (maxValue - minValue);

	gxShape_Line2D line;

	// Draw top & bottom

	line.m_color = ueColor32(128, 0, 0, 255);

	line.m_a.Set(rect.m_left, rect.m_top);
	line.m_b.Set(rect.m_right, rect.m_top);
	gxShapeDraw_DrawLine(line);

	line.m_a.Set(rect.m_left, rect.m_bottom);
	line.m_b.Set(rect.m_right, rect.m_bottom);
	gxShapeDraw_DrawLine(line);

	// Draw graph

	line.m_color = ueColor32::Yellow;
	line.m_b.Set(x, rect.m_bottom - (s_data->m_graph[(s_data->m_graphStart + startPointIndex) & sizeModulo] - minValue) * yScale);

	for (u32 i = startPointIndex + 1; i < s_data->m_graphLength; i++)
	{
		x += 1.0f;
		line.m_a = line.m_b;
		line.m_b.Set(x, rect.m_bottom - (s_data->m_graph[(s_data->m_graphStart + i) & sizeModulo] - minValue) * yScale);

		gxShapeDraw_DrawLine(line);
	}

	gxShapeDraw_End();

	// Draw min & max legend

	char buffer[64];

	gxText text;
	text.m_color = ueColor32::White;
	text.m_color.SetAlphaF(gxDebugWindow_GetTextTransparency(window));
	text.m_utf8Buffer = buffer;

	ueStrFormatS(buffer, "%u ms", (u32) maxValue);
	text.m_x = rect.m_left;
	text.m_y = rect.m_top;
	gxTextRenderer_Draw(ctx, &text);

	ueStrFormatS(buffer, "%u ms", (u32) minValue);
	text.m_x = rect.m_left;
	text.m_y = rect.m_bottom - gxFont_GetSizeInPixels(gxTextRenderer_GetDefaultFont()) - 3;
	gxTextRenderer_Draw(ctx, &text);
}

void gxProfiler_UpdateAndDrawProfiler_Tree(glCtx* ctx, gxDebugWindow* window)
{
	const char columnsText[] = "Name                                               |   %%   |  F %%  | ms incl | ms excl";

	// Update rectangle

	gxFont* font = gxTextRenderer_GetDefaultFont();
	const f32 fontWidth = (f32) gxFont_GetWidthInPixels(font);
	const f32 fontHeight = (f32) gxFont_GetSizeInPixels(font);

	// Draw background

	gxDebugWindow_DrawBackground(ctx, window);

	// Draw text

	ueVec2 pos;
	gxDebugWindow_GetDrawPos(pos, window);

	// Validate tree index

	if (s_data->m_profCapture->m_numTrees <= s_data->m_profCaptureTreeIndex)
	{
		s_data->m_profCaptureTreeIndex = s_data->m_profCapture->m_numTrees - 1;
		s_data->m_numProfCapturePathSegments = 0;
	}

	// Find current node

	const ueProfilerTree& tree = s_data->m_profCapture->m_trees[s_data->m_profCaptureTreeIndex];

	char* parentNodeName = s_data->m_profCapturePath;
	char* nodeName = s_data->m_profCapturePath;
	ueProfilerNode* parentFirst = NULL;
	ueProfilerNode* parent = NULL;
	ueProfilerNode* nodeFirst = tree.m_root;
	ueProfilerNode* node = tree.m_root;
	ueProfilerNode* prev = NULL;
	for (u32 i = 0; i < s_data->m_numProfCapturePathSegments; i++)
	{
		while (node && ueStrCmp(node->m_name, nodeName))
		{
			prev = node;
			node = node->m_nextSibling;
		}

		// Cut the path?
		if (!node)
		{
			s_data->m_graphLength = 0;
			s_data->m_numProfCapturePathSegments = i;
			break;
		}

		parentNodeName = nodeName;
		nodeName += ueStrLen(nodeName) + 1;

		if (i == s_data->m_numProfCapturePathSegments - 1)
			break;

		prev = NULL;
		parentFirst = nodeFirst;
		parent = node;
		nodeFirst = node = node->m_firstChild;
	}

	// Set up initial selection to root

	if (s_data->m_numProfCapturePathSegments == 0)
	{
		UE_ASSERT(tree.m_root);

		s_data->m_numProfCapturePathSegments = 1;
		ueStrCpyS(s_data->m_profCapturePath, tree.m_root->m_name);

		parentNodeName = s_data->m_profCapturePath;
		nodeName = parentNodeName + ueStrLen(parentNodeName) + 1;
		prev = NULL;
		nodeFirst = node = tree.m_root;
		parentFirst = parent = NULL;
		s_data->m_graphLength = 0;
	}

	// Update
	if (inSys_IsConsumerActive(s_data->m_inputConsumerId))
	{
		if (inSys_WasPressed(s_data->m_inputDown))
		{
			if (node && node->m_nextSibling)
			{
				node = node->m_nextSibling;
				ueMemCpy(parentNodeName, node->m_name, ueStrLen(node->m_name) + 1);
				s_data->m_graphLength = 0;
			}
		}
		else if (inSys_WasPressed(s_data->m_inputUp))
		{
			if (prev)
			{
				node = prev;
				ueMemCpy(parentNodeName, node->m_name, ueStrLen(node->m_name) + 1);
				s_data->m_graphLength = 0;
			}
		}
		else if (inSys_WasPressed(s_data->m_inputRight))
		{
			if (node && node->m_firstChild)
			{
				parentFirst = nodeFirst;
				parent = node;
				nodeFirst = node = node->m_firstChild;

				ueProfilerNode* temp = node;
				while (temp && ueStrCmp(temp->m_name, nodeName))
					temp = temp->m_nextSibling;
				if (temp)
					node = temp;
				else
					ueMemCpy(nodeName, node->m_name, ueStrLen(node->m_name) + 1);
				s_data->m_graphLength = 0;
				s_data->m_numProfCapturePathSegments++;
			}
		}
		else if (inSys_WasPressed(s_data->m_inputLeft))
		{
			if (nodeFirst != tree.m_root)
			{
				node = parent;
				nodeFirst = parentFirst;
				s_data->m_graphLength = 0;

				s_data->m_numProfCapturePathSegments--;
			}
		}
	}

	// Draw full node name

	const f32 textScale = 1.0f;
	char buffer[512];

	gxText text;
	text.m_x = pos[0];
	text.m_y = pos[1];
	text.m_scaleX = text.m_scaleY = textScale;
	text.m_utf8Buffer = buffer;
	text.m_color = ueColor32::Red;

	u32 bufferLength = 0;
	nodeName = s_data->m_profCapturePath;
	for (u32 i = 0; i < s_data->m_numProfCapturePathSegments; i++)
	{
		const u32 nodeNameLength = ueStrLen(nodeName);

		buffer[bufferLength++] = ' ';
		ueStrCpy(buffer + bufferLength, UE_ARRAY_SIZE(buffer) - bufferLength, nodeName);
		bufferLength += nodeNameLength;
		nodeName += nodeNameLength + 1;

		if (i + 1 < s_data->m_numProfCapturePathSegments)
		{
			buffer[bufferLength++] = ' ';
			buffer[bufferLength++] = '-';
			buffer[bufferLength++] = '>';
		}
	}
	buffer[bufferLength++] = ':';
	buffer[bufferLength++] = 0;

	gxTextRenderer_Draw(ctx, &text);

	// Draw column names

	ueStrFormatS(buffer, columnsText, NULL);
	const s32 numberStart = ueStrFirstIndexOf(buffer, '|');
	text.m_y += 10.0f + gxFont_GetSizeInPixels(font) * textScale;
	gxTextRenderer_Draw(ctx, &text);

	// Draw children

	const f32 frameTime = (f32) tree.m_root->m_inclNanoSecs;
	const f32 parentInclTime = parent ? parent->m_inclNanoSecs : frameTime;

	text.m_y += 10.0f;

	u32 index = 0;
	while (nodeFirst)
	{
		text.m_color = nodeFirst == node ? ueColor32::Yellow : ueColor32::Green;
		text.m_y += gxFont_GetSizeInPixels(font) * textScale;
		ueStrFormatS(buffer, "%s %s", nodeFirst->m_name, nodeFirst->m_firstChild ? "[...]" : "");

		bufferLength = ueStrLen(buffer);
		ueMemSet(buffer + bufferLength, ' ', numberStart - bufferLength);
		ueStrFormat(buffer + numberStart, UE_ARRAY_SIZE(buffer) - numberStart,
			"| %5.1f | %5.1f | %7u | %7u",
			nodeFirst->m_inclNanoSecs * 100.0f / parentInclTime,
			nodeFirst->m_inclNanoSecs * 100.0f / frameTime,
			nodeFirst->m_inclNanoSecs / 1000,
			nodeFirst->m_exclNanoSecs / 1000);

		gxTextRenderer_Draw(ctx, &text);

		nodeFirst = nodeFirst->m_nextSibling;
		index++;
	}

	// Update graph

	if (node && !ueVar_GetBool(s_data->m_profilerCapturePause))
	{
		UE_ASSERT(ueIsPow2(UE_ARRAY_SIZE(s_data->m_graph)));

		const u32 sizeModulo = UE_ARRAY_SIZE(s_data->m_graph) - 1;

		// Add new entry to graph

		s_data->m_graph[(s_data->m_graphStart + s_data->m_graphLength) & sizeModulo] = (f32) (node->m_exclNanoSecs >> 10);

		// Update graph length (or start point)

		if (s_data->m_graphLength < UE_ARRAY_SIZE(s_data->m_graph))
			s_data->m_graphLength++;
		else
			s_data->m_graphStart = (s_data->m_graphStart + 1) & sizeModulo;
	}

	// Draw noncaptured branch

	if (parent)
	{
		const u32 parentExclTime = parent->m_exclNanoSecs;

		text.m_color = ueColor32::Green;
		text.m_y += gxFont_GetSizeInPixels(font) * textScale;
		ueStrCpyS(buffer, "<uncaptured>");

		bufferLength = ueStrLen(buffer);
		ueMemSet(buffer + bufferLength, ' ', numberStart - bufferLength);
		ueStrFormat(buffer + numberStart, UE_ARRAY_SIZE(buffer) - numberStart,
			"| %5.1f | %5.1f | %7u | %7u",
			parentExclTime * 100.0f / parentInclTime,
			parentExclTime * 100.0f / frameTime,
			parentExclTime / 1000,
			parentExclTime / 1000);

		gxTextRenderer_Draw(ctx, &text);
	}
}

void gxProfiler_DrawProfilerFunc(glCtx* ctx, gxDebugWindow* window, void* userData)
{
	if (!s_data->m_profCapture)
		return;

	switch (s_data->m_profView)
	{
		case gxProfilerView_Threads: gxProfiler_DrawProfiler_Threads(ctx, window); break;
		case gxProfilerView_Tree: gxProfiler_UpdateAndDrawProfiler_Tree(ctx, window); break;
	}
}

void gxProfiler_DrawResources(glCtx* ctx)
{
	// TODO
}
/*
void gxProfiler_Draw(glCtx* ctx)
{
	UE_ASSERT(s_data);

	if (!s_data->m_isEnabled)
		return;

	UE_PROF_SCOPE("gxProfiler_Draw");

	if (ueVar_GetBool(s_data->m_showFPS))
		gxDebugWindow_SetVisible(s_data->m_fpsWindow, ???);
	if (ueVar_GetBool(s_data->m_showMem))
		gxProfiler_DrawMem(ctx);
	if (ueVar_GetBool(s_data->m_showWarnings))
		gxProfiler_DrawWarnings(ctx);
	if (ueVar_GetBool(s_data->m_showProfiler))
		gxProfiler_DrawProfiler(ctx);
	if (ueVar_GetBool(s_data->m_showResources))
		gxProfiler_DrawResources(ctx);
}
*/
void gxProfiler_AddAllocator(ueAllocator* allocator)
{
	UE_ASSERT(s_data);
	s_data->m_allocators.PushBack(allocator);
}

void gxProfiler_RemoveAllocator(ueAllocator* allocator)
{
	UE_ASSERT(s_data);
	s_data->m_allocators.RemoveFirstEqual(allocator);
}

ueBool gxProfiler_IsWaitingForProfilerCapture()
{
	UE_ASSERT(s_data);
	return
		!s_data->m_profCapture ||
		(!ueVar_GetBool(s_data->m_profilerCapturePause) &&
		ueClock_GetSecsSince(s_data->m_profCapture->m_startTime) >= ueVar_GetF32(s_data->m_profilerCaptureFreq));
}

void gxProfiler_SetProfilerCapture(const ueProfilerCapture* profCapture)
{
	UE_ASSERT(s_data);
	s_data->m_profCapture = profCapture;
}

void gxProfiler_Enable(ueBool enable)
{
	UE_ASSERT(s_data);
	s_data->m_isEnabled = enable;
	if (!enable)
		inSys_EnableConsumer(s_data->m_inputConsumerId, UE_FALSE);
	else if (s_data->m_isInputEnabled)
		inSys_EnableConsumer(s_data->m_inputConsumerId, UE_TRUE);
}

ueBool gxProfiler_IsEnabled()
{
	UE_ASSERT(s_data);
	return s_data->m_isEnabled;
}

void gxProfiler_EnableInput(ueBool enable)
{
	UE_ASSERT(s_data);
	s_data->m_isInputEnabled = enable;
	if (s_data->m_isEnabled)
		inSys_EnableConsumer(s_data->m_inputConsumerId, enable);
}

ueBool gxProfiler_IsInputEnabled()
{
	UE_ASSERT(s_data);
	return s_data->m_isInputEnabled;
}

void gxProfiler_SetResourceVisualizationCallback(u32 resourceType, gxResourceVisualizationCallback callback, void* userData)
{
	UE_ASSERT(s_data);
}

#include "GraphicsExt/gxDebugMenu.h"
#include "Base/ueEnv.h"
#include "Base/Containers/ueList.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxDebugWindow.h"

struct gxDebugMenuItem : ueList<gxDebugMenuItem>::Node
{
	enum Type
	{
		Type_Menu = 0,
		Type_Var,
		Type_Command,
		Type_UserAction,

		Type_MAX
	};

	Type m_type;
	char* m_name;
	gxDebugMenuItem* m_parent;

	gxDebugMenuItem(Type type) :
		m_type(type),
		m_parent(NULL)
	{}
};

struct gxDebugMenuItem_Menu : gxDebugMenuItem
{
	ueList<gxDebugMenuItem> m_children;

	gxDebugMenuItem_Menu() : gxDebugMenuItem(Type_Menu) {}
};

struct gxDebugMenuItem_Var : gxDebugMenuItem
{
	ueVar* m_var;

	gxDebugMenuItem_Var() : gxDebugMenuItem(Type_Var) {}
};

struct gxDebugMenuItem_Command : gxDebugMenuItem
{
	char* m_command;
	ueTime m_lastExecTime;

	gxDebugMenuItem_Command() : gxDebugMenuItem(Type_Command), m_lastExecTime(0) {}
};

struct gxDebugMenuItem_UserAction : gxDebugMenuItem
{
	void* m_userData;
	gxDebugMenuCallback m_callback;
	ueTime m_lastExecTime;

	gxDebugMenuItem_UserAction() : gxDebugMenuItem(Type_UserAction), m_lastExecTime(0) {}
};

struct gxDebugMenuData
{
	ueAllocator* m_allocator;

	ueList<gxDebugMenuItem> m_roots;

	gxDebugMenuItem_Menu* m_currentMenu;
	gxDebugMenuItem* m_currentItem;
	ueBool m_isEnabled;

	ueResourceHandle<gxFont> m_font;

	ueVar* m_drawScale;
	ueVar* m_textTransparency;
	ueVar* m_backgroundTransparency;

	inConsumerId m_inputConsumerId;
	inEventId m_inputLeft;
	inEventId m_inputRight;
	inEventId m_inputUp;
	inEventId m_inputDown;
	inEventId m_inputAccept;
	inEventId m_inputBack;

	gxDebugWindow* m_window;
};

void gxDebugMenu_DrawFunc(glCtx* ctx, gxDebugWindow* window, void* userData);

static gxDebugMenuData* s_data = NULL;

void gxDebugMenu_LoadDebugWindowsLayout(const char*, void*)
{
	gxDebugWindowSys_LoadLayout();
}

void gxDebugMenu_SaveDebugWindowsLayout(const char*, void*)
{
	gxDebugWindowSys_SaveLayout();
}

void gxDebugMenu_CreateInitialMenu()
{
	gxDebugMenuItem* debugMenu = gxDebugMenu_CreateMenu(NULL, "Debug");
	gxDebugMenu_CreateVar(debugMenu, s_data->m_drawScale);
	gxDebugMenu_CreateVar(debugMenu, s_data->m_textTransparency);
	gxDebugMenu_CreateVar(debugMenu, s_data->m_backgroundTransparency);
	gxDebugMenu_CreateUserAction(debugMenu, "Load Debug Windows Layout", gxDebugMenu_LoadDebugWindowsLayout);
	gxDebugMenu_CreateUserAction(debugMenu, "Save Debug Windows Layout", gxDebugMenu_SaveDebugWindowsLayout);
}

void gxDebugMenu_StartupEnv(ueAllocator* allocator)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxDebugMenuData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_currentMenu = NULL;
	s_data->m_currentItem = NULL;
	s_data->m_isEnabled = UE_FALSE;

	s_data->m_drawScale = ueVar_CreateF32("debug_menu.draw_scale", 1.0f, 0.2f, 5.0f, 0.1f);
	s_data->m_textTransparency = ueVar_CreateF32("debug_menu.text_transparency", 1.0f, 0.1f, 1.0f, 0.1f);
	s_data->m_backgroundTransparency = ueVar_CreateF32("debug_menu.background_transparency", 0.6f, 0.1f, 1.0f, 0.1f);
}

void gxDebugMenu_Startup(gxDebugMenuStartupParams* params)
{
	UE_ASSERT_MSG(s_data, "Must call gxDebugMenu_StartupEnv() before.");

	s_data->m_font.SetByName(params->m_fontName);

	s_data->m_inputConsumerId = inSys_RegisterConsumer("debug menu", params->m_inputPriority);
	inSys_EnableConsumer(s_data->m_inputConsumerId, UE_FALSE);
	s_data->m_inputLeft = inSys_RegisterEvent(s_data->m_inputConsumerId, "left", TempPtr(inBinding(inDev_Keyboard, inKey_Left)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Left)));
	s_data->m_inputRight = inSys_RegisterEvent(s_data->m_inputConsumerId, "right", TempPtr(inBinding(inDev_Keyboard, inKey_Right)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Right)));
	s_data->m_inputUp = inSys_RegisterEvent(s_data->m_inputConsumerId, "up", TempPtr(inBinding(inDev_Keyboard, inKey_Up)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Up)));
	s_data->m_inputDown = inSys_RegisterEvent(s_data->m_inputConsumerId, "down", TempPtr(inBinding(inDev_Keyboard, inKey_Down)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Down)));
	s_data->m_inputAccept = inSys_RegisterEvent(s_data->m_inputConsumerId, "accept", TempPtr(inBinding(inDev_Keyboard, inKey_Return)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_A)));
	s_data->m_inputBack = inSys_RegisterEvent(s_data->m_inputConsumerId, "back", TempPtr(inBinding(inDev_Keyboard, inKey_Escape)), TempPtr(inBinding(inDev_Gamepad, inGamepadButton_B)));

	gxDebugMenu_CreateInitialMenu();

	gxDebugWindowDesc windowDesc;
	windowDesc.m_autoSize = UE_FALSE;
	windowDesc.m_drawFunc = gxDebugMenu_DrawFunc;
	windowDesc.m_name = "Debug Menu";
	windowDesc.m_showName = UE_FALSE;
	windowDesc.m_rect.SetLeftTopAndSize(3, 3, 256, 512);
	windowDesc.m_isVisible = UE_FALSE;
	s_data->m_window = gxDebugWindow_Create(&windowDesc);
	UE_ASSERT(s_data->m_window);
}

void gxDebugMenu_Destroy(ueList<gxDebugMenuItem>& items)
{
	while (gxDebugMenuItem* item = items.PopFront())
	{
		switch (item->m_type)
		{
			case gxDebugMenuItem::Type_Command:
			{
				gxDebugMenuItem_Command* commandItem = (gxDebugMenuItem_Command*) item;
				ueDelete(commandItem->m_command, s_data->m_allocator);
				break;
			}
			case gxDebugMenuItem::Type_Menu:
			{
				gxDebugMenuItem_Menu* menuItem = (gxDebugMenuItem_Menu*) item;
				gxDebugMenu_Destroy(menuItem->m_children);
				break;
			}
		}

		ueDelete(item->m_name, s_data->m_allocator);
		ueDelete(item, s_data->m_allocator);
	}
}

void gxDebugMenu_Shutdown()
{
	UE_ASSERT(s_data);

	gxDebugWindow_Destroy(s_data->m_window);

	inSys_UnregisterConsumer(s_data->m_inputConsumerId);

	UE_ASSERT(s_data->m_drawScale);
	ueVar_Destroy(s_data->m_drawScale);
	ueVar_Destroy(s_data->m_textTransparency);
	ueVar_Destroy(s_data->m_backgroundTransparency);
	s_data->m_drawScale = NULL;

	gxDebugMenu_Destroy(s_data->m_roots);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool gxDebugMenu_IsEnabled()
{
	UE_ASSERT(s_data);
	return s_data->m_isEnabled;
}

void gxDebugMenu_Enable(ueBool enable)
{
	UE_ASSERT(s_data);
	s_data->m_isEnabled = enable;
	inSys_EnableConsumer(s_data->m_inputConsumerId, enable);
	gxDebugWindow_SetVisible(s_data->m_window, enable);
	if (s_data->m_isEnabled)
		gxDebugWindow_BringToFront(s_data->m_window);
}

void gxDebugMenu_Clear()
{
	UE_ASSERT(s_data);

	gxDebugMenu_Destroy(s_data->m_roots);

	s_data->m_currentMenu = NULL;
	s_data->m_currentItem = NULL;

	gxDebugMenu_CreateInitialMenu();
}

gxDebugMenuItem* gxDebugMenu_GetItem(const char* _name)
{
	UE_ASSERT(s_data);

	char name[2048];
	ueStrCpy(name, UE_ARRAY_SIZE(name), _name);

	char* parts[16];
	u32 numParts = UE_ARRAY_SIZE(parts);
	ueStrTokenize(name, ".", parts, &numParts);

	gxDebugMenuItem* item = s_data->m_roots.Front();
	for (u32 i = 0; i < numParts; i++)
	{
		while (item && ueStrCmp(item->m_name, parts[i]))
			item = item->Next();
		if (!item)
			return NULL;
	}
	return item;
}

void gxDebugMenu_AddItem(gxDebugMenuItem* parent, gxDebugMenuItem* item)
{
	if (!parent)
	{
		s_data->m_roots.PushBack(item);

		if (!s_data->m_currentItem)
		{
			s_data->m_currentMenu = NULL;
			s_data->m_currentItem = s_data->m_roots.Front();
		}
		return;
	}

	item->m_parent = parent;

	UE_ASSERT(parent->m_type == gxDebugMenuItem::Type_Menu);
	gxDebugMenuItem_Menu* parentMenu = (gxDebugMenuItem_Menu*) parent;
	parentMenu->m_children.PushBack(item);
}

template <class ITEM_TYPE>
ITEM_TYPE* gxDebugMenu_CreateItem(const char* name)
{
	UE_ASSERT(s_data);

	ITEM_TYPE* item = new(s_data->m_allocator) ITEM_TYPE();
	UE_ASSERT(item);
	const u32 nameLength = ueStrLen(name);
	item->m_name = (char*) s_data->m_allocator->Alloc(nameLength + 1);
	UE_ASSERT(item->m_name);
	ueMemCpy(item->m_name, name, nameLength + 1);
	return item;
}

gxDebugMenuItem* gxDebugMenu_CreateMenu(gxDebugMenuItem* parent, const char* menuName)
{
	gxDebugMenuItem_Menu* item = gxDebugMenu_CreateItem<gxDebugMenuItem_Menu>(menuName);
	gxDebugMenu_AddItem(parent, item);
	return item;
}

gxDebugMenuItem* gxDebugMenu_CreateVar(gxDebugMenuItem* parent, const char* varName)
{
	ueVar* var = ueEnv_FindVar(varName);
	UE_ASSERT(var);
	return gxDebugMenu_CreateVar(parent, var);
}

gxDebugMenuItem* gxDebugMenu_CreateVar(gxDebugMenuItem* parent, ueVar* var)
{
	gxDebugMenuItem_Var* item = gxDebugMenu_CreateItem<gxDebugMenuItem_Var>( ueVar_GetName(var) );
	item->m_var = var;
	gxDebugMenu_AddItem(parent, item);
	return item;
}

gxDebugMenuItem* gxDebugMenu_CreateCommand(gxDebugMenuItem* parent, const char* commandName, const char* command)
{
	gxDebugMenuItem_Command* item = gxDebugMenu_CreateItem<gxDebugMenuItem_Command>(commandName);
	const u32 commandLength = ueStrLen(command);
	item->m_command = (char*) s_data->m_allocator->Alloc(commandLength + 1);
	UE_ASSERT(item->m_command);
	ueMemCpy(item->m_command, command, commandLength + 1);
	gxDebugMenu_AddItem(parent, item);
	return item;
}

gxDebugMenuItem* gxDebugMenu_CreateUserAction(gxDebugMenuItem* parent, const char* actionName, gxDebugMenuCallback callback, void* userData)
{
	gxDebugMenuItem_UserAction* item = gxDebugMenu_CreateItem<gxDebugMenuItem_UserAction>(actionName);
	item->m_callback = callback;
	item->m_userData = userData;
	gxDebugMenu_AddItem(parent, item);
	return item;
}

void gxDebugMenu_Update(f32 dt)
{
	UE_ASSERT(s_data);

	if (!s_data->m_isEnabled)
		return;

	UE_PROF_SCOPE("gxDebugMenu_Update");

	const ueBool isRight = inSys_WasPressed(s_data->m_inputRight);
	const ueBool isLeft = inSys_WasPressed(s_data->m_inputLeft);
	const ueBool isUp = inSys_WasPressed(s_data->m_inputUp);
	const ueBool isDown = inSys_WasPressed(s_data->m_inputDown);
	const ueBool isOpen = inSys_WasPressed(s_data->m_inputAccept);
	const ueBool isClose = inSys_WasPressed(s_data->m_inputBack);

	if (isUp)
	{
		if (!s_data->m_currentItem)
			s_data->m_currentItem = s_data->m_currentMenu ? s_data->m_currentMenu->m_children.Back() : NULL;
		else if (s_data->m_currentItem->Previous())
			s_data->m_currentItem = s_data->m_currentItem->Previous();
		else
			s_data->m_currentItem = s_data->m_currentMenu ? NULL : s_data->m_roots.Back();
	}
	else if (isDown)
	{
		if (!s_data->m_currentItem)
			s_data->m_currentItem = s_data->m_currentMenu ? s_data->m_currentMenu->m_children.Front() : s_data->m_roots.Front();
		else if (s_data->m_currentItem->Next())
			s_data->m_currentItem = s_data->m_currentItem->Next();
		else
			s_data->m_currentItem = s_data->m_currentMenu ? NULL : s_data->m_roots.Front();
	}
	else if (isOpen)
	{
		if (!s_data->m_currentItem)
		{
			if (s_data->m_currentMenu)
			{
				s_data->m_currentItem = s_data->m_currentMenu;
				s_data->m_currentMenu = NULL;
			}
		}
		else switch (s_data->m_currentItem->m_type)
		{
			case gxDebugMenuItem::Type_Menu:
			{
				gxDebugMenuItem_Menu* menuItem = (gxDebugMenuItem_Menu*) s_data->m_currentItem;
				if (menuItem->m_children.Front())
				{
					s_data->m_currentMenu = menuItem;
					s_data->m_currentItem = NULL;
				}
				break;
			}

			case gxDebugMenuItem::Type_Command:
			{
				gxDebugMenuItem_Command* commandItem = (gxDebugMenuItem_Command*) s_data->m_currentItem;
				ueEnv_DoString(commandItem->m_command);
				commandItem->m_lastExecTime = ueClock_GetCurrent();
				break;
			}

			case gxDebugMenuItem::Type_UserAction:
			{
				gxDebugMenuItem_UserAction* userActionItem = (gxDebugMenuItem_UserAction*) s_data->m_currentItem;
				userActionItem->m_callback(userActionItem->m_name, userActionItem->m_userData);
				userActionItem->m_lastExecTime = ueClock_GetCurrent();
				break;
			}
		}
	}
	else if (isClose)
	{
		if (s_data->m_currentMenu)
		{
			s_data->m_currentItem = s_data->m_currentMenu;
			s_data->m_currentMenu = NULL;
		}
	}
	else if (isRight)
	{
		if (s_data->m_currentItem)
			switch (s_data->m_currentItem->m_type)
			{
				case gxDebugMenuItem::Type_Var:
				{
					gxDebugMenuItem_Var* varItem = (gxDebugMenuItem_Var*) s_data->m_currentItem;
					ueVar_IncStep(varItem->m_var);
					break;
				}
			}
	}
	else if (isLeft)
	{
		if (s_data->m_currentItem)
			switch (s_data->m_currentItem->m_type)
			{
				case gxDebugMenuItem::Type_Var:
				{
					gxDebugMenuItem_Var* varItem = (gxDebugMenuItem_Var*) s_data->m_currentItem;
					ueVar_DecStep(varItem->m_var);
					break;
				}
			}
	}
}

void gxDebugMenu_DrawFunc(glCtx* ctx, gxDebugWindow* window, void* userData)
{
	UE_ASSERT(s_data);

	if (!s_data->m_isEnabled)
		return;

	UE_PROF_SCOPE("gxDebugMenu_Draw");

	const f32 execFadeOutTimeSecs = 0.5f;

	const f32 scale = ueVar_GetF32(s_data->m_drawScale);
	const f32 textAlpha = ueVar_GetF32(s_data->m_textTransparency);
	const f32 backgroundAlpha = ueVar_GetF32(s_data->m_backgroundTransparency);

	const f32 fontSize = (f32) gxFont_GetSizeInPixels(*s_data->m_font);

	// Determine rectangle height

	u32 numItems = s_data->m_currentMenu ? 2 : 1;
	{
		gxDebugMenuItem* curr = s_data->m_currentMenu ? s_data->m_currentMenu->m_children.Front() : s_data->m_roots.Front();
		while (curr)
		{
			numItems++;
			curr = curr->Next();
		}
	}

	gxDebugWindow_SetSize(window, fontSize * scale * 40 + 10, fontSize * scale * numItems + 10);

	// Draw background

	gxDebugWindow_SetTransparency(window, ueVar_GetF32(s_data->m_backgroundTransparency));
	gxDebugWindow_DrawBackground(ctx, window);

	// Draw the text

	ueVec2 pos;
	gxDebugWindow_GetDrawPos(pos, window);

	char buffer[1024];

	const f32 x = pos[0];
	f32 y = pos[1];

	gxText text;
	text.m_x = x;
	text.m_y = y;
	text.m_font = *s_data->m_font;
	text.m_scaleX = text.m_scaleY = scale;
	text.m_color.SetAlphaF(textAlpha);

	// Draw header

	text.m_utf8Buffer = s_data->m_currentMenu ? s_data->m_currentMenu->m_name : "Main Menu";
	text.m_color = ueColor32::White;
	text.m_color.SetAlphaF(textAlpha);
	gxTextRenderer_Draw(ctx, &text);
	y += fontSize * scale;

	if (s_data->m_currentMenu)
	{
		ueStrFormatS(buffer, "%c[...]", !s_data->m_currentItem ? '>' : ' ');
		text.m_utf8Buffer = buffer;
		text.m_color = !s_data->m_currentItem ? ueColor32::Green : ueColor32::White;
		text.m_color.SetAlphaF(textAlpha);
		text.m_y = y;
		gxTextRenderer_Draw(ctx, &text);
		y += fontSize * scale;
	}

	// Draw all items

	gxDebugMenuItem* curr = s_data->m_currentMenu ? s_data->m_currentMenu->m_children.Front() : s_data->m_roots.Front();
	while (curr)
	{
		text.m_color = curr == s_data->m_currentItem ? ueColor32::Green : ueColor32::White;

		switch (curr->m_type)
		{
			case gxDebugMenuItem::Type_Menu:
			{
				gxDebugMenuItem_Menu* menuItem = (gxDebugMenuItem_Menu*) curr;
				ueStrFormatS(buffer, " %s ... [%d]", curr->m_name, menuItem->m_children.Length());
				break;
			}
			case gxDebugMenuItem::Type_Var:
			{
				gxDebugMenuItem_Var* varItem = (gxDebugMenuItem_Var*) curr;

				ueStrFormatS(buffer, " <%s> = ", curr->m_name);
				const u32 bufferLength = ueStrLen(buffer);

				ueVar_GetAsString(varItem->m_var, buffer + bufferLength, UE_ARRAY_SIZE(buffer) - bufferLength);
				break;
			}
			case gxDebugMenuItem::Type_Command:
			{
				gxDebugMenuItem_Command* commandItem = (gxDebugMenuItem_Command*) curr;
				const f32 secsSinceExec = ueClock_GetSecsSince(commandItem->m_lastExecTime);
				if (secsSinceExec < execFadeOutTimeSecs)
					text.m_color = ueColor32::Lerp(ueColor32::Red, text.m_color, secsSinceExec / execFadeOutTimeSecs);
				ueStrFormatS(buffer, " [%s]", curr->m_name);
				break;
			}
			case gxDebugMenuItem::Type_UserAction:
			{
				gxDebugMenuItem_UserAction* userActionItem = (gxDebugMenuItem_UserAction*) curr;
				const f32 secsSinceExec = ueClock_GetSecsSince(userActionItem->m_lastExecTime);
				if (secsSinceExec < execFadeOutTimeSecs)
					text.m_color = ueColor32::Lerp(ueColor32::Red, text.m_color, secsSinceExec / execFadeOutTimeSecs);
				ueStrFormatS(buffer, " [%s]", curr->m_name);
				break;
			}
		}

		if (curr == s_data->m_currentItem)
			buffer[0] = '>';

		text.m_utf8Buffer = buffer;
		text.m_x = x;
		text.m_y = y;
		text.m_color.SetAlphaF(textAlpha);

		gxTextRenderer_Draw(ctx, &text);

		curr = curr->Next();

		y += fontSize * scale;
	}
}

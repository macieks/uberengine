#ifndef GX_DEBUG_MENU_H
#define GX_DEBUG_MENU_H

#include "Base/ueBase.h"

struct glCtx;
struct ueVar;

/**
 *	@addtogroup gx
 *	@{
 */

/**
 *	@struct gxDebugMenuItem
 *	@brief Debug menu item
 */
struct gxDebugMenuItem;
//! Debug menu callback for "action items"
typedef void (*gxDebugMenuCallback)(const char* actionName, void* userData);

//! Debug menu startup parameters
struct gxDebugMenuStartupParams
{
	const char* m_fontName;			//!< Debug menu font name
	f32 m_inputPriority;			//!< Debug menu input priority

	gxDebugMenuStartupParams() :
		m_fontName("common/debug_font"),
		m_inputPriority(GX_DEFAULT_DEBUG_MENU_INPUT_PRIORITY)
	{}
};

//! Starts up environment part (environment functions and variables) of the debug menu
void gxDebugMenu_StartupEnv(ueAllocator* allocator);
//! Starts up debug menu
void gxDebugMenu_Startup(gxDebugMenuStartupParams* params);
//! Shuts down debug menu
void gxDebugMenu_Shutdown();

//! Updates debug menu
void gxDebugMenu_Update(f32 dt);

//! Tells whether debug menu is enabled
ueBool gxDebugMenu_IsEnabled();
//! Toggles debug menu
void gxDebugMenu_Enable(ueBool enable);

//! Resets debug menu (removes all menu items)
void gxDebugMenu_Clear();

//! Gets debug menu item of given name
gxDebugMenuItem* gxDebugMenu_GetItem(const char* name);
//! Creates menu item which can contain other items
gxDebugMenuItem* gxDebugMenu_CreateMenu(gxDebugMenuItem* parent, const char* menuName);
//! Creates an item containing editable environment variable
gxDebugMenuItem* gxDebugMenu_CreateVar(gxDebugMenuItem* parent, const char* varName);
//! Creates an item containing editable environment variable
gxDebugMenuItem* gxDebugMenu_CreateVar(gxDebugMenuItem* parent, ueVar* var);
//! Creates and item containing command that can be executed when this item is selected in debug menu
gxDebugMenuItem* gxDebugMenu_CreateCommand(gxDebugMenuItem* parent, const char* commandName, const char* command);
//! Creates custom user action item which can be executed when selected in debug menu
gxDebugMenuItem* gxDebugMenu_CreateUserAction(gxDebugMenuItem* parent, const char* actionName, gxDebugMenuCallback callback, void* userData = NULL);

// @}

#endif // GX_DEBUG_MENU_H

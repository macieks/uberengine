#ifndef GX_DEBUG_WINDOW_H
#define GX_DEBUG_WINDOW_H

#include "Base/ueShapes.h"

struct glCtx;

/**
 *	@addtogroup gx
 *	@{
 */

/**
 *	@struct gxDebugWindow
 *	@brief Debug window used to display text (or anything else via custom draw callback).
 */
struct gxDebugWindow;

//! Debug window draw callback
typedef void (*gxDebugWindow_DrawFunc)(glCtx* ctx, gxDebugWindow* window, void* userData);

//! Debug window description
struct gxDebugWindowDesc
{
	const char* m_name;				//!< Window name / title
	ueBool m_showName;				//!< Indicates whether to display window name at the top

	const char* m_content;			//!< Optional initial window content / text

	ueRect m_rect;					//!< Window rectangle

	f32 m_transparency;				//!< Window transparency (background, outlines, etc.)
	f32 m_textTransparency;			//!< Window text transparency

	ueBool m_isVisible;				//!< Indicates whether window is visible at startup
	ueBool m_autoSize;				//!< Indicates whether to calculate window size automatically based on its text dimensions
	ueBool m_hasResizeButton;
	ueBool m_hasCloseButton;
	ueBool m_switchable;			//!< Indicates whether it's possible to switch to window (via CTRL+TAB)

	gxDebugWindow_DrawFunc m_drawFunc;	//!< Optional draw function

	void* m_userData;				//!< Custom user data

	gxDebugWindowDesc() :
		m_name(NULL),
		m_showName(UE_TRUE),
		m_content(NULL),
		m_rect(50, 50, 100, 100),
		m_transparency(0.75f),
		m_textTransparency(1.0f),
		m_isVisible(UE_TRUE),
		m_autoSize(UE_TRUE),
		m_hasResizeButton(UE_TRUE),
		m_hasCloseButton(UE_TRUE),
		m_switchable(UE_TRUE),
		m_drawFunc(NULL),
		m_userData(NULL)
	{}
};

//! Starts up debug window system
void	gxDebugWindowSys_Startup(ueAllocator* allocator);
//! Starts up debug window system part that requires loading resources; to be called after resource manager initialization
void	gxDebugWindowSys_StartupResources();
//! Shuts down debug window system
void	gxDebugWindowSys_Shutdown();
//! Shuts down debug window system that requires unloading resources; to be called before resource manager shutdown
void	gxDebugWindowSys_ShutdownResources();
//! Gets whether debug window system is initialized
ueBool	gxDebugWindowSys_IsInitialized();
//! Toggles on & off interaction with debug windows
void	gxDebugWindowSys_SetActive(ueBool active);
//! Updates all debug windows
void	gxDebugWindowSys_Update(f32 dt);
//! Draws all (visible) debug windows
void	gxDebugWindowSys_Draw(glCtx* ctx);
//! Loads window layout from file
ueBool	gxDebugWindowSys_LoadLayout(const char* layoutFileName = "debug_windows");
//! Saves window layout to a file
ueBool	gxDebugWindowSys_SaveLayout(const char* layoutFileName = "debug_windows");

//! Creates debug window
gxDebugWindow* gxDebugWindow_Create(gxDebugWindowDesc* desc);
//! Destroys debug window
void gxDebugWindow_Destroy(gxDebugWindow* window);

//! Clear window content / text
void gxDebugWindow_ClearContent(gxDebugWindow* window);
//! Appends window content / text
void gxDebugWindow_AppendContent(gxDebugWindow* window, const char* content);
//! Sets window content / text
void gxDebugWindow_SetContent(gxDebugWindow* window, const char* content);

//! Sets window transparency
void gxDebugWindow_SetTransparency(gxDebugWindow* window, f32 transparency);
//! Sets text transparency
void gxDebugWindow_SetTextTransparency(gxDebugWindow* window, f32 textTransparency);
//! Changes window visibility
void gxDebugWindow_SetVisible(gxDebugWindow* window, ueBool isVisible);
//! Sets user data
void gxDebugWindow_SetUserData(gxDebugWindow* window, void* userData);
//! Brings window to front
void gxDebugWindow_BringToFront(gxDebugWindow* window);

//! Sets window rectangle
void gxDebugWindow_SetRect(gxDebugWindow* window, const ueRect& rect);
//! Sets window width and height
void gxDebugWindow_SetSize(gxDebugWindow* window, f32 width, f32 height);
//! Sets window draw rectangle
void gxDebugWindow_SetDrawSize(gxDebugWindow* window, f32 width, f32 height);
//! Gets window left top corner
const ueVec2& gxDebugWindow_GetPos(gxDebugWindow* window);
//! Gets window left top draw rectangle's corner
void gxDebugWindow_GetDrawPos(ueVec2& out, gxDebugWindow* window);
//! Gets window draw rectangle
void gxDebugWindow_GetDrawRect(ueRect& out, gxDebugWindow* window);

//! Gets text transparency
f32 gxDebugWindow_GetTextTransparency(gxDebugWindow* window);
//! Gets whether window is visible
ueBool gxDebugWindow_IsVisible(gxDebugWindow* window);
//! Gets cursor position
void gxDebugWindow_GetCursorPos(ueVec2& pos);

//! Draws window background
void gxDebugWindow_DrawBackground(glCtx* ctx, gxDebugWindow* window);

// @}

#endif // GX_DEBUG_WINDOW_H
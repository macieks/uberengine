#ifndef UI_LIB_H
#define UI_LIB_H

/**
 *	@defgroup ui User Interface
 *	@brief User Interface library (currently the only implementation is based on libRocket)
 */

#include "Base/ueBase.h"

struct glCtx;

/**
 *	@addtogroup ui
 *	@{
 */

/**
 *	@struct uiScene
 *	@brief UI scene, e.g. main menu, options
 */
struct uiScene;

//! Available UI event types
enum uiEventType
{
	uiEventType_OnClick = 0,	//!< An element was clicked

	uiEventType_MAX
};

//! UI event listener function
typedef void (*uiEventListener)(uiEventType eventType, const char* value, void* userData);

//! UI library startup parameters
struct uiLibStartupParams
{
	ueAllocator* m_allocator;	//!< Allocator used for all UI memory allocations

	const s8* m_rootAssetPath;	//!< Root asset path for all UI resources (scenes, css files, images)

	ueBool m_enableDebugger;	//!< Indicates whether to enable debugger

	u32 m_maxEvents;			//!< Max. number of different events to register for

	u32 m_maxTextures;			//!< Max. number of UI textures
	u32 m_maxGeometries;		//!< Max. number of UI geometries
	u32 m_dynamicVBSize;		//!< Size of dynamic vertex buffer used by UI library
	u32 m_numDynamicIBIndices;	//!< Number of dynamic index buffer indices

	uiLibStartupParams() :
		m_allocator(NULL),
#if !defined(UE_FINAL)
		m_enableDebugger(UE_TRUE),
#else
		m_enableDebugger(UE_FALSE),
#endif
		m_maxEvents(256),
		m_rootAssetPath(""),
		m_maxTextures(256),
		m_maxGeometries(256),
		m_dynamicVBSize(1 << 16),
		m_numDynamicIBIndices(1 << 17)
	{}
};

//! Starts up UI library
void uiLib_Startup(uiLibStartupParams* params);
//! Shuts down UI library
void uiLib_Shutdown();
//! Toggles input handling for UI library
void uiLib_EnableInput(ueBool enable);
//! Sets UI canvas dimensions
void uiLib_SetDimensions(u32 width, u32 height);
//! Loads UI cursor
ueBool uiLib_LoadCursor(const char* name);
//! Loads UI font
ueBool uiLib_LoadFont(const char* name);
//! Sets UI event listener
void uiLib_SetEventListener(uiEventListener listener, void* userData);
//! Toggles UI debugger visibility (must initialize library with debugger)
void uiLib_ShowDebugger(ueBool show);
//! Tells whether UI debugger is visible
ueBool uiLib_IsDebuggerVisible();
//! Updates the whole UI
void uiLib_Update(f32 dt);
//! Draws the whole UI
void uiLib_Draw(glCtx* ctx);

//! Create UI scene
uiScene* uiScene_Create(const char* name);
//! Destroys UI scene
void uiScene_Destroy(uiScene* scene);
//! Sets UI scene visibility
void uiScene_SetVisible(uiScene* scene, ueBool visible);

// @}

#endif // UI_LIB_H
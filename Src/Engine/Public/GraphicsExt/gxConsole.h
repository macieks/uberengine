#ifndef GX_CONSOLE_H
#define GX_CONSOLE_H

#include "Base/ueBase.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Interactive console startup parameters
struct gxConsoleStartupParams
{
	ueAllocator* m_stackAllocator;	//!< Prefarably stack allocator

	u32 m_maxTextLines;				//!< Max. number of text lines
	u32 m_maxTextSize;				//!< Max. size of text buffer

	u32 m_maxHistoryTextLines;		//!< Max. number of history text lines
	u32 m_maxHistoryTextSize;		//!< Max. size of history text buffer

	gxConsoleStartupParams() :
		m_stackAllocator(NULL),
		m_maxTextLines(512),
		m_maxTextSize(1 << 16),
		m_maxHistoryTextLines(64),
		m_maxHistoryTextSize(1 << 10)
	{}
};

//! Starts up interactive console
void gxConsole_Startup(gxConsoleStartupParams* params);
//! Shuts down interactive console
void gxConsole_Shutdown();
//! Updates interactive console
void gxConsole_Update(f32 dt);

// @}

#endif // GX_CONSOLE_H

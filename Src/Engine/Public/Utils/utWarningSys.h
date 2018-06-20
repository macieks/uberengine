#ifndef UT_WARNING_SYS_H
#define UT_WARNING_SYS_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

#if defined(UE_ENABLE_WARNINGS)

// Warning system used to collect & report per-frame warnings

//! Starts up warning system
void utWarningSys_Startup(ueAllocator* allocator, ueSize bufferSize = 1024, u32 maxWarnings = 32);
//! Shuts down warning system
void utWarningSys_Shutdown();

//! Removes all queued warnings; typically called at the beginning of each frame
void utWarningSys_Reset();
//! Adds warning to the warning list
void utWarningSys_AddWarningV(const char* file, u32 line, const char* function, const char* condition, const char* msg, va_list args);

//! Gets number of warnings
u32 utWarningSys_GetNumWarnings();
//! Gets pointer to warning at given index
const char* utWarningSys_GetWarning(u32 index);

#define UT_WARNING(msg, ...) \
{ \
	va_list argList; \
	va_start(argList, msg); \
	utWarningSys_AddWarningV(__FILE__, __LINE__, __FUNCTION__, "", msg, argList); \
	va_end(argList); \
}

#define UT_WARNING_CHECK(condition, msg, ...) \
{ \
	va_list argList; \
	va_start(argList, msg); \
	utWarningSys_AddWarningV(__FILE__, __LINE__, __FUNCTION__, #condition, msg, argList); \
	va_end(argList); \
}

#else // defined(UE_ENABLE_WARNINGS)

#define UT_WARNING(msg, ...) {}
#define UT_WARNING_CHECK(condition, msg, ...) {}

#endif // defined(UE_ENABLE_WARNINGS)

// @}

#endif // UT_WARNING_SYS_H

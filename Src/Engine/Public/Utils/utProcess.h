#ifndef UT_PROCESS_H
#define UT_PROCESS_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

struct utProcess;

//! Starts up process subsystem
void		utProcess_Startup(u32 numArgs, const char** args);
//! Gets number of command line arguments
u32			utProcess_GetNumArgs();
//! Gets command line arguments at given index
const char*	utProcess_GetArg(u32 index);
//! Gets command line argument for name; returns NULL if not found; returns "" (non-NULL) if found but has empty value
const char*	utProcess_GetArgByName(const char* name);

struct utProcessParams
{
	const char* m_executableFileName;
	const char* m_arguments;
	ueBool m_wait;
	ueBool m_separateWindow;

	utProcessParams() :
		m_executableFileName(NULL),
		m_arguments(NULL),
		m_wait(UE_TRUE),
		m_separateWindow(UE_TRUE)
	{}
};

//! Spawns process; returns true on success, false othersize; gets process handle and exit code only if m_wait is set to UE_TRUE
ueBool utProcess_Spawn(utProcessParams* params, utProcess** process, s32* errorCode);
//! Closes process and gets exit code
void utProcess_Close(utProcess* process, s32* exitCode);

//! Gets whether debugger is attached to current process
ueBool utProcess_IsDebuggerAttached();

//! Converts command line into arguments
void utProcess_CommandLineToArgs(const char* srcBuffer, char* dstBuffer, u32 dstBufferCapacity, char** argv, u32 argvCapacity, u32* argc);

//! Gets application data directory for use by savegames etc.; includes trailing slash; if necessary creates company and app directories
ueBool utProcess_GetAppDataDir(const char* companyName, const char* appName, char* dstBuffer, u32 dstBufferCapacity);

//! Opens URL in a new window
void utProcess_OpenURL(const char* url);

// @}

#endif // UT_PROCESS_H
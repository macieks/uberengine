#ifndef UE_CALLSTACK_H
#define UE_CALLSTACK_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Starts up callstack helper
void ueCallstackHelper_Startup();
//! Shuts down up callstack helper
void ueCallstackHelper_Shutdown();

//! Gets current callstack; returns true on success, UE_FALSE otherwise
ueBool ueCallstackHelper_GetCallstack(void** frames, u32& framesSize, u32 numFramesToSkip = 0);
//! Gets callstack frame file name and line number; returns UE_TRUE on success, UE_FALSE otherwise
ueBool ueCallstackHelper_GetFrameDetails(void* frame, const char*& fileName, u32& lineNumber, char* symbolName = NULL, u32 symbolNameBufferSize = 0);

//! Dumps given callstack to logs
void ueCallstackHelper_DumpCallstack(void** frames, u32 numFrames);
//! Dumps current callstack to logs
void ueCallstackHelper_DumpCallstack(u32 numFramesToSkip = 2);

// @}

#endif // UE_CALLSTACK_H

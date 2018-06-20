#include "Base/ueBase.h"
#include "Base/ueCallstack.h"

void ueCallstackHelper_Startup() {}

void ueCallstackHelper_Shutdown() {}

ueBool ueCallstackHelper_GetCallstack(void** frames, u32& framesSize, u32 numFramesToSkip)
{
	return UE_FALSE;
}

ueBool ueCallstackHelper_GetFrameDetails(void* frame, const char*& fileName, u32& lineNumber, char* symbolName, u32 symbolNameBufferSize)
{
	return UE_FALSE;
}

#include "Base/Base.h"

#include <execinfo.h>

void ueCallstackHelper_Startup()
{
}

void ueCallstackHelper_Shutdown()
{
}

ueBool ueCallstackHelper_GetCallstack(void** frames, u32& framesSize, u32 framesToSkip)
{
    void* tempFrames[256];
    const u32 numRetrievedFrames = (u32) backtrace(tempFrames, UE_ARRAY_SIZE(tempFrames));

    if (numRetrievedFrames <= framesToSkip)
    {
        framesSize = 0;
        return UE_TRUE;
    }

    const u32 maxFrames = ueMin(framesSize, numRetrievedFrames - framesToSkip);

    framesSize = 0;
    while (framesSize < maxFrames)
    {
        frames[framesSize] = tempFrames[framesSize + framesToSkip];
        framesSize++;
    }
    return UE_TRUE;
}

ueBool ueCallstackHelper_GetFrameDetails(void* frame, const char*& fileName, u32& lineNumber)
{
    char** frameDesc = backtrace_symbols(&frame, 1);
    if (!frameDesc)
        return UE_FALSE;
    fileName = frameDesc[0]; // TODO: Extract file name from string into temp string
    lineNumber = 0; // TODO: Extract line number from string
    return UE_TRUE;
}

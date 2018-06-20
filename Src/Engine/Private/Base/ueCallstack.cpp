#include "Base/ueBase.h"

void ueCallstackHelper_DumpCallstack(void** frames, u32 numFrames)
{
	char symbolName[256];

	for (u32 i = 0; i < numFrames; ++i)
	{
		const char* fileName = NULL;
		u32 lineNumber = 0;
		if (!ueCallstackHelper_GetFrameDetails(frames[i], fileName, lineNumber, symbolName, UE_ARRAY_SIZE(symbolName)))
			ueLog("  0x%08x (Unknown file)\n", frames[i]);
		else
			ueLog("  %s(%d) [%s]\n", fileName, lineNumber, symbolName);
	}
}

void ueCallstackHelper_DumpCallstack(u32 numFramesToSkip)
{
	void* frames[128];
	u32 numFrames = UE_ARRAY_SIZE(frames);
	if (ueCallstackHelper_GetCallstack(frames, numFrames, numFramesToSkip))
		ueCallstackHelper_DumpCallstack(frames, numFrames);
}

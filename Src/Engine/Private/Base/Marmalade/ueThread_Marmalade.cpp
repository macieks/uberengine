#include "Base/ueBase.h"
#include "Threading/thThread.h"

thThreadId	thThread_Create(thThreadParams* params)
{
	if (!s3eThreadAvailable())
		return 0;

	ueAssert(0);
	return 0;
}

void		thThread_Join(thThreadId id) {}
ueBool		thThread_Start(thThreadId id) { return UE_FALSE; }
void		thThread_SetPriority(thThreadId id, f32 priority) {}

void		thThread_Exit(s32 exitCode) {}
void		thThread_Sleep(u32 milliSecs) { s3eDeviceYield(milliSecs); }
void		thThread_Yield() {}
thThreadId	thThread_GetCurrent() { return 0; }
ueBool		thThread_IsMain() { return UE_TRUE; }

u32			thThread_GetNumProcessors() { return 1; }

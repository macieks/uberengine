#include "Base/ueThreading.h"
#include "Base/Win32/ueThreading_Private_Win32.h"

ueEvent* ueEvent_Create(ueBool isManualReset)
{
	HANDLE handle = CreateEvent(NULL, isManualReset ? TRUE : FALSE, FALSE, NULL);
	UE_ASSERT(handle);
	return (ueEvent*) handle;
}

void ueEvent_Destroy(ueEvent* e)
{
	HANDLE handle = (HANDLE) e;
	const BOOL result = CloseHandle(handle);
	UE_ASSERT(result != 0);
}

void ueEvent_Reset(ueEvent* e)
{
	HANDLE handle = (HANDLE) e;
	ResetEvent(handle);
}

ueBool ueEvent_Wait(ueEvent* e, u32 milliseconds)
{
	HANDLE handle = (HANDLE) e;
	DWORD result = WaitForSingleObject(handle, milliseconds == 0 ? INFINITE : milliseconds);
	return result == WAIT_OBJECT_0;
}

void ueEvent_Signal(ueEvent* e)
{
	HANDLE handle = (HANDLE) e;
	DWORD result = SetEvent(handle);
	UE_ASSERT(result != 0);
}

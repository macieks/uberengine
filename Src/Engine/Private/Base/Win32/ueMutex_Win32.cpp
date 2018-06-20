#include "Base/ueThreading.h"
#include "Base/Win32/ueThreading_Private_Win32.h"

ueMutex* ueMutex_Create()
{
	ueMutex* m = new(g_threadingData.m_mutexPool) ueMutex;
	UE_ASSERT(m);
	InitializeCriticalSection(&m->m_handle);
	return m;
}

void ueMutex_Destroy(ueMutex* m)
{
	DeleteCriticalSection(&m->m_handle);
	ueDelete(m, g_threadingData.m_mutexPool);
}

void ueMutex_Lock(ueMutex* m)
{
	EnterCriticalSection(&m->m_handle);
}

void ueMutex_Unlock(ueMutex* m)
{
	LeaveCriticalSection(&m->m_handle);
}
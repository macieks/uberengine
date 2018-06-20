#include "Base/Win32/ueThreading_Private_Win32.h"

ueThreadingData g_threadingData;

void ueThreading_Startup(ueAllocator* allocator, u32 maxThreads, u32 maxMutexes, u32 maxEvents)
{
	UE_ASSERT(!g_threadingData.m_allocator);
	g_threadingData.m_allocator = allocator;
	g_threadingData.m_mutexPool.Init(allocator, sizeof(ueMutex), maxMutexes);
}

void ueThreading_Shutdown()
{
	UE_ASSERT(g_threadingData.m_allocator);
	g_threadingData.m_mutexPool.Deinit();
	g_threadingData.m_allocator = NULL;
}
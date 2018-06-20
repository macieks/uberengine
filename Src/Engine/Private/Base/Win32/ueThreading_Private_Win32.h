#ifndef UE_THREADING_PRIVATE_H
#define UE_THREADING_PRIVATE_H

#include "Base/Containers/ueGenericPool.h"

#if defined(UE_LINUX) || defined(UE_MAC)
    #include <semaphore.h>
#endif

struct ueMutex
{
#if defined(UE_WIN32) || defined(UE_X360)
	CRITICAL_SECTION m_handle;
#elif defined(UE_LINUX)
    pthread_mutex_t m_handle;
#elif defined(UE_MARMALADE)
	void* m_handle;
	s32 m_debugCounter;
#endif
};
/*
struct ueEvent
{
#if defined(UE_WIN32) || defined(UE_X360)
	HANDLE m_handle;
#elif defined(UE_LINUX)
    //pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
    //sem_t m_sem;
#elif defined(UE_MARMALADE)
	void* m_lock;
#endif
};
*/
struct ueThreadingData
{
	ueAllocator* m_allocator;
	ueGenericPool m_mutexPool;

	ueThreadingData() : m_allocator(NULL) {}
};

extern ueThreadingData g_threadingData;

#endif // UE_THREADING_PRIVATE_H
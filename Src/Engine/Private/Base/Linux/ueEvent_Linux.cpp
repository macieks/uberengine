#include "Base/Base.h"
#include "Threading/SyncEvent.h"

//#define UE_EVENT_IMPL_COND
//#define UE_EVENT_IMPL_SEM
#define UE_EVENT_IMPL_MUTEX

#if defined(UE_EVENT_IMPL_COND)

ueSyncEvent::ueSyncEvent(ueBool isManualReset)
{
    ueAssert(!isManualReset);
    pthread_cond_init(&m_cond, NULL);
    pthread_mutex_init(&m_mutex, NULL);
}

ueSyncEvent::~ueSyncEvent()
{
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
}

void ueSyncEvent::Reset()
{
//	ueAssert(!"Reset not available on linux.");
}

ueBool ueSyncEvent::Wait(u32 milliseconds)
{
    if (milliseconds == 0)
        pthread_cond_wait(&m_cond, &m_mutex);
    else
    {
        timespec t;
        t.tv_nsec = (milliseconds % 1000) * 1000000;
        t.tv_sec = milliseconds / 1000;
        pthread_cond_timedwait(&m_cond, &m_mutex, &t);
    }
    return UE_TRUE;
}

void ueSyncEvent::Signal()
{
	pthread_cond_signal(&m_cond);
}

#elif defined(UE_EVENT_IMPL_SEM)

ueSyncEvent::ueSyncEvent(ueBool isManualReset)
{
    ueAssert(!isManualReset);
    int ret = sem_init(&m_sem,
             0,     // not shared
             0);    // initially set to non signaled state
    ueAssert(ret == 0);
}

ueSyncEvent::~ueSyncEvent()
{
	int ret = sem_destroy(&m_sem);
	ueAssert(ret == 0);
}

void ueSyncEvent::Reset()
{
//	ueAssert(!"Reset not available on linux.");
}

ueBool ueSyncEvent::Wait(u32 milliseconds)
{
    ueAssert(milliseconds == 0);
    int ret = sem_wait(&m_sem);
    ueAssert(ret == 0);
    return UE_TRUE;
}

void ueSyncEvent::Signal()
{
	int ret = sem_post(&m_sem);
	ueAssert(ret == 0);
}

#elif defined(UE_EVENT_IMPL_MUTEX)

ueSyncEvent::ueSyncEvent(ueBool isManualReset)
{
    ueAssert(!isManualReset);

    int ret;
    ret = pthread_mutex_init(&m_mutex, NULL);
    ueAssert(!ret);
    ret = pthread_mutex_lock(&m_mutex);
    ueAssert(!ret);
}

ueSyncEvent::~ueSyncEvent()
{
	pthread_mutex_destroy(&m_mutex);
}

void ueSyncEvent::Reset()
{
    const int ret = pthread_mutex_unlock(&m_mutex);
    ueAssert(!ret);
}

ueBool ueSyncEvent::Wait(u32 milliseconds)
{
    ueAssert(milliseconds == 0);
    const int ret = pthread_mutex_lock(&m_mutex);
    ueAssert(!ret);
    return UE_TRUE;
}

void ueSyncEvent::Signal()
{
	const int ret = pthread_mutex_unlock(&m_mutex);
	ueAssert(!ret || ret == EPERM);
}

#endif

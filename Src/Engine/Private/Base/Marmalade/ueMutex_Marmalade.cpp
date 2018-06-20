#include "Base/ueBase.h"
#include "Threading/thMutex.h"

// FIXME: For now we don't need mutexes for our Marmalade based games
#define TH_MUTEX_STUB

thMutex::thMutex() :
	m_handle(NULL),
	m_debugCounter(0)
{
#ifndef TH_MUTEX_STUB
	m_handle = s3eThreadLockCreate();
	ueAssert(m_handle);
#endif
}

thMutex::~thMutex()
{
#ifndef TH_MUTEX_STUB
	s3eThreadLockDestroy((s3eThreadLock*) m_handle);
#endif
}

void thMutex::Lock()
{
#ifndef TH_MUTEX_STUB
	ueAssert(m_debugCounter == 0);
	s3eThreadLockAcquire((s3eThreadLock*) m_handle);
	m_debugCounter++;
#endif
}

void thMutex::Unlock()
{
#ifndef TH_MUTEX_STUB
	m_debugCounter--;
	s3eThreadLockRelease((s3eThreadLock*) m_handle);
#endif
}
#include "Base/ueBase.h"
#include "Threading/thEvent.h"

thEvent::thEvent(ueBool isManualReset) :
	m_lock(NULL)
{
	ueAssert(!isManualReset);

	m_lock = s3eThreadLockCreate();
	ueAssert(m_lock);
}

thEvent::~thEvent()
{
	s3eThreadLockDestroy((s3eThreadLock*) m_lock);
}

void thEvent::Reset()
{
	s3eThreadLockRelease((s3eThreadLock*) m_lock);
}

ueBool thEvent::Wait(u32 milliseconds)
{
	ueAssert(!milliseconds);
	const s3eResult result = s3eThreadLockAcquire((s3eThreadLock*) m_lock);
	return result == S3E_RESULT_SUCCESS;
}

void thEvent::Signal()
{
	s3eThreadLockRelease((s3eThreadLock*) m_lock);
}
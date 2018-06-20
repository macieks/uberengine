#ifndef UE_THREADING_H
#define UE_THREADING_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ue
 *	@{
 */

#if defined(UE_WIN32)
	//! Thread id uniquely identifying thread
	typedef HANDLE ueThreadId;
#else
	//! Thread id uniquely identifying thread
	typedef u32 ueThreadId;
#endif

//! Thread function
typedef void (*ueThreadFunc)(void* userData);

//! Thread creation parameters
struct ueThreadParams
{
	ueThreadFunc m_func;			//!< Main thread function
	f32 m_priority;					//!< Thread priority within 0..1 range; 0 - lowest; 1 - highest
	u32 m_stackSize;                //!< Stack size; 0 indicates default
	u32 m_processor;				//!< Processor id or mask
	const char* m_name;				//!< User friendly thread name
	void* m_userData;

	ueThreadParams() :
		m_func(NULL),
		m_priority(0.5f),
		m_stackSize(0),
		m_processor(0),
		m_name(NULL),
		m_userData(NULL)
	{}
};

//! Creates thread; returns 0 on failure
ueThreadId	ueThread_Create(ueThreadParams* params);
//! Suspends execution of the calling thread until the target thread terminates
void		ueThread_Join(ueThreadId id);
//! Starts thread
ueBool		ueThread_Start(ueThreadId id);
//! Sets thread priority
void		ueThread_SetPriority(ueThreadId id, f32 priority);
//! Exits current thread with given exit code
void		ueThread_Exit(s32 exitCode);
//! Puts current thread into sleep for given number of millisecs
void		ueThread_Sleep(u32 milliSecs);
//! Yields current thread
void		ueThread_Yield();
//! Gets current thread id
ueThreadId	ueThread_GetCurrent();
//! Tells whether current thread is the main thread
ueBool		ueThread_IsMain();
//! Gets processor count on system
u32			ueThread_GetNumProcessors();

//! OS mutex
struct ueMutex;

//! Creates mutex
ueMutex* ueMutex_Create();
//! Destroys mutex
void ueMutex_Destroy(ueMutex* m);
//! Locks mutex blocking until succeeded
void ueMutex_Lock(ueMutex* m);
//! Unlocks mutex
void ueMutex_Unlock(ueMutex* m);

//! Scoped mutex lock; unlocks mutex on destruction
class ueMutexLock
{
public:
	//! Initializes mutez lock; if lock is set (default), then mutex is locked at this point
	UE_INLINE ueMutexLock(ueMutex* mutex, ueBool lock = UE_TRUE) :
		m_mutex(mutex),
		m_isLocked(lock)
	{
		if (lock)
			ueMutex_Lock(m_mutex);
	}

	//! Unlocks mutex if locked
	UE_INLINE ~ueMutexLock()
	{
		if (m_isLocked)
			ueMutex_Unlock(m_mutex);
	}

	//! Unlocks the mutex
	UE_INLINE void Unlock()
	{
		UE_ASSERT(m_isLocked);
		ueMutex_Unlock(m_mutex);
		m_isLocked = UE_FALSE;
	}

private:
	ueMutex* m_mutex; //!< Handled mutex
	ueBool m_isLocked; //!< Indicates whether mutex is locked
};

//! OS synchronization event
struct ueEvent;

/**
 *	Creates synchronization event.
 *
 *	An event is in non-signaled state at startup.
 *
 *	@param isManualReset indicates whether the event is of manual reset type; if so, it has to be manually
 *		reset to non-signaled state when desired; for non-manual reset events this is being done
 *		automatically when Wait() method leaves
 */
ueEvent* ueEvent_Create(ueBool isManualReset = UE_FALSE);
//! Destroys an event
void ueEvent_Destroy(ueEvent* e);
//!	Resets event to non-signaled state.
void ueEvent_Reset(ueEvent* e);
/**
 *	Waits given time for another thread to be signaled or if the event is in signaled state it just
 *	passes through putting itself in non-signaled state if non-manual event (otherwise it's signaled
 *	afterwards as well).
 */
ueBool ueEvent_Wait(ueEvent* e, u32 milliseconds = 0);
//!	Puts an event into signaled state causing a thread being waiting on Wait() method to unlock.
void ueEvent_Signal(ueEvent* e);

//! Initializes threading API
void ueThreading_Startup(ueAllocator* allocator, u32 maxThreads, u32 maxMutexes, u32 maxEvents);
//! Shuts down threading API
void ueThreading_Shutdown();

// @}

#endif // UE_THREADING_H
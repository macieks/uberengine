#include "Base/ueAsync_Private.h"
#include "Base/ueThreading.h"

ueAsyncState ueAsync_GetState(ueAsync* async)
{
	if (async->m_state != ueAsyncState_InProgress || !async->m_pollFunc)
		return async->m_state;
	async->m_pollFunc(async);
	return async->m_state;
}

ueAsyncState ueAsync_Wait(ueAsync* async, u32 millisecs)
{
	if (async->m_state != ueAsyncState_InProgress)
		return async->m_state;

	if (async->m_waitFunc)
		async->m_waitFunc(async, millisecs);
	else
	{
		const f32 millisecsF = (f32) millisecs;
		const ueTime startTime = ueClock_GetCurrent();
		while (ueClock_GetMillisecsSince(startTime) < millisecsF)
		{
			if (async->m_pollFunc)
				async->m_pollFunc(async);
			ueThread_Yield();
		}
	}

	return async->m_state;
}

void* ueAsync_GetUserData(ueAsync* async)
{
	return async->m_userData;
}

void* ueAsync_GetData(ueAsync* async)
{
	UE_ASSERT(async->m_getDataFunc);
	UE_ASSERT(async->m_state != ueAsyncState_InProgress);
	return async->m_getDataFunc(async);
}

void ueAsync_Destroy(ueAsync* async, ueBool block)
{
	UE_ASSERT(async->m_destroyFunc);
	async->m_destroyFunc(async, block);
}

void ueAsync_SetDone(ueAsync* async, ueBool success)
{
	UE_ASSERT(async->m_state == ueAsyncState_InProgress);
	async->m_state = success ? ueAsyncState_Succeeded : ueAsyncState_Failed;
}
#ifndef UE_ASYNC_PRIVATE_H
#define UE_ASYNC_PRIVATE_H

#include "Base/ueAsync.h"

typedef void (*ueAsync_PollFunc)(ueAsync* async);
typedef void (*ueAsync_WaitFunc)(ueAsync* async, u32 millisecs);
typedef void* (*ueAsync_GetDataFunc)(ueAsync* async);
typedef void (*ueAsync_DestroyFunc)(ueAsync* async, ueBool block);

struct ueAsync
{
	volatile ueAsyncState m_state;

	ueAsync_PollFunc m_pollFunc; // Optional
	ueAsync_WaitFunc m_waitFunc; // Optional
	ueAsync_GetDataFunc m_getDataFunc; // Optional
	ueAsync_DestroyFunc m_destroyFunc;

	void* m_userData;

	ueAsync() :
		m_state(ueAsyncState_InProgress),
		m_pollFunc(NULL),
		m_waitFunc(NULL),
		m_getDataFunc(NULL),
		m_destroyFunc(NULL),
		m_userData(NULL)
	{}
};

#endif // UE_ASYNC_PRIVATE_H

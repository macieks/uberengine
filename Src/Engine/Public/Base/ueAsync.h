#ifndef UE_ASYNC_H
#define UE_ASYNC_H

#include "Base/ueBase.h"

/**
 *	@struct ueAsync
 *	@brief Asynchronous operation handle
 */
struct ueAsync;

//! Asynchronous operation states
enum ueAsyncState
{
	ueAsyncState_InProgress = 0,	//!< Still going
	ueAsyncState_Succeeded,			//!< Finished successfully
	ueAsyncState_Failed,			//!< Finished but failed

	ueAsyncState_MAX
};

//! Gets asynchronous operation state
ueAsyncState ueAsync_GetState(ueAsync* async);
//! Waits specific number of milliseconds for an asynchronous operation to finish; 0 millisecs indicates to wait until done
ueAsyncState ueAsync_Wait(ueAsync* async, u32 millisecs = 0);
//! Gets user data associated with the asynchronous operation
void* ueAsync_GetUserData(ueAsync* async);
//! Gets asynchronous operation specific data; see specific operation's documentation for details
void* ueAsync_GetData(ueAsync* async);
//! Destroys asynchronous operation; block paramater indicates whether to wait for a complete operation clean up (when set to UE_TRUE) or not (when set to UE_FALSE)
void ueAsync_Destroy(ueAsync* async, ueBool block = UE_TRUE);

//! Sets asynchronous operation to "done" state (either "success" or "failure")
void ueAsync_SetDone(ueAsync* async, ueBool success = UE_TRUE);

#endif // UE_ASYNC_H

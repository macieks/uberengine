#ifndef UE_RESOURCE_H
#define UE_RESOURCE_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueBase.h"

struct ueResource;

//! Possible resource states
enum ueResourceState
{
	ueResourceState_Initial = 0,		//!< Created, loading not started
	ueResourceState_WaitingForSyncInit,	//!< Initializing: waiting for synchronous (main thread) initialization stage (e.g. copying GPU texture data)
	ueResourceState_WaitingForAsyncInit,//!< Initializing: waiting for asynchronous initialization stage (e.g. loading data from HDD)
	ueResourceState_Initializing,		//!< Initializing: in progress (either sync or async)
	ueResourceState_Ready,				//!< Successfully initialized and ready to use
	ueResourceState_FailedToInitialize,	//!< Failed to initialize

	ueResourceState_MAX
};

//! Indicates whether resource is ready to use
ueBool				ueResource_IsReady(const ueResource* resource);
//! Gets resource state
ueResourceState		ueResource_GetState(const ueResource* resource);
//! Waits for the resource to finish initialization; returns UE_TRUE if initialization succeeds, UE_FALSE otherwise
ueBool				ueResource_Wait(ueResource* resource);
//! Gets resource name
const char*			ueResource_GetName(const ueResource* resource);
//! Gets resource type id
u32					ueResource_GetTypeId(const ueResource* resource);
//! Gets resource type name
const char*			ueResource_GetTypeName(const ueResource* resource);
//! Increases reference count for a resource
void				ueResource_IncRefCount(ueResource* r);

// Resource manager

//! Resource enumeration callback
typedef void (*ueResourceEnumCallback)(ueResource* resource, void* userData);

//! Starts up resource manager
void		ueResourceMgr_Startup(ueAllocator* allocator, u32 maxResourceTypes = 64, u32 maxResources = 65536);
//! Shuts down resource manager
void		ueResourceMgr_Shutdown();
//! Performs synchronous per-frame work: loading / unloading of resources
void		ueResourceMgr_DoWork(f32 maxWorkTimeSecs);
//! Waits for all resources to be loaded
ueBool		ueResourceMgr_WaitAll();
//! Enumerates all resources of specific type
void		ueResourceMgr_Enumerate(u32 typeId, ueResourceEnumCallback callback, void* userData);
//! Gets requested resource; if not loaded, loads it synchronously or asynchronously (depending on parameters)
ueResource*	ueResourceMgr_Get(u32 typeId, const char* name, ueBool initImmediately = UE_FALSE, ueBool loadIfNotPresent = UE_TRUE);
//! Releases given resource
void		ueResourceMgr_Release(ueResource* resource);
//! Adds resource created manually
void		ueResourceMgr_Add(ueResource* resource);
//! Gets max number of resource types
u32			ueResourceMgr_GetMaxResourceTypes();

template <typename TYPE> struct ueResourceType {};
//! Defines resource type
#define UE_DEFINE_RESOURCE_TYPE(resourceType, resourceTypeID) \
	struct resourceType; \
	template <> struct ueResourceType<resourceType> { static const u32 ID = resourceTypeID; }

//! Resource handle
template <class TYPE>
class ueResourceHandle
{
public:
	static const u32 TYPE_ID = ueResourceType<TYPE>::ID;

	//! Initializes resource handle
	UE_INLINE ueResourceHandle();
	//! Deinitializes resource handle; releases handled resource
	UE_INLINE ~ueResourceHandle();

	//! Copy operator
	UE_INLINE void operator = (ueResourceHandle<TYPE>& other);
	//! Copy operator
	UE_INLINE void operator = (TYPE* other);
	//! Gets handled resource pointer
	UE_INLINE TYPE* operator * () const;
	//! Gets handled resource pointer
	UE_INLINE TYPE* operator -> () const;
	//! Gets handled resource as ueResource type
	UE_INLINE ueResource* AsResource() const;

	//! Gets whether handled resource is non-NULL
	UE_INLINE ueBool IsValid() const;
	//! Gets whether handled resource is ready for use
	UE_INLINE ueBool IsReady() const;
	//! Gets resource state
	UE_INLINE ueResourceState GetState() const;
	//! Waits until resource is fully loaded; returns UE_TRUE on success, UE_FALSE otherwise
	UE_INLINE ueBool Wait();

	//! Sets resource by name; for immediate init, returns success; for async init, returns UE_TRUE
	UE_INLINE ueBool SetByName(const char* name, ueBool initImmediately = UE_TRUE);
	//! Releases handled object (and sets handled resource to NULL)
	UE_INLINE void Release();

	//! Gets resource name (if valid)
	UE_INLINE const char* GetName() const;

private:
	TYPE* m_resource;
};

#include "Base/ueResourceHandle_Inline.h"

// @}

#endif // UE_RESOURCE_H

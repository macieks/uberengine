#include "Base/ueResource_Private.h"
#include "Base/Containers/ueHashMap.h"
#include "Base/ueThreading.h"
#include "Base/ueWindow.h"

// ueResourceMgrData

typedef ueHashMap<const char*, ueResource*> ResourceMapType;

struct ueResourceMgrData
{
	ueAllocator* m_allocator;
	ueBool m_showMsgOnResourceLoadFailure;

	ueHashMap<u32, ueResourceTypeDesc*> m_types;

	ueMutex* m_mutex;
	ResourceMapType m_resources;

	ueMutex* m_initQueueMutex;
	ueList<ueResource> m_initQueue;

	ueMutex* m_deinitQueueMutex;
	ueList<ueResource> m_deinitQueue;

	ueResourceMgrData() :
		m_allocator(NULL),
		m_showMsgOnResourceLoadFailure(UE_TRUE)
	{}
};

static ueResourceMgrData* s_data = NULL;

// ueResource

ueResource::ueResource() :
	m_name(NULL),
	m_type(0),
	m_refCount(0),
	m_state(ueResourceState_Initial),
	m_readyEvent(NULL)
{
	m_readyEvent = ueEvent_Create();
}

ueResource::~ueResource()
{
	ueEvent_Destroy(m_readyEvent);
	UE_ASSERT(m_refCount == 0);
}

void ueResource_IncRefCount(ueResource* r)
{
	ueMutexLock lock(s_data->m_mutex);
	r->m_refCount++;
}

ueBool ueResource_IsReady(const ueResource* r)
{
	return r->m_state == ueResourceState_Ready;
}

ueResourceState ueResource_GetState(const ueResource* r)
{
	return r->m_state;
}

ueBool ueResource_Wait(ueResource* r)
{
	ueEvent_Wait(r->m_readyEvent);
	return ueResource_IsReady(r);
}

const char* ueResource_GetName(const ueResource* r)
{
	return r->m_name;
}

u32 ueResource_GetTypeId(const ueResource* r)
{
	return r->m_type->m_id;
}

const char* ueResource_GetTypeName(const ueResource* r)
{
	return r->m_type->m_name;
}

// Resource Manager

void ueResourceMgr_Startup(ueAllocator* allocator, u32 maxResourceTypes, u32 maxResources)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) ueResourceMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;
	UE_ASSERT_FUNC(s_data->m_resources.Init(allocator, maxResources));
	UE_ASSERT_FUNC(s_data->m_types.Init(allocator, maxResourceTypes));
	s_data->m_mutex = ueMutex_Create();
	s_data->m_initQueueMutex = ueMutex_Create();
	s_data->m_deinitQueueMutex = ueMutex_Create();
}

void ueResourceMgr_Shutdown()
{
	UE_ASSERT(s_data);

	UE_ASSERT(s_data->m_types.Size() == 0);
	s_data->m_types.Deinit();

#if defined(UE_ENABLE_ASSERTION) || 1
	if (s_data->m_resources.Size())
	{
		const u32 numDumpResources = ueMin(s_data->m_resources.Size(), (u32) 10);

		ueLogW("There are %u resources that have not been released (showing first %u):", s_data->m_resources.Size(), numDumpResources);
		ResourceMapType::Iterator iter(s_data->m_resources);
		u32 index = 0;
		while (index < numDumpResources && iter.Next())
		{
			ueResource* resource = *iter.Value();
			ueLogW("  [%u] %s (ref count = %u)", index++, resource->m_name, resource->m_refCount);
		}	
	
		UE_ASSERT(!"Unfreed resources.");
	}
#endif
	s_data->m_resources.Deinit();

	ueMutex_Destroy(s_data->m_mutex);
	ueMutex_Destroy(s_data->m_initQueueMutex);
	ueMutex_Destroy(s_data->m_deinitQueueMutex);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

u32 ueResourceMgr_GetMaxResourceTypes()
{
	UE_ASSERT(s_data);
	return s_data->m_types.Capacity();
}

void ueResourceMgr_RegisterType(ueResourceTypeDesc* typeDesc)
{
	UE_ASSERT(s_data);
	UE_ASSERT_MSGP(!ueResourceMgr_GetType(typeDesc->m_id), "Type with id = %u already registered", typeDesc->m_id);
	s_data->m_types.Insert(typeDesc->m_id, typeDesc);
}

void ueResourceMgr_UnregisterType(ueResourceTypeDesc* typeDesc)
{
	UE_ASSERT(s_data);
	UE_ASSERT_FUNC_MSG(s_data->m_types.Remove(typeDesc->m_id), "Type not found");
}

ueResourceTypeDesc* ueResourceMgr_GetType(u32 typeId)
{
	UE_ASSERT(s_data);
	ueResourceTypeDesc** td = s_data->m_types.Find(typeId);
	return td ? *td : NULL;
}

void ueResourceMgr_Add(ueResource* resource)
{
	ueMutexLock lock(s_data->m_mutex);
	UE_ASSERT(resource->m_state == ueResourceState_Ready);
	s_data->m_resources.Insert(resource->m_name, resource);
	ueLogI("Resource (%s) '%s' ready (added manually)", resource->m_type->m_name, resource->m_name);
}

void ueResourceMgr_Release(ueResource* resource)
{
	ueMutexLock lock(s_data->m_mutex);

	UE_ASSERT(resource->m_refCount > 0);
	if (--resource->m_refCount > 0)
		return;

	// Destroy now if we're sure resource is not being initialized now

	if (resource->m_state == ueResourceState_Ready ||
		resource->m_state == ueResourceState_FailedToInitialize)
		ueResourceMgr_DestroyInLock(resource, ueThread_IsMain());
}

void ueResourceMgr_DestroyInLock(ueResource* resource, ueBool isMainThread)
{
	UE_ASSERT(s_data);
	ueResourceTypeDesc* type = resource->m_type;

	// Schedule for destruction

	if (!isMainThread && type->m_needsSyncDeinit)
	{
		ueMutexLock deinitQueueLock(s_data->m_deinitQueueMutex);
		s_data->m_deinitQueue.PushFront(resource);
	}

	// Destroy immediately

	else
	{
		s_data->m_resources.Remove(resource->m_name);
		type->m_destroyFunc(resource);
	}
}

void ueResourceMgr_OnInitStageDone(ueResource* resource, ueResourceState newState, ueBool addInitQueue)
{
	ueMutexLock lock(s_data->m_mutex);

	if (newState == ueResourceState_FailedToInitialize)
	{
		ueLogE("Missing resource ", ueResource_GetName(resource));
		if (s_data->m_showMsgOnResourceLoadFailure)
			ueWindowUtils_ShowMessageBox(NULL, "Missing resource", ueResource_GetName(resource));
	}

	resource->m_state = newState;
	if (resource->m_refCount == 0)
		ueResourceMgr_DestroyInLock(resource, ueThread_IsMain());
	else if (addInitQueue)
		ueResourceMgr_AddInitQueue(resource);
}

ueResource* ueResourceMgr_Get(u32 typeId, const char* name, ueBool initImmediately, ueBool loadIfNotPresent)
{
	UE_ASSERT(s_data);
	UE_ASSERT_MSGP(ueStrIsLower(name), "Resource name must be lowercase (attempted to get resource '%s')", name);

	ueResource* resource = NULL;
	ueResourceTypeDesc* type = NULL;
	{
		ueMutexLock lock(s_data->m_mutex);

		// Find existing resource

		ueResource** resourcePtr = s_data->m_resources.Find(name);
		if (resourcePtr)
		{
			resource = *resourcePtr;
			resource->m_refCount++;
			UE_ASSERT(resource->m_type->m_id == typeId);
			return resource;
		}

		// Don't load?

		if (!loadIfNotPresent)
			return NULL;

		// Get type

		type = ueResourceMgr_GetType(typeId);
		UE_ASSERT_MSG(type, "Resource type wasn't registered");

		// Create resource

		resource = type->m_createFunc(name);
		UE_ASSERT(resource);
		resource->m_type = type;
		UE_ASSERT(!ueStrCmp(resource->m_name, name));

		resource->m_refCount = 1;
		s_data->m_resources.Insert(resource->m_name, resource);
	}

	UE_ASSERT(resource->m_state == ueResourceState_WaitingForAsyncInit || resource->m_state == ueResourceState_WaitingForSyncInit);

	// Synchronous initialization (assumes no other threads attempts to release this resource while initializing it)

	if (initImmediately)
	{
		// Perform as many as needed initialization steps

		while (1)
		{
			// Single initialization step

			type->m_initFunc(resource, UE_TRUE);
			while (resource->m_state == ueResourceState_Initializing)
				ueThread_Yield();

#ifdef UE_MARMALADE

			// Prevent Android devices from sleeping while loading assets

			if (s3eDeviceGetInt(S3E_DEVICE_OS) == S3E_OS_ID_ANDROID)
				s3eDeviceBacklightOn();
#endif

			// Check state

			switch (resource->m_state)
			{
				// Success!

				case ueResourceState_Ready:
					ueLogI("Resource (%s) '%s' ready", type->m_name, name);
					return resource;

				// Failed to initialize
				
				case ueResourceState_FailedToInitialize:
					ueLogE("Failed to initialize resource (%s) '%s'", type->m_name, name);
					return resource;

				// Initialization not yet finished
				
				case ueResourceState_WaitingForAsyncInit:
				case ueResourceState_WaitingForSyncInit:
					break;
			}
		}
	}

	// Asynchronous initialization

	else
	{
		// Trigger asynchronous initialization or add to init-queue

		if (resource->m_state == ueResourceState_WaitingForAsyncInit)
			type->m_initFunc(resource, UE_FALSE);
		else
			ueResourceMgr_AddInitQueue(resource);
	}

	return resource;
}

void ueResourceMgr_AddInitQueue(ueResource* resource)
{
	UE_ASSERT(s_data);
	ueMutexLock initQueueLock(s_data->m_initQueueMutex);
	s_data->m_initQueue.PushFront(resource);
}

void ueResourceMgr_RemoveInitQueue(ueResource* resource)
{
	UE_ASSERT(s_data);
	ueMutexLock initQueueLock(s_data->m_initQueueMutex);
	s_data->m_initQueue.Remove(resource);
}

void ueResourceMgr_DoWork(f32 maxWorkTimeSecs)
{
	UE_ASSERT(s_data);
	UE_ASSERT(ueThread_IsMain());

	UE_PROF_SCOPE("ueResourceMgr_DoWork");

	const ueTime startTime = ueClock_GetCurrent();

	ueResourceMgr_ProcessSyncDeinits();
	ueResourceMgr_ProcessInits(startTime, maxWorkTimeSecs);
}

void ueResourceMgr_ProcessSyncDeinits()
{
	// Copy deinit queue

	ueList<ueResource> deinitQueue;
	{
		ueMutexLock deinitQueueLock(s_data->m_deinitQueueMutex);
		deinitQueue.CopyFrom(s_data->m_deinitQueue);
	}

	// Perform all deinitializations

	while (ueResource* resource = deinitQueue.PopBack())
	{
		UE_ASSERT(resource->m_refCount == 0);
		ueResourceTypeDesc* type = resource->m_type;
		type->m_destroyFunc(resource);
	}
}

void ueResourceMgr_ProcessInits(ueTime startTime, f32 maxWorkTimeSecs)
{
	// Copy init queue

	ueList<ueResource> initQueue;
	{
		ueMutexLock initQueueLock(s_data->m_initQueueMutex);
		initQueue.CopyFrom(s_data->m_initQueue);
	}

	ueResource* resource = NULL;

	// Process all async inits

	resource = initQueue.Front();
	while (resource)
		if (resource->m_state == ueResourceState_WaitingForAsyncInit)
		{
			ueResource* next = resource->Next();
			initQueue.Remove(resource);
			resource->m_type->m_initFunc(resource, UE_FALSE);
			resource = next;
		}
		else
			resource = resource->Next();

	// Process as many as possible sync inits

	resource = initQueue.Back();
	while (resource)
	{
		UE_ASSERT(resource->m_state == ueResourceState_WaitingForSyncInit);

		ueResource* prev = resource->Previous();
		initQueue.Remove(resource);

		resource->m_type->m_initFunc(resource, UE_FALSE);

		// Check timeout

		if (ueClock_GetSecsSince(startTime) >= maxWorkTimeSecs)
			break;

		resource = prev;
	}

	// Copy back unprocessed resources
	{
		ueMutexLock initQueueLock(s_data->m_initQueueMutex);
		s_data->m_initQueue.Append(initQueue);
	}
}
#ifndef UE_RESOURCE_PRIVATE_H
#define UE_RESOURCE_PRIVATE_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueResource.h"
#include "Base/Containers/ueList.h"
#include "Base/ueThreading.h"

struct ueResourceTypeDesc;

/**
 *	@struct ueResource
 *	@brief Base engine resource
 */
struct ueResource : ueList<ueResource>::Node
{
	ueResource();
	~ueResource();

	char* m_name;				//!< Resource name
	u32 m_refCount;				//!< Reference count
	ueResourceTypeDesc* m_type;	//!< Resource type

	ueEvent* m_readyEvent;		//!< Event used to notify resource ready
	ueResourceState m_state;	//!< Resource state
};

//! Creates (but doesn't initialize) resource; must be quick to perform
typedef ueResource* (*ueResourceCreateFunc)(const char* name);
//! Initializes resource; isSyncInit inidicates whether it's called synchronously (i.e. from the calling thread)
typedef void (*ueResourceInitFunc)(ueResource* resource, ueBool isSyncInit);
//! Destroys resource; invoked synchronously or not depending on resource type configuration
typedef void (*ueResourceDestroyFunc)(ueResource* resource);

//! Resource type description
struct ueResourceTypeDesc
{
	u32 m_id;							//!< Unique resource id
	const char* m_name;					//!< Resource type name
	ueResourceCreateFunc m_createFunc;	//!< Resource create function
	ueResourceInitFunc m_initFunc;		//!< Resource initialization function
	ueResourceDestroyFunc m_destroyFunc;//!< Resource deinitialization function (synchronous or not)
	ueBool m_needsSyncDeinit;			//!< Indicates whether resource destruction must be done synchronously (i.e. from main thread)
	const char* m_config;				//!< Optional resource specific configuration name
	ueAssetPath m_assetPathType;		//!< Default asset loading path type wrt. localization

	ueResourceTypeDesc() :
		m_id(0),
		m_name(NULL),
		m_createFunc(NULL),
		m_initFunc(NULL),
		m_destroyFunc(NULL),
		m_needsSyncDeinit(UE_FALSE),
		m_config(NULL),
		m_assetPathType(ueAssetPath_LocalizedFirst)
	{}
};

//! Performs cast from ueResource to destination type; returns NULL if cast failed
template <typename TYPE>
UE_INLINE TYPE* ueResource_Cast(ueResource* resource, ueBool assertOnFailure = UE_TRUE)
{
	TYPE* dst = (TYPE::TYPE_ID == ueResource_GetTypeId(resource)) ? static_cast<TYPE*>(resource) : NULL;
	UE_ASSERT_MSG(!assertOnFailure || dst, "Resource cast failed");
	return dst;
}

void ueResourceMgr_RegisterType(ueResourceTypeDesc* typeDesc);
void ueResourceMgr_UnregisterType(ueResourceTypeDesc* typeDesc);
ueResourceTypeDesc* ueResourceMgr_GetType(u32 typeId);

void ueResourceMgr_AddInitQueue(ueResource* resource);
void ueResourceMgr_RemoveInitQueue(ueResource* resource);
void ueResourceMgr_OnInitStageDone(ueResource* resource, ueResourceState newState, ueBool addInitQueue);
void ueResourceMgr_ProcessSyncDeinits();
void ueResourceMgr_ProcessInits(ueTime startTime, f32 maxWorkTimeSecs);
void ueResourceMgr_DestroyInLock(ueResource* resource, ueBool isMainThread);

//! Constructs resource allocating memory for both the resource and its name
#define UE_NEW_RESOURCE(resourceType, resourceVariable, allocator, name) \
	resourceType* resourceVariable = NULL; \
	{ \
		const u32 nameBufferLength = ueStrLen(name) + 1; \
		const u32 size = sizeof(resourceType) + nameBufferLength; \
		void* memory = allocator->Alloc(size); \
		if (memory) \
		{ \
			resourceVariable = new(memory) resourceType(); \
			resourceVariable->m_name = (char*) (resourceVariable + 1); \
			ueMemCpy(resourceVariable->m_name, name, nameBufferLength); \
		} \
	}

#define UE_NEW_RESOURCE_RET(resourceType, resourceVariable, allocator, name) \
	UE_NEW_RESOURCE(resourceType, resourceVariable, allocator, name) \
	if (!resourceVariable) \
		return NULL;

// @}

#endif // UE_RESOURCE_PRIVATE_H
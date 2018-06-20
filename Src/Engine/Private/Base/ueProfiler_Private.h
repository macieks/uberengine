#ifndef UE_PROFILER_PRIVATE_H
#define UE_PROFILER_PRIVATE_H

#include "Base/Containers/ueHashMap.h"
#include "Base/ueAtomic.h"
#include "Base/ueThreading.h"

struct ueProfilerOperation
{
	enum Type
	{
		Type_Push = 0,
		Type_PushDynamic,
		Type_Pop,

		Type_MAX
	};

	ueTime m_ticks;
	ueThreadId m_threadId;
	u8 m_type;
};

struct ueProfilerOperation_Push : ueProfilerOperation
{
	u16 m_id; //!< Profiled scope id
};

struct ueProfilerOperation_PushDynamic : ueProfilerOperation_Push
{
	u16 m_nameSize; //!< Name length including terminal

	//! Gets profiled scope name
	UE_INLINE const char* GetName() const { return (char*) (this + 1); }
	UE_INLINE u32 GetSize() const { return sizeof(ueProfilerOperation_PushDynamic) + m_nameSize; }
};

struct ueProfilerOperation_Pop : ueProfilerOperation
{
};

struct ueProfilerMgrData
{
	struct Buffer
	{
		u8* m_data;
		u32 m_capacity;
		volatile u32 m_size;

		// Lock free allocator
		UE_INLINE u8* Alloc(u32 numBytes)
		{
			while (1)
			{
				const u32 size = m_size;
				const u32 newSize = size + numBytes;
				if (newSize >= m_capacity) return NULL;

				if (ueAtomic_CAS(&m_size, size, newSize))
					return m_data + size;
			}
			return NULL;
		}
	};

	struct Scope
	{
		u16 m_id;
		u32 m_flags;
	};

	ueAllocator* m_allocator;

	Buffer m_buffer;				//!< Push-pop buffer
	ueProfilerData m_lastFrame;		//!< Copy of the push-pop buffer

	ueProfilerCapture* m_capture;	//!< Optional capture
	u32 m_captureSize;
	ueBool m_captureUpToDate;

	ueMutex* m_scopesMutex;
	u32 m_numScopes;
	ueHashMap<const char*, Scope> m_scopes;
	ueHashMap<u32, const char*> m_scopesById;
};

extern ueProfilerMgrData* g_profilerMgrData;

#endif // UE_PROFILER_PRIVATE_H

#include "Base/Allocators/ueMemoryDebugger.h"
#include "Base/ueCallstack.h"
#include "Base/ueThreading.h"
#include "Base/Containers/ueHashMap.h"

#if defined(UE_ENABLE_MEMORY_DEBUGGING)

struct ueMemoryDebugger
{
	//! Base allocation information
	struct AllocInfo
	{
		ueSize m_size;
		ueAllocator* m_allocator;
	};

	//! Allocation info with callstack
	struct AllocInfoWithCallstack : AllocInfo
	{
		void* m_callstack[1]; // The actual callstack size is up to whatever callstackDepth was passed to Init* function
	};

	enum State
	{
		State_Idle = 0,
		State_Initialized,
		State_Running
	};
	State m_state;
	ueBool m_runOutOfMemory;

	ueAllocator* m_allocator;
	void* m_memory;

typedef ueHashMap<void*, AllocInfo*> AllocMapType;
	AllocMapType m_map;
	ueGenericPool m_pool;

	u32 m_callstackDepth;

	u32 m_numAllocatorsToTrack;
	ueAllocator* m_allocatorsToTrack[16];

	ueMutex* m_mutex;

	ueMemoryDebugger() :
		m_state(State_Idle),
		m_runOutOfMemory(UE_FALSE),
		m_memory(NULL),
		m_numAllocatorsToTrack(0),
		m_mutex(NULL)
	{}
};

void ueMemoryDebugger_OnReallocCallback(ueAllocator* allocator, void* memory, ueSize size, ueSize alignment, void* resultMemory, void* userData);

void ueMemoryDebugger_AddAllocatorToTrack(ueMemoryDebugger* md, ueAllocator* allocator)
{
	UE_ASSERT(md->m_state >= ueMemoryDebugger::State_Initialized);
	UE_ASSERT(md->m_numAllocatorsToTrack < UE_ARRAY_SIZE(md->m_allocatorsToTrack));
	allocator->AddListener(ueMemoryDebugger_OnReallocCallback, md);
	md->m_allocatorsToTrack[md->m_numAllocatorsToTrack++] = allocator;
}

void ueMemoryDebugger_RemoveAllocatorToTrack(ueMemoryDebugger* md, ueAllocator* allocator)
{
	UE_ASSERT(md->m_state >= ueMemoryDebugger::State_Initialized);
	for (u32 i = 0; i < md->m_numAllocatorsToTrack; i++)
		if (md->m_allocatorsToTrack[i] == allocator)
		{
			allocator->RemoveListener(ueMemoryDebugger_OnReallocCallback, md);
			md->m_allocatorsToTrack[i] = md->m_allocatorsToTrack[--md->m_numAllocatorsToTrack];
			return;
		}
	UE_ASSERT(0);
}

ueMemoryDebugger* ueMemoryDebugger_Create(ueAllocator* allocator, u32 maxTrackedAllocs, u32 callstackDepth)
{
	// Calculate required memory size

	const ueSize allocInfoSize = sizeof(ueMemoryDebugger::AllocInfo) + callstackDepth * sizeof(void*);
	const ueSize poolSize = ueGenericPool::CalcMemReq(allocInfoSize, maxTrackedAllocs);
	const ueSize mapSize = ueMemoryDebugger::AllocMapType::CalcMemReq(maxTrackedAllocs);
	const ueSize reqMemorySize = ueAlignPow2((ueSize) sizeof(ueMemoryDebugger), (ueSize) UE_DEFAULT_ALIGNMENT) + poolSize + mapSize;

	// Allocate memory

	u8* memory = (u8*) allocator->Alloc(reqMemorySize);
	UE_ASSERT(memory);

	// Initialize memory debugger

	u8* currMemory = memory;

	ueMemoryDebugger* md = new(currMemory) ueMemoryDebugger();
	currMemory += sizeof(ueMemoryDebugger);
	currMemory = (u8*) ueAlignPow2(currMemory, UE_DEFAULT_ALIGNMENT);
	md->m_allocator = allocator;
	md->m_state = ueMemoryDebugger::State_Initialized;
	md->m_callstackDepth = callstackDepth;
	md->m_memory = memory;
	md->m_mutex = ueMutex_Create();
	md->m_pool.InitMem(currMemory, poolSize, allocInfoSize, maxTrackedAllocs);
	currMemory += poolSize;
	md->m_map.InitMem(currMemory, mapSize, maxTrackedAllocs);
	currMemory += mapSize;

	return md;
}

void ueMemoryDebugger_Destroy(ueMemoryDebugger* md)
{
	UE_ASSERT(md->m_state == ueMemoryDebugger::State_Initialized);
	UE_ASSERT(md->m_numAllocatorsToTrack == 0);

	ueMutex_Destroy(md->m_mutex);
	ueDelete(md, md->m_allocator);
}

void ueMemoryDebugger_Begin(ueMemoryDebugger* md)
{
	UE_ASSERT(md->m_state == ueMemoryDebugger::State_Initialized);
	md->m_state = ueMemoryDebugger::State_Running;
	md->m_runOutOfMemory = UE_FALSE;

	md->m_map.Clear();
	md->m_pool.Clear();
}

void ueMemoryDebugger_End(ueMemoryDebugger* md)
{
	UE_ASSERT(md->m_state == ueMemoryDebugger::State_Running);
	md->m_state = ueMemoryDebugger::State_Initialized;
}

void ueMemoryDebugger_OnReallocCallback(ueAllocator* allocator, void* memory, ueSize size, ueSize alignment, void* result, void* userData)
{
	UE_ASSERT(allocator);

	ueMemoryDebugger* md = (ueMemoryDebugger*) userData;

	ueMutexLock lock(md->m_mutex);
	UE_ASSERT(md->m_state == ueMemoryDebugger::State_Running);

	// Allocate

	if (!memory && size)
	{
		// Allocation failed?

		if (!result)
			return;

		// Not enough memory in memory debugger?

		if (md->m_runOutOfMemory)
			return;

		// Create new entry

		ueMemoryDebugger::AllocInfo* info = (ueMemoryDebugger::AllocInfo*) md->m_pool.Alloc();
		if (!info)
		{
			md->m_runOutOfMemory = UE_TRUE;
			ueLogW("Run out of debug memory - memory tracking results will be invalid.");
			return;
		}

		info->m_size = size;
		info->m_allocator = allocator;

		if (md->m_callstackDepth > 0)
		{
			ueMemoryDebugger::AllocInfoWithCallstack* infoCS = (ueMemoryDebugger::AllocInfoWithCallstack*) info;
			u32 callstackSize = md->m_callstackDepth;
			ueCallstackHelper_GetCallstack(infoCS->m_callstack, callstackSize);
			if (callstackSize < md->m_callstackDepth)
				infoCS->m_callstack[callstackSize] = 0;
		}

		UE_ASSERT_FUNC_MSG(md->m_map.Insert(result, info), "Allocation at 0x%x already existed", result);
	}

	// Free

	else if (!size)
	{
		ueMemoryDebugger::AllocInfo* info = NULL;
		if (!md->m_map.Remove(memory, &info))
		{
			UE_ASSERT_MSGP(md->m_runOutOfMemory, "Freeing non-tracked memory at: 0x%x allocator: %s", memory, allocator->GetName());
			return;
		}
		UE_ASSERT(allocator == info->m_allocator);
		md->m_pool.Free(info);
	}

	// Reallocate

	else
	{
		if (!result) // Reallocate failed
			return;

		// Remove old entry

		ueMemoryDebugger::AllocInfo* info = NULL;
		if (!md->m_map.Remove(memory, &info))
		{
			UE_ASSERT_MSGP(md->m_runOutOfMemory, "Reallocating non-tracked memory at: 0x%x allocator: %s", memory, allocator->GetName());
			return;
		}
		UE_ASSERT(allocator == info->m_allocator);

		// Update size

		info->m_size = size;

		// Recapture callstack

		if (md->m_callstackDepth > 0)
		{
			ueMemoryDebugger::AllocInfoWithCallstack* infoCS = (ueMemoryDebugger::AllocInfoWithCallstack*) info;
			u32 callstackSize = md->m_callstackDepth;
			ueCallstackHelper_GetCallstack(infoCS->m_callstack, callstackSize);
			if (callstackSize < md->m_callstackDepth)
				infoCS->m_callstack[callstackSize] = 0;
		}

		// Insert new entry

		UE_ASSERT_FUNC_MSG(md->m_map.Insert(result, info), "Allocation at 0x%x already existed", result);
	}
}

u32 ueMemoryDebugger_GetNumUnfreedAllocations(ueMemoryDebugger* md, ueAllocator* allocator, ueBool mutexLocked)
{
	ueMutexLock lock(md->m_mutex, mutexLocked);

	if (!allocator)
		return md->m_map.Size();

	u32 numUnfreedAllocs = 0;
	ueHashMap<void*, ueMemoryDebugger::AllocInfo*>::Iterator iter(md->m_map);
	while (iter.Next())
	{
		ueMemoryDebugger::AllocInfo* info = *iter.Value();
		if (info->m_allocator == allocator)
			numUnfreedAllocs++;
	}

	return numUnfreedAllocs;
}

void ueMemoryDebugger_DumpUnfreedAllocations(ueMemoryDebugger* md, ueAllocator* allocator, u32 maxItems)
{
	ueMutexLock lock(md->m_mutex);
	UE_ASSERT(md->m_state == ueMemoryDebugger::State_Running);

	const u32 numUnfreedAllocs = ueMemoryDebugger_GetNumUnfreedAllocations(md, allocator, UE_FALSE);
	const char* allocatorName = allocator ? allocator->GetName() : "<all allocators>";

	if (md->m_runOutOfMemory)
		ueLogW("The debug info may be invalid (memory debugger run out of memory)");

	if (numUnfreedAllocs == 0)
		ueLogD("There is no unfreed allocations (allocator: %s)", allocatorName);
	else
	{
		ueLogW("Unfreed allocations for allocator %s [%u]:", allocatorName, numUnfreedAllocs);

		const u32 numItems = ueMin(numUnfreedAllocs, maxItems);
		ueHashMap<void*, ueMemoryDebugger::AllocInfo*>::Iterator iter(md->m_map);
		u32 i = 0;
		while (iter.Next() && i < numItems)
		{
			ueMemoryDebugger::AllocInfo* info = *iter.Value();

			if (allocator && info->m_allocator != allocator)
				continue;
			i++;

			ueLogW(" %u bytes (allocator: %s)", (u32) info->m_size, info->m_allocator->GetName());

			if (md->m_callstackDepth > 0)
			{
				ueMemoryDebugger::AllocInfoWithCallstack* infoCS = (ueMemoryDebugger::AllocInfoWithCallstack*) info;

				u32 callstackSize = 0;
				while (callstackSize < md->m_callstackDepth && infoCS->m_callstack[callstackSize])
					callstackSize++;

				ueCallstackHelper_DumpCallstack(infoCS->m_callstack, callstackSize);
			}
		}
	}
}

#endif // defined(UE_ENABLE_MEMORY_DEBUGGING)
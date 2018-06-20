#include "Base/Allocators/ueStackAllocator.h"

void ueStackAllocator::InitMem(void* memory, ueSize memorySize, u32 maxAllocs, void* debugMemory, ueSize debugMemorySize)
{
	UE_ASSERT(!maxAllocs || CalcDebugMemReq(maxAllocs) == debugMemorySize);

	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal | ueAllocatorStatsFlags_HasUsed | ueAllocatorStatsFlags_HasUsedPeak;
	m_stats.m_total = memorySize;

	m_reallocFunc = _ReallocFunc;
	m_memory = memory;
	m_capacity = memorySize;
	m_size = 0;

	m_maxAllocs = maxAllocs;
	m_offsets = (u32*) debugMemory;
	m_numAllocs = 0;
}

void* ueStackAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	// Freeing

	if (memory && !size)
	{
		UE_ASSERT_MSGP(m_memory <= memory, "Attempting to free invalid address (0x%x)", memory);
		UE_ASSERT_MSGP(memory < (u8*) m_memory + m_capacity, "Attempting to free invalid address (0x%x)", memory);

#if defined(UE_ENABLE_MEMORY_DEBUGGING) // Tell the user exactly which deallocation should happen now
		UE_ASSERT(m_numAllocs > 0);
		if (m_offsets && m_numAllocs >= 2) // We don't keep track of first allocation's start :(
		{
			const ueSize expectedEnd = m_offsets[m_numAllocs - 1];
			const ueSize expectedStart = (m_numAllocs >= 2) ? m_offsets[m_numAllocs - 2] : 0;
			const void* expectedPtr = (u8*) m_memory + expectedStart;
			UE_ASSERT_MSGP(expectedPtr == memory, "Unexpected memory freeing (got 0x%08x; expected = 0x%08x, %u bytes; wrong by %d bytes). Make sure to always free in exact reverse order of allocating.", memory, expectedPtr, (u32) (expectedEnd - expectedStart), (s32) ((ueSize) expectedPtr - (ueSize) memory));

			m_numAllocs--;

			// TODO: Would be nice to get access to memory debugger and show expected alloc's callstack (which is an abstract listener from ueAllocator point of view)
		}
#endif
		UE_ASSERT_MSGP(memory < (u8*) m_memory + m_size, "Unexpected memory freeing (too early?; address = 0x%x). Make sure to always free in exact reverse order of allocating.", memory);

		const ueSize newSize = (ueSize) ((ueSize) memory - (ueSize) m_memory);
		const ueSize totalAllocSize = m_size - newSize;
		m_stats.RemoveAlloc(totalAllocSize);
		m_size = newSize;
		return NULL;
	}

	// Allocation

	if (!memory && size)
	{
		const ueSize alignedOffset = ueAlignPow2((ueSize) m_memory + m_size, alignment);
		const ueSize alignedSize = ueAlignPow2(size, alignment);
		const ueSize newSize = alignedOffset - (ueSize) m_memory + alignedSize;
		if (newSize > m_capacity)
			return NULL;
		const ueSize totalAllocSize = newSize - m_size;
		m_stats.AddAlloc(totalAllocSize);
		m_size = newSize;

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
		if (m_offsets)
		{
			UE_ASSERT(m_numAllocs < m_maxAllocs);
			m_offsets[m_numAllocs++] = m_size;
		}
#endif
		return (void*) alignedOffset;
	}

	// Reallocation not allowed

	UE_ASSERT(!"Reallocation not allowed for stack allocator");
	return NULL;
}
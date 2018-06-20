#include "Base/Allocators/ueSingleBlockAllocator.h"

ueSingleBlockAllocator::ueSingleBlockAllocator() :
	m_memory(NULL),
	m_memorySize(0),
	m_isAllocated(UE_FALSE)
{}

void ueSingleBlockAllocator::InitMem(void* memory, ueSize memorySize)
{
	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal;
	m_stats.m_total = memorySize;

	m_reallocFunc = _ReallocFunc;
	m_memory = memory;
	m_memorySize = m_memorySize;
	m_isAllocated = UE_FALSE;
}

void* ueSingleBlockAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	UE_ASSERT(ueIsAligned(m_memory, alignment));

	// Free

	if (memory && !size)
	{
		UE_ASSERT(m_isAllocated);
		UE_ASSERT(m_memory == memory);
		m_isAllocated = UE_FALSE;
		return NULL;
	}

	// Allocate

	if (!memory && size)
	{
		UE_ASSERT(!m_isAllocated);
		UE_ASSERT(m_memorySize <= size);
		m_isAllocated = UE_TRUE;
		return m_memory;
	}

	// Reallocate

	UE_ASSERT(memory && size);
	UE_ASSERT(m_isAllocated);
	UE_ASSERT(m_memory == memory);
	UE_ASSERT(m_memorySize <= size);
	return m_memory;
}
#include "Base/Allocators/ueDLMallocAllocator.h"
#include "Base/Allocators/dlmalloc.h"

void ueDLMallocAllocator::InitMem(void* memory, ueSize memorySize)
{
	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal | ueAllocatorStatsFlags_HasUsed | ueAllocatorStatsFlags_HasUsedPeak;
	m_stats.m_total = memorySize;

	m_reallocFunc = _ReallocFunc;
	m_memory = memory;
	m_memorySize = memorySize;
	m_mspace = create_mspace_with_base(m_memory, m_memorySize, false);
	UE_ASSERT(m_mspace);
}

void ueDLMallocAllocator::Deinit()
{
	destroy_mspace(m_mspace);
	m_mspace = NULL;
}

void* ueDLMallocAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	// Freeing

	if (memory && !size)
	{
		m_stats.RemoveAlloc(mspace_usable_size(memory));
		mspace_free(m_mspace, memory);
		return NULL;
	}

	// Allocation

	if (!memory && size)
	{
		void* result = mspace_memalign(m_mspace, alignment, size);
		if (result)
			m_stats.AddAlloc(mspace_usable_size(result));
		return result;
	}

	// Reallocation

	const ueSize oldSize = mspace_usable_size(memory);
	const ueSize newSizeAligned = ueAlignPow2(size, alignment);
	if (newSizeAligned <= oldSize && oldSize <= newSizeAligned + 32)
		return memory;

	void* result = mspace_memalign(m_mspace, alignment, size);
	if (!result)
		return NULL;
	ueMemCpy(result, memory, ueMin(size, oldSize));
	mspace_free(m_mspace, memory);
	m_stats.UpdateAlloc(oldSize, mspace_usable_size(result));
	return result;
}
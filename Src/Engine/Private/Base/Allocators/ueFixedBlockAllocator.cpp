#include "Base/Allocators/ueFixedBlockAllocator.h"

ueSize ueFixedBlockAllocator::CalcMemReq(ueSize blockSize, u32 numBlocks, ueSize blockAlignment)
{
	blockSize = ueAlignPow2(blockSize, blockAlignment);
	return ueGenericPool::CalcMemReq(blockSize, numBlocks);
}

void ueFixedBlockAllocator::InitMem(void* memory, ueSize memorySize, ueSize blockSize, u32 numBlocks, ueSize blockAlignment)
{
	UE_ASSERT(memory);
	UE_ASSERT(memorySize = CalcMemReq(blockSize, numBlocks, blockAlignment));

	blockSize = ueAlignPow2(blockSize, blockAlignment);

	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal | ueAllocatorStatsFlags_HasUsed | ueAllocatorStatsFlags_HasUsedPeak | ueAllocatorStatsFlags_HasLargestBlock;
	m_stats.m_total = numBlocks * blockSize;
	m_stats.m_largestBlock = blockSize;

	m_reallocFunc = _ReallocFunc;
	m_pool.InitMem(memory, memorySize, blockSize, numBlocks);
}

void* ueFixedBlockAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	// Free

	if (memory && !size)
	{
		m_pool.Free(memory);
		m_stats.RemoveAlloc(m_pool.GetBlockSize());
		return NULL;
	}

	// Allocate

	UE_ASSERT(size <= m_pool.GetBlockSize());

	if (!memory && size)
	{
		memory = m_pool.Alloc();
		if (memory)
			m_stats.AddAlloc(m_pool.GetBlockSize());
		return memory;
	}

	// Reallocate

	UE_ASSERT(memory && size);
	return memory;
}
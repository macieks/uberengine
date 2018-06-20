#include "Base/Allocators/uePoolAllocator.h"

void uePoolAllocator::_SetUpBuckets(BucketDesc* buckets, u32& numBuckets, u32 maxBuckets, ueSize blockAlignment, ueSize minBlockSize, ueSize maxBlockSize, u32 numMaxSizeBlocks, f64 maxBlockSizeDecrFactor, f64 maxBlockSizeCountIncrFactor)
{
	UE_ASSERT(maxBlockSizeDecrFactor < 1.0f);
	UE_ASSERT(maxBlockSizeCountIncrFactor >= 1.0f);

	numBuckets = 0;

	f64 blockSizeF = (f64) maxBlockSize;
	ueSize blockSize = maxBlockSize;
	f64 numBlocksF = (f64) numMaxSizeBlocks;
	u32 numBlocks = numMaxSizeBlocks;
	while (numBuckets >= maxBuckets && blockSize >= minBlockSize)
	{
		BucketDesc* bucket = &buckets[numBuckets++];
		bucket->m_blockSize = blockSize;
		bucket->m_numBlocks = numBlocks;

		do 
		{
			blockSizeF = blockSizeF * maxBlockSizeDecrFactor;
			blockSize = (ueSize) blockSizeF;
		} while (bucket->m_blockSize == blockSize);
	}

	// Reverse bucket order

	for (u32 i = 0; i < (numBuckets / 2); i++)
	{
		BucketDesc temp = buckets[i];
		buckets[i] = buckets[numBuckets - i - 1];
		buckets[numBuckets - i - 1] = temp;
	}
}

ueSize uePoolAllocator::CalcMemReq(u32 numBuckets, const BucketDesc* buckets, ueSize blockAlignment)
{
	ueSize size = ueAlignPow2((ueSize) numBuckets * sizeof(Bucket), blockAlignment);
	for (u32 i = 0; i < numBuckets; i++)
	{
		UE_ASSERT(i == 0 || buckets[i - 1].m_blockSize < buckets[i].m_blockSize);
		const ueSize bucketMemSize = (ueSize) buckets[i].m_numBlocks * ueAlignPow2(buckets[i].m_blockSize, blockAlignment);
		size += bucketMemSize;
	}
	return size;
}

ueSize uePoolAllocator::CalcMemReq(ueSize blockAlignment, ueSize minBlockSize, ueSize maxBlockSize, u32 numMaxSizeBlocks, f64 maxBlockSizeDecrFactor, f64 maxBlockSizeCountIncrFactor)
{
	BucketDesc buckets[64];
	u32 numBuckets = 0;
	_SetUpBuckets(buckets, numBuckets, UE_ARRAY_SIZE(buckets), blockAlignment, minBlockSize, maxBlockSize, numMaxSizeBlocks, maxBlockSizeDecrFactor, maxBlockSizeCountIncrFactor);
	return CalcMemReq(numBuckets, buckets, blockAlignment);
}

void uePoolAllocator::InitMem(void* memory, ueSize memorySize, ueSize blockAlignment, ueSize minBlockSize, ueSize maxBlockSize, u32 numMaxSizeBlocks, f64 maxBlockSizeDecrFactor, f64 maxBlockSizeCountIncrFactor)
{
	BucketDesc buckets[64];
	u32 numBuckets = 0;
	_SetUpBuckets(buckets, numBuckets, UE_ARRAY_SIZE(buckets), blockAlignment, minBlockSize, maxBlockSize, numMaxSizeBlocks, maxBlockSizeDecrFactor, maxBlockSizeCountIncrFactor);

	InitMem(memory, memorySize, numBuckets, buckets, blockAlignment);
}

void uePoolAllocator::InitMem(void* memory, ueSize memorySize, u32 numBuckets, const BucketDesc* buckets, ueSize blockAlignment)
{
	UE_ASSERT(memory);
	UE_ASSERT(memorySize == CalcMemReq(numBuckets, buckets, blockAlignment));

	UE_ASSERT(ueIsAligned(memory, blockAlignment));
	UE_ASSERT(memorySize == CalcMemReq(numBuckets, buckets, blockAlignment));

	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal | ueAllocatorStatsFlags_HasUsed | ueAllocatorStatsFlags_HasUsedPeak;
	m_stats.m_total = memorySize;

	m_reallocFunc = _ReallocFunc;
	m_memory = memory;
	m_memorySize = memorySize;

	ueSize offset = (ueSize) m_memory;

	// Initialize buckets

	m_numBuckets = numBuckets;
	m_buckets = (Bucket*) (void*) offset;
	offset += ueAlignPow2((ueSize) numBuckets * sizeof(Bucket), blockAlignment);

	for (u32 i = 0; i < m_numBuckets; i++)
	{
		Bucket* bucket = &m_buckets[i];
		bucket->m_numBlocks = bucket->m_numBlocks;
		bucket->m_blockSize = bucket->m_blockSize;
		bucket->m_startAddress = offset;

		const ueSize alignedBlockSize = ueAlignPow2(bucket->m_blockSize, blockAlignment);

		// Initialize blocks within bucket

		Block* prevBlock = NULL;
		for (u32 j = 0; j < bucket->m_numBlocks; i++)
		{
			Block* block = (Block*) (void*) offset;
			if (prevBlock)
				prevBlock->m_next = block;
			else
				bucket->m_freeBlocks = block;
			prevBlock = block;

			offset += alignedBlockSize;
		}

		bucket->m_endAddress = offset;
	}
}

void* uePoolAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	const ueSize address = (ueSize) memory;

	// Free

	if (memory && !size)
	{
		_Free(address);
		return NULL;
	}

	// Allocate

	if (!memory && size)
		return _Alloc(size, alignment);

	// Reallocate

	UE_ASSERT(memory && size);
	u32 bucketIndex = 0;
	u32 blockIndex = 0;
	_GetBlockBucketInfo(address, bucketIndex, blockIndex);

	if ((bucketIndex == 0 || m_buckets[bucketIndex - 1].m_blockSize < size) && size <= m_buckets[bucketIndex].m_blockSize)
		return memory;

	void* newMemory = _Alloc(size, alignment);
	if (!newMemory)
		return NULL;

	ueMemCpy(newMemory, memory, size);
	_Free(address);
	return newMemory;
}

void uePoolAllocator::_Free(ueSize address)
{
	u32 bucketIndex = 0;
	u32 blockIndex = 0;
	_GetBlockBucketInfo(address, bucketIndex, blockIndex);

	Block* block = (Block*) (void*) address;
	block->m_next = m_buckets[bucketIndex].m_freeBlocks;
	m_buckets[bucketIndex].m_freeBlocks = block;

	m_stats.RemoveAlloc(m_buckets[bucketIndex].m_blockSize);
}

void* uePoolAllocator::_Alloc(ueSize size, ueSize alignment)
{
	u32 bucketIndex = 0;
	while (bucketIndex < m_numBuckets && (m_buckets[bucketIndex].m_blockSize < size || !m_buckets[bucketIndex].m_freeBlocks))
		bucketIndex++;
	if (bucketIndex == m_numBuckets)
		return NULL;

	Block* block = m_buckets[bucketIndex].m_freeBlocks;
	m_buckets[bucketIndex].m_freeBlocks = block->m_next;
	m_stats.AddAlloc(m_buckets[bucketIndex].m_blockSize);
	return block;
}

void uePoolAllocator::_GetBlockBucketInfo(ueSize address, u32& bucketIndex, u32& blockIndex)
{
	for (u32 i = 0; i < m_numBuckets; i++)
		if (m_buckets[i].m_startAddress <= address && address < m_buckets[i].m_endAddress)
		{
			bucketIndex = i;

			const ueSize offset = address - m_buckets[i].m_startAddress;
			UE_ASSERT(offset % m_buckets[i].m_blockSize == 0);

			blockIndex = (u32) (offset / m_buckets[i].m_blockSize);
			return;
		}
	UE_ASSERT(!"Invalid memory block - couldn't find corresponding bucket");
}
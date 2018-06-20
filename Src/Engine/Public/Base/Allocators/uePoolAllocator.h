#ifndef UE_POOL_ALLOCATOR_H
#define UE_POOL_ALLOCATOR_H

#include "Base/ueBase.h"

//! Allocator that allocates blocks from pre-generated pool of pre-defined sizes
class uePoolAllocator : public ueAllocator
{
public:
	struct BucketDesc
	{
		u32 m_numBlocks;
		ueSize m_blockSize;
	};

private:
	struct Block
	{
		Block* m_next;
	};

	struct Bucket
	{
		ueSize m_startAddress;
		ueSize m_endAddress;

		u32 m_numBlocks;
		ueSize m_blockSize;

		Block* m_freeBlocks;
	};

	u32 m_numBuckets;
	Bucket* m_buckets;

	void* m_memory;
	ueSize m_memorySize;

public:
	static ueSize CalcMemReq(ueSize blockAlignment = 16, ueSize minBlockSize = 32, ueSize maxBlockSize = 256, u32 numMaxSizeBlocks = 128, f64 maxBlockSizeDecrFactor = 0.5, f64 maxBlockSizeCountIncrFactor = 2.0);
	static ueSize CalcMemReq(u32 numBuckets, const BucketDesc* buckets, ueSize blockAlignment = 16);

	void InitMem(void* memory, ueSize memorySize, ueSize blockAlignment = 16, ueSize minBlockSize = 32, ueSize maxBlockSize = 256, u32 numMaxSizeBlocks = 128, f64 maxBlockSizeDecrFactor = 0.5, f64 maxBlockSizeCountIncrFactor = 2.0);
	void InitMem(void* memory, ueSize memorySize, u32 numBuckets, const BucketDesc* buckets, ueSize blockAlignment = 16);

private:
	static void _SetUpBuckets(BucketDesc* buckets, u32& numBuckets, u32 maxBuckets, ueSize blockAlignment, ueSize minBlockSize, ueSize maxBlockSize, u32 numMaxSizeBlocks, f64 maxBlockSizeDecrFactor, f64 maxBlockSizeCountIncrFactor);
	void _GetBlockBucketInfo(ueSize address, u32& bucketIndex, u32& blockIndex);

	void _Free(ueSize address);
	void* _Alloc(ueSize size, ueSize alignment);

	UE_DECLARE_ALLOC_FUNCS(uePoolAllocator)
};

#endif // UE_POOL_ALLOCATOR_H

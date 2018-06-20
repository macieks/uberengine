#ifndef UE_FIXED_BLOCK_ALLOCATOR_H
#define UE_FIXED_BLOCK_ALLOCATOR_H

#include "Base/Containers/ueGenericPool.h"

//! Fixed block size allocator; super fast and zero per-allocation overhead
class ueFixedBlockAllocator : public ueAllocator
{
public:
	static ueSize CalcMemReq(ueSize blockSize, u32 numBlocks, ueSize blockAlignment);
	void InitMem(void* memory, ueSize memorySize, ueSize blockSize, u32 numBlocks, ueSize blockAlignment);

	UE_DECLARE_ALLOC_FUNCS(ueFixedBlockAllocator)

private:
	ueGenericPool m_pool;
};

#endif // UE_FIXED_BLOCK_ALLOCATOR_H

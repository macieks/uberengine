#ifndef UE_STACK_ALLOCATOR_H
#define UE_STACK_ALLOCATOR_H

#include "Base/ueBase.h"

//! Stack allocator (can only allocate & free in matching order; super fast & very low per allocation overhead)
class ueStackAllocator : public ueAllocator
{
private:
	void* m_memory;
	ueSize m_capacity;
	ueSize m_size;

	u32* m_offsets;
	u32 m_numAllocs;
	u32 m_maxAllocs;

public:
	UE_INLINE static u32 CalcDebugMemReq(u32 maxAllocs) { return sizeof(u32) * maxAllocs; }

	//! Initializes from memory; debug memory (optional) is used for debugging allocation / deallocation pointers
	void InitMem(void* memory, ueSize memorySize, u32 maxAllocs = 0, void* debugMemory = NULL, ueSize debugMemorySize = 0);

	UE_INLINE ueSize GetOffset() const { return m_size; }
	UE_INLINE void SetOffset(ueSize size) { m_size = size; }

	UE_DECLARE_ALLOC_FUNCS(ueStackAllocator)
};

//! Helper class used to automatically rewind stack allocator state on destruction
class ueStackAllocatorRewinder
{
public:
	UE_INLINE ueStackAllocatorRewinder(ueStackAllocator* allocator) :
		m_allocator(allocator),
		m_sizeToRewind(m_allocator->GetOffset())
	{}

	UE_INLINE ~ueStackAllocatorRewinder()
	{
		if (m_sizeToRewind != U32_MAX)
			m_allocator->SetOffset(m_sizeToRewind);
	}

	UE_INLINE void Disable() { m_sizeToRewind = U32_MAX; }

private:
	ueStackAllocator* m_allocator;
	ueSize m_sizeToRewind;
};

#endif // UE_STACK_ALLOCATOR_H

#ifndef UE_SINGLE_BLOCK_ALLOCATOR_H
#define UE_SINGLE_BLOCK_ALLOCATOR_H

#include "Base/ueBase.h"

//! An allocator that can only allocate single memory block (so, it's super quick)
class ueSingleBlockAllocator : public ueAllocator
{
private:
	void* m_memory;
	ueSize m_memorySize;
	ueBool m_isAllocated;

public:
	ueSingleBlockAllocator();
	void InitMem(void* memory, ueSize memorySize);

	UE_DECLARE_ALLOC_FUNCS(ueSingleBlockAllocator)
};

//! Allocates fixed size vector on the stack
#define UE_INIT_SINGLE_BLOCK_ALLOCATOR_WITH_STACK_MEMORY(varName, memSize) \
	u8 varName##_memory[memSize]; \
	ueSingleBlockAllocator varName; \
	varName.InitMem(varName##_memory, memSize);

#endif // UE_SINGLE_BLOCK_ALLOCATOR_H

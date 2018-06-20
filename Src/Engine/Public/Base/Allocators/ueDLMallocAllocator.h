#ifndef UE_DLMALLOC_ALLOCATOR_H
#define UE_DLMALLOC_ALLOCATOR_H

#include "Base/ueBase.h"

//! dlmalloc (Doug Lea's allocator) based allocator
class ueDLMallocAllocator : public ueAllocator
{
private:
	void* m_memory;
	ueSize m_memorySize;
	void* m_mspace;

public:
	void InitMem(void* memory, ueSize memorySize);
	void Deinit();

	UE_DECLARE_ALLOC_FUNCS(ueDLMallocAllocator)
};

#endif // UE_DLMALLOC_ALLOCATOR_H

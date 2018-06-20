#ifndef UE_MALLOC_ALLOCATOR_H
#define UE_MALLOC_ALLOCATOR_H

#include "Base/ueBase.h"

//! Standard malloc/free based allocator
class ueMallocAllocator : public ueAllocator
{
public:
	ueMallocAllocator();

	UE_DECLARE_ALLOC_FUNCS(ueMallocAllocator)
};

#endif // UE_MALLOC_ALLOCATOR_H

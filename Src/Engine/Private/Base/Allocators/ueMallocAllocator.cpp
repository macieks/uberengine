#include "Base/Allocators/ueMallocAllocator.h"

#include <stdlib.h>

#if defined(UE_TOOLS)
	ueAllocator* ueAllocator::GetGlobal()
	{
		static ueMallocAllocator s_globalAllocator;
		return &s_globalAllocator;
	}
#endif // defined(UE_TOOLS)

ueMallocAllocator::ueMallocAllocator()
{
	m_reallocFunc = _ReallocFunc;
}

void* ueMallocAllocator::_Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	// Free

	if (memory && !size)
	{
#if !defined(UE_WIN32)
		free(memory);
#else
		_aligned_free(memory);
#endif
		m_stats.RemoveAlloc(0);
		return NULL;
	}

	// Alloc

	else if (!memory && size)
	{
#if !defined(UE_WIN32)
		void* result = malloc(size);
#else
		void* result = _aligned_malloc(size, alignment);
#endif
		if (result)
		{
			UE_ASSERT(ueIsAligned(result, alignment));
			m_stats.AddAlloc(0);
		}

#if 0 // Debugging memory usage under Marmalade
		const u32 used = s3eMemoryGetInt(S3E_MEMORY_USED) >> 10;
		const u32 total = s3eMemoryGetInt(S3E_MEMORY_SIZE) >> 10;
		ueLogD("MEMORY used %u / %u kB", used, total);
#endif

		return result;
	}

	// Realloc

	else
	{
		UE_ASSERT(memory && size);
#if !defined(UE_WIN32)
		void* result = realloc(memory, size);
#else
		void* result = _aligned_realloc(memory, size, alignment);
#endif
		if (!result)
			return NULL;
		UE_ASSERT(ueIsAligned(result, alignment));
		return result;
	}
}
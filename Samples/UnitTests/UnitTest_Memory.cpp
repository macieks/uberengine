#include "Base/Allocators/ueBestFitAllocator.h"

void UnitTest_Memory(ueAllocator* allocator)
{
	ueBestFitAllocator a;

	// Initialize allocator

	const u32 numBlocks = 3;
	void* ptrs[numBlocks];
	ueSize sizes[numBlocks];
	for (u32 i = 0; i < numBlocks; i++)
	{
		sizes[i] = 16 << i;
		ptrs[i] = allocator->Alloc(sizes[i]);
		UE_ASSERT(ptrs[i]);
	}

	const u32 maxManagedBlocks = 1024;
	const ueSize managementDataSize = a.CalcManagementMemoryReq(maxManagedBlocks);
	void* managementData = allocator->Alloc(managementDataSize);
	UE_ASSERT(managementData);
	a.InitMem(managementData, managementDataSize, maxManagedBlocks, 4, numBlocks, ptrs, sizes);

	a.DebugDump();

	// Do allocations

	const u32 numTestAllocs = 5;
	void* allocs[numTestAllocs];

	for (u32 i = 0; i < numTestAllocs; i++)
	{
		const u32 allocSize = 2 << i;
		ueLog(" <<< ALLOC %d >>>\n", allocSize);
		allocs[i] = a.Alloc(allocSize, 4);
		a.DebugDump();
	}

	for (u32 i = 0; i < numTestAllocs; i++)
	{
		const u32 allocSize = 2 << i;
		ueLog(" <<< FREE %d >>>\n", allocSize);
		a.Free(allocs[i]);
		a.DebugDump();
	}

	// Clean up

	a.Deinit();

	for (u32 i = 0; i < numBlocks; i++)
		allocator->Free(ptrs[i]);
	allocator->Free(managementData);
}

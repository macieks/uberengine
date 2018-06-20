#include "Base/Containers/ueRedBlackTreeSet.h"
#include "Base/ueRand.h"

typedef ueRedBlackTreeSet<u32> SetType;
#define NUM_ELEMS 65536

void UnitTest_RedBlackTreeSet(ueAllocator* allocator)
{
	// Generate unique numbers

	ueRand* r = ueRand_GetGlobal();
	u32 nums[NUM_ELEMS];
	for (u32 i = 0; i < NUM_ELEMS; i++)
		nums[i] = i + (ueRand_U32(r) & 0xFFFF0000);

	// Initialize set

	SetType set;
	UE_ASSERT_FUNC(set.Init(allocator, NUM_ELEMS));

	// Insert

	for (u32 i = 0; i < NUM_ELEMS; i++)
	{
		UE_ASSERT(set.Size() == i);
		set.Insert(nums[i]);
	}

	// Test searching

	for (u32 i = 0; i < NUM_ELEMS; i++)
	{
		SetType::Iterator iter = set.Find(nums[i]);
		UE_ASSERT(iter.Valid());

		SetType::Iterator iter2 = set.LowerBound(nums[i]);
		UE_ASSERT(iter2.Valid());

		UE_ASSERT(*iter == *iter2);
	}

	u32 count = 0;
	SetType::Iterator iter(set);
	while (iter.Valid())
	{
		count++;
		iter++;
	}
	UE_ASSERT(count == NUM_ELEMS);

	// Remove

	for (u32 i = 0; i < NUM_ELEMS; i++)
	{
		UE_ASSERT(set.Size() == NUM_ELEMS - i);
		set.Remove(nums[i]);
	}

	// Deinit

	set.Deinit();
}
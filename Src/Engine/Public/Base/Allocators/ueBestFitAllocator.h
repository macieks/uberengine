#ifndef UE_BEST_FIT_ALLOCATOR_H
#define UE_BEST_FIT_ALLOCATOR_H

#include "Base/Containers/ueGenericPool.h"
#include "Base/Containers/ueRedBlackTreeMap.h"

/**
 *	Best fit allocator.
 *
 *	- non-intrusive (managed memory is kept separate from management structures)
 *	- best fit (uses red black trees to maintain internal structures)
 *	- O(logn) alloc/free time
 *	- large per-block overhead of ~85 bytes
 *
 *	Useful where it's prefered to keep managed memory separate from memory management structures,
 *	for example when managing physical memory on X360 or local memory on PS3.
 */
class ueBestFitAllocator : public ueAllocator
{
public:
	static ueSize CalcManagementMemoryReq(u32 maxManagedBlocks);
	void InitMem(void* managementMemory, ueSize managementMemorySize, u32 maxManagedBlocks, ueSize minAlignment, u32 numBlocks, void** blocks, ueSize* blockSizes);
	void Deinit();

	void DebugDump();

private:

	UE_DECLARE_ALLOC_FUNCS(ueBestFitAllocator)

	void* _Alloc(ueSize size, ueSize alignment);
	void _Free(void* ptr);

	struct Elem
	{
		void* m_address;
		Elem* m_next;
		Elem* m_prev;

		struct Pred
		{
			UE_INLINE s32 operator () (const Elem* a, const Elem* b) const
			{
				return ueCmpPredicate<void*>()(a->m_address, b->m_address);
			}
		};
	};

	typedef ueRedBlackTreeMap<void*, ueSize> UsedByAddrMapType;
	typedef ueRedBlackTreeMap<Elem*, ueSize, Elem::Pred> FreeByAddrMapType;
	typedef ueRedBlackTreeMap<ueSize, Elem*> FreeBySizeMapType;

	void AddFreeBySize(FreeByAddrMapType::Iterator freeByAddrIter);
	void AddFreeBySize(Elem* elem, ueSize size);
	void AddFree(void* ptr, ueSize size);

	void RemoveFreeBySize(FreeByAddrMapType::Iterator freeByAddrIter, ueBool releaseElem);
	void RemoveFreeBySize(FreeBySizeMapType::Iterator freeBySizeIter, Elem* elem, ueBool releaseElem);
	void RemoveFree(FreeByAddrMapType::Iterator freeByAddrIter);
	void RemoveFree(FreeBySizeMapType::Iterator freeBySizeIter, Elem* elem);

	ueBool SplitBlock(ueSize requestedSize, ueSize alignment, ueSize blockPtr, ueSize blockSize, ueSize& startPtr, ueSize& startSize, ueSize& ptr, ueSize& size, ueSize& endPtr, ueSize& endSize);

	ueSize m_minAlignment;
	ueGenericPool m_elemsPool;

	UsedByAddrMapType m_usedByAddr; // map: address -> size
	FreeByAddrMapType m_freeByAddr; // map: address -> size
	FreeBySizeMapType m_freeBySize; // map: size -> list of addresses
};

#endif // UE_BEST_FIT_ALLOCATOR_H

#include "Base/Allocators/ueBestFitAllocator.h"

ueSize ueBestFitAllocator::CalcManagementMemoryReq(u32 maxManagedBlocks)
{
	return
		ueGenericPool::CalcMemReq(sizeof(Elem), maxManagedBlocks) +
		UsedByAddrMapType::CalcMemReq(maxManagedBlocks) +
		FreeByAddrMapType::CalcMemReq(maxManagedBlocks) +
		FreeBySizeMapType::CalcMemReq(maxManagedBlocks);
}

void ueBestFitAllocator::InitMem(void* managementData, ueSize managementDataSize, u32 maxManagedBlocks, ueSize minAlignment, u32 numBlocks, void** blocks, ueSize* blockSizes)
{
	UE_ASSERT(managementData);
	UE_ASSERT(managementDataSize == CalcManagementMemoryReq(maxManagedBlocks));

	ueSize containerSize;
	u8* containerMemory = (u8*) managementData;

	containerSize = m_elemsPool.CalcMemReq(sizeof(Elem), maxManagedBlocks);
	m_elemsPool.InitMem(containerMemory, containerSize, sizeof(Elem), maxManagedBlocks);
	containerMemory += containerSize;

	containerSize = m_usedByAddr.CalcMemReq(maxManagedBlocks);
	m_usedByAddr.InitMem(containerMemory, containerSize, maxManagedBlocks);
	containerMemory += containerSize;

	containerSize = m_freeByAddr.CalcMemReq(maxManagedBlocks);
	m_freeByAddr.InitMem(containerMemory, containerSize, maxManagedBlocks);
	containerMemory += containerSize;

	containerSize = m_freeBySize.CalcMemReq(maxManagedBlocks);
	m_freeBySize.InitMem(containerMemory, containerSize, maxManagedBlocks);

	m_stats.m_flags = ueAllocatorStatsFlags_HasTotal | ueAllocatorStatsFlags_HasUsed | ueAllocatorStatsFlags_HasUsedPeak;
	m_stats.m_total = 0;

	m_reallocFunc = _ReallocFunc;

	m_minAlignment = minAlignment;
	for (u32 i = 0; i < numBlocks; i++)
	{
		m_stats.m_total += blockSizes[i];
		AddFree(blocks[i], blockSizes[i]);
	}
}

void ueBestFitAllocator::Deinit()
{
	m_usedByAddr.Deinit();
	m_freeByAddr.Deinit();
	m_freeBySize.Deinit();
	m_elemsPool.Deinit();
}

void* ueBestFitAllocator::_Realloc(void* ptr, ueSize _size, ueSize _alignment, void* allocInfo)
{
	ueSize size = (ueSize) _size;
	ueSize alignment = (ueSize) _alignment;

	if (!ptr && size)
		return _Alloc(size, alignment);
	else if (ptr && !size)
	{
		_Free(ptr);
		return NULL;
	}

	// Realloc

	UE_ASSERT(ptr && size);

	Elem ptrElem;
	ptrElem.m_address = ptr;
	FreeByAddrMapType::Iterator iter = m_freeByAddr.Find(&ptrElem);
	UE_ASSERT(iter.Valid());
	UE_ASSERT(ueIsAligned(ptr, alignment)); // Can't realloc with different alignment

	size = ueAlignPow2(size, alignment);
	if (iter.Value() >= size)
		return ptr; // No need to reallocate

	void* newPtr = _Alloc(size, alignment);
	if (!newPtr)
		return NULL; // Failure
	ueMemCpy(newPtr, ptr, ueMin(size, iter.Value()));
	_Free(ptr);
	return newPtr;
}

ueBool ueBestFitAllocator::SplitBlock(ueSize requestedSize, ueSize alignment, ueSize blockPtr, ueSize blockSize, ueSize& startPtr, ueSize& startSize, ueSize& ptr, ueSize& size, ueSize& endPtr, ueSize& endSize)
{
	// Align and check if there's enough space after aligning

	ptr = ueAlignPow2(blockPtr, ueMax(m_minAlignment, alignment));
	if (ptr + requestedSize > blockPtr + blockSize)
		return UE_FALSE;

	// Determine start block

	if (ptr == blockPtr)
		startPtr = 0;
	else
	{
		startPtr = blockPtr;
		startSize = ptr - blockPtr;
	}

	// Determine end block

	const ueSize endAligned = ueAlignPow2(ptr + requestedSize, m_minAlignment);

	if (endAligned >= blockPtr + blockSize)
	{
		endPtr = 0;
		size = blockSize - (ptr - blockPtr);
	}
	else
	{
		endPtr = endAligned;
		endSize = blockSize - (endPtr - blockPtr);
		size = endAligned - ptr;
	}

	return UE_TRUE;
}

void* ueBestFitAllocator::_Alloc(ueSize size, ueSize alignment)
{
	size = ueAlignPow2(size, ueMax(m_minAlignment, alignment));

	for (FreeBySizeMapType::Iterator i = m_freeBySize.LowerBound(size); i.Valid(); ++i)
		for (Elem* j = i.Value(); j; j = j->m_next)
		{
			// Determine splits

			ueSize startPtr = NULL;
			ueSize startSize = 0;
			ueSize ptr = NULL;
			ueSize endPtr = NULL;
			ueSize endSize = 0;
			if (!SplitBlock(size, alignment, (ueSize) j->m_address, i.Key(), startPtr, startSize, ptr, size, endPtr, endSize))
				continue; // May fail even though block is large enough but isn't after applying alignment

			// Split

			RemoveFree(i, j);

			if (startPtr)
				AddFree((void*) startPtr, startSize);
			if (endPtr)
				AddFree((void*) endPtr, endSize);

			m_usedByAddr.Insert((void*) ptr, size);
			m_stats.AddAlloc(size);
			return (void*) ptr;
		}
	return NULL;
}

void ueBestFitAllocator::_Free(void* ptr)
{
	// Look up used block

	UsedByAddrMapType::Iterator usedIter = m_usedByAddr.Find(ptr);
	UE_ASSERT(usedIter.Valid());

	ueSize size = usedIter.Value();
	m_stats.RemoveAlloc(size);

	// Merge with next block

	Elem ptrElem;
	ptrElem.m_address = ptr;

	FreeByAddrMapType::Iterator next = m_freeByAddr.LowerBound(&ptrElem);
	if (next.Valid() && (u8*) ptr + usedIter.Value() == next.Key()->m_address)
	{
		size += next.Value();
		RemoveFree(next);
	}

	// Merge with previous block

	FreeByAddrMapType::Iterator prev = m_freeByAddr.LowerBound(&ptrElem);
	if (prev.Valid())
		--prev;
	if (prev.Valid() && (u8*) prev.Key()->m_address + prev.Value() == ptr)
	{
		RemoveFreeBySize(prev, UE_FALSE);
		prev.Value() += size;
		AddFreeBySize(prev);
	}
	else
		AddFree(ptr, size);

	// Return used block

	m_usedByAddr.Remove(usedIter);
}

void ueBestFitAllocator::AddFreeBySize(FreeByAddrMapType::Iterator freeByAddrIter)
{
	AddFreeBySize(freeByAddrIter.Key(), freeByAddrIter.Value());
}

void ueBestFitAllocator::AddFreeBySize(Elem* elem, ueSize size)
{
	FreeBySizeMapType::Iterator freeBySizeIter = m_freeBySize.Find(size);
	if (!freeBySizeIter.Valid())
		m_freeBySize.Insert(size, elem);
	else
	{
		elem->m_next = freeBySizeIter.Value();
		freeBySizeIter.Value() = elem;
		if (elem->m_next)
			elem->m_next->m_prev = elem;
	}
}

void ueBestFitAllocator::AddFree(void* ptr, ueSize size)
{
	Elem* elem = (Elem*) m_elemsPool.Alloc();
	UE_ASSERT(elem);
	elem->m_address = ptr;
	elem->m_next = NULL;
	elem->m_prev = NULL;

	m_freeByAddr.Insert(elem, size);
	AddFreeBySize(elem, size);
}

void ueBestFitAllocator::RemoveFreeBySize(FreeByAddrMapType::Iterator freeByAddrIter, ueBool releaseElem)
{
	FreeBySizeMapType::Iterator freeBySizeIter = m_freeBySize.Find(freeByAddrIter.Value());
	UE_ASSERT(freeBySizeIter.Valid());
	Elem* elem = freeByAddrIter.Key();
	RemoveFreeBySize(freeBySizeIter, elem, releaseElem);
}

void ueBestFitAllocator::RemoveFreeBySize(FreeBySizeMapType::Iterator freeBySizeIter, Elem* elem, ueBool releaseElem)
{
	// Remove from the list

	if (elem->m_prev)
		elem->m_prev->m_next = elem->m_next;
	else
		freeBySizeIter.Value() = elem->m_next;
	if (elem->m_next)
		elem->m_next->m_prev = elem->m_prev;

	if (releaseElem)
		m_elemsPool.Free(elem);
	else
	{
		elem->m_next = NULL;
		elem->m_prev = NULL;
	}

	// Remove whole map entry if list is empty

	if (!freeBySizeIter.Value())
		m_freeBySize.Remove(freeBySizeIter);
}

void ueBestFitAllocator::RemoveFree(FreeByAddrMapType::Iterator freeByAddrIter)
{
	FreeBySizeMapType::Iterator freeBySizeIter = m_freeBySize.Find(freeByAddrIter.Value());
	UE_ASSERT(freeBySizeIter.Valid());
	RemoveFree(freeBySizeIter, freeByAddrIter.Key());
}

void ueBestFitAllocator::RemoveFree(FreeBySizeMapType::Iterator freeBySizeIter, Elem* elem)
{
	m_freeByAddr.Remove(elem);
	RemoveFreeBySize(freeBySizeIter, elem, UE_TRUE);
}

void ueBestFitAllocator::DebugDump()
{
	ueLog("=== BLOCKS [%d]: ===\n", m_freeByAddr.Size() + m_usedByAddr.Size());

	void* prevEnd = NULL;
	FreeByAddrMapType::Iterator i(m_freeByAddr);
	UsedByAddrMapType::Iterator j(m_usedByAddr);
	while (i.Valid() || j.Valid())
	{
		ueBool free;
		if (i.Valid() && j.Valid())
			free = i.Key()->m_address < j.Key();
		else
			free = i.Valid();
			
		if (prevEnd && prevEnd != (free ? i.Key()->m_address : j.Key()))
			ueLog(".... 0x%x %d\n", prevEnd, (u32) (ueSize) (ueSize) (free ? i.Key()->m_address : j.Key()) - (u32) (ueSize) prevEnd);

		ueLog("%s 0x%x %d\n", free ? "FREE" : "used", (free ? i.Key()->m_address : j.Key()), (free ? i.Value() : j.Value()));
		prevEnd = (u8*) (free ? i.Key()->m_address : j.Key()) + (free ? i.Value() : j.Value());

		if (free)
			++i;
		else
			++j;
	}
}
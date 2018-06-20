#include "Base/Containers/ueGenericPool.h"

#define UE_GENERIC_POOL_ITER_LAST ((s32) 0x7FFFFFFF)

ueGenericPool::ueGenericPool() :
	m_allocator(NULL),
	m_memory(NULL),
	m_memorySize(0),
	m_blockSize(0),
	m_numBlocks(0),
	m_freeBlocks(NULL),
	m_numUsedBlocks(0),
	m_usageMask(NULL)
{}

ueGenericPool::~ueGenericPool()
{
	Deinit();
}

ueBool ueGenericPool::Init(ueAllocator* allocator, ueSize blockSize, u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	const ueSize memorySize = CalcMemReq(blockSize, numBlocks, blockAlignment, supportIterator);

	m_allocator = allocator;

	void* memory = m_allocator->Alloc(memorySize, blockAlignment);
	if (!memory)
		return UE_FALSE;
	InitMem(memory, memorySize, blockSize, numBlocks, blockAlignment, supportIterator);
	return UE_TRUE;
}

ueSize ueGenericPool::CalcMemReq(ueSize blockSize, u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	return ueAlignPow2(blockSize, blockAlignment) * numBlocks + (supportIterator ? ((numBlocks + 31) >> 5) * sizeof(u32) : 0);
}

void ueGenericPool::InitMem(void* memory, ueSize memorySize, ueSize blockSize, u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	UE_ASSERT(!m_memory);
	UE_ASSERT(memory);
	UE_ASSERT(memorySize == CalcMemReq(blockSize, numBlocks, blockAlignment, supportIterator));
	UE_ASSERT(blockSize > 0);
	UE_ASSERT(numBlocks > 0);

	// Initialize container

	UE_ASSERT(ueIsAligned(memory, blockAlignment));
	UE_ASSERT(blockSize * numBlocks <= memorySize);

	m_memory = memory;
	m_memorySize = memorySize;

	m_blockSize = blockSize;
	m_numBlocks = numBlocks;

	// Set up usage bit array (optional)

	if (supportIterator)
	{
		UE_ASSERT(m_memorySize - m_blockSize * m_numBlocks >= ((m_numBlocks + 31) >> 5) * sizeof(u32));
		m_usageMask = (u32*) ((u8*) memory + m_blockSize * m_numBlocks);
	}

	// Reset elements

	Clear();
}

void ueGenericPool::Deinit()
{
	if (m_memory)
	{
		if (m_allocator)
		{
			m_allocator->Free(m_memory);
			m_allocator = NULL;
		}
		m_memory = NULL;
	}

	m_memorySize = 0;

	m_usageMask = NULL;
}

void ueGenericPool::Clear()
{
	m_numUsedBlocks = 0;

	m_freeBlocks = (Block*) GetBlock(0);
	Block* prevBlock = m_freeBlocks;
	Block* block = m_freeBlocks;
	for (u32 i = 1; i < m_numBlocks; i++)
	{
		block = (Block*) GetBlock(i);
		prevBlock->m_next = block;
		prevBlock = block;
	}
	block->m_next = NULL;

	if (m_usageMask)
		ueMemSet(m_usageMask, 0, ((m_numBlocks + 31) >> 5) * sizeof(u32));
}

void* ueGenericPool::Next(Iterator& iter) const
{
	if (iter.m_current == UE_GENERIC_POOL_ITER_LAST)
		return NULL;

	if (iter.m_current == U32_MAX)
	{
		iter.m_current = 0;
		if (m_usageMask[0] & 1)
			return GetBlock(0);
	}

	// Move to next element

	u32 index = iter.m_current + 1;

	while (1)
	{
		// Jump to next non-zero 32-bit int

		while (index < m_numBlocks && !m_usageMask[index >> 5]) index += 32;
		if (index >= m_numBlocks)
		{
			iter.m_current = UE_GENERIC_POOL_ITER_LAST;
			return NULL;
		}

		// Jump to next non-zero bit

		const u32 mask = m_usageMask[index >> 5];
		const u32 startOfNext32Bits = ((index >> 5) + 1) << 5;
		const u32 iterationBound = ueMin(startOfNext32Bits, m_numBlocks);
		while (index < iterationBound && !(mask & (1 << (index & 31)))) index++;
		if (index == m_numBlocks)
		{
			iter.m_current = UE_GENERIC_POOL_ITER_LAST;
			return NULL;
		}

		// Found next element?

		if (index < iterationBound)
		{
			iter.m_current = index;
			return GetBlock(index);
		}
	}
}

void* ueGenericPool::Alloc()
{
	if (!m_freeBlocks)
		return NULL;

	if (SupportsIterator())
		SetBlockAllocated(m_freeBlocks);

	m_numUsedBlocks++;

	Block* elem = m_freeBlocks;
	m_freeBlocks = elem->m_next;
	return elem;
}

u32 ueGenericPool::AllocIndex()
{
	if (!m_freeBlocks) return U32_MAX;
	void* elem = Alloc();
	return (u32) (((ueSize) elem - (ueSize) m_memory) / m_blockSize);
}

void ueGenericPool::Free(void* ptr)
{
	UE_ASSERT(IsValidBlock(ptr));

	Block* block = (Block*) ptr;

	if (SupportsIterator())
		SetBlockFree(block);

	m_numUsedBlocks--;

	block->m_next = m_freeBlocks;
	m_freeBlocks = block;
}

void ueGenericPool::FreeIndex(u32 index)
{
	Free( GetBlock(index) );
}

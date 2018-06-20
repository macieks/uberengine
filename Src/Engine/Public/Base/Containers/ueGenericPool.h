#ifndef UE_GENERIC_POOL_H
#define UE_GENERIC_POOL_H

#include "Base/ueBase.h"

//! Pool of equally sized elements with 0 (zero) bytes per-block management overhead
class ueGenericPool
{
public:
	//! Generic pool iterator
	class Iterator
	{
		friend class ueGenericPool;
	public:
		//! Creates iterator for a given pool
		UE_INLINE Iterator(ueGenericPool& pool);
		//! Gets next (including first) element
		UE_INLINE void* Next();
	private:
		u32 m_current; //!< Current position
		ueGenericPool* m_pool; //!< Iterated pool
	};

public:
	ueGenericPool();
	~ueGenericPool();

	//! Initializes pool with allocator
	ueBool Init(ueAllocator* allocator, ueSize blockSize, u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Calculates pool memory requirements
	static ueSize CalcMemReq(ueSize blockSize, u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Initializes pool with given memory
	void InitMem(void* memory, ueSize memorySize, ueSize blockSize, u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Deinitializes pool
	void Deinit();

	//! Allocates new element; returns pointer to new element or NULL if failed
	void* Alloc();
	//! Allocates new element; returns index to new element or U32_MAX if failed
	u32 AllocIndex();
	//! Frees element
	void Free(void* block);
	//! Frees element at given index
	void FreeIndex(u32 index);

	//! Gets number of allocated elements
	u32 Size() const;

	//! Removes all elements
	void Clear();

	//! Tells whether pool supports iterator
	ueBool SupportsIterator() const;

	//! Gets total number of blocks (allocated or free)
	u32 GetNumBlocks() const;
	//! Tells whether block at given index is allocated
	ueBool IsBlockAllocated(u32 blockIndex) const;
	//! Gets block at given index
	void* GetBlock(u32 index) const;
	//! Gets index of given block
	u32 GetIndex(const void* block) const;
	//! Tells whether given block is valid (and allocated)
	ueBool IsValidBlock(const void* block) const;
	//! Tells whether block at given index is valid (and allocated)
	ueBool IsValidIndex(u32 index) const;

	//! Gets block size
	ueSize GetBlockSize() const;
	//! Tells whether there's any free block in a pool (i.e. whether Alloc*() would succeed)
	ueBool HasFreeBlock() const;

	//! Gets pointer to internal memory
	void* GetMemory() const;
	//! Gets size of internal memory
	ueSize GetMemorySize() const;

private:
	void* Next(Iterator& iterator) const;

	u32 GetIndex_Unsafe(const void* block) const;

	struct Block
	{
		Block* m_next;
	};

	ueAllocator* m_allocator;
	void* m_memory;
	ueSize m_memorySize;

	ueSize m_blockSize;
	u32 m_numBlocks;

	Block* m_freeBlocks;
	u32 m_numUsedBlocks;

	u32* m_usageMask;

	void SetBlockAllocated(const Block* block);
	void SetBlockFree(const Block* block);

	friend class Iterator;
};

//------------------------- Inline -------------------------

UE_INLINE ueGenericPool::Iterator::Iterator(ueGenericPool& pool) : m_current(U32_MAX), m_pool(&pool)
{
	UE_ASSERT(m_pool->SupportsIterator());
}

UE_INLINE void* ueGenericPool::Iterator::Next()
{
	return m_pool->Next(*this);
}

UE_INLINE ueBool ueGenericPool::SupportsIterator() const
{
	return m_usageMask != NULL;
}

UE_INLINE u32 ueGenericPool::GetNumBlocks() const
{
	return m_numBlocks;
}

UE_INLINE void* ueGenericPool::GetBlock(u32 index) const
{
	UE_ASSERT(index < m_numBlocks);
	return (u8*) m_memory + index * m_blockSize;
}

UE_INLINE u32 ueGenericPool::GetIndex(const void* block) const
{
	UE_ASSERT(IsValidBlock(block));
	const s32 offset = (s32) ((ueSize) block - (ueSize) m_memory);
	const u32 index = (u32) (offset / m_blockSize);
	return index;
}

UE_INLINE u32 ueGenericPool::GetIndex_Unsafe(const void* block) const
{
	const u32 offset = (u32) ((ueSize) block - (ueSize) m_memory);
	const u32 index = (u32) (offset / m_blockSize);
	return index;
}

UE_INLINE ueBool ueGenericPool::IsValidIndex(u32 index) const
{
	return index < m_numBlocks && (!m_usageMask || IsBlockAllocated(index));
}

UE_INLINE ueBool ueGenericPool::IsValidBlock(const void* block) const
{
	const s32 offset = (s32) ((ueSize) block - (ueSize) m_memory);
	const u32 index = (u32) (offset / m_blockSize);
	return 0 <= offset && offset < (s32) m_memorySize &&
		!(offset % m_blockSize) &&
		index < m_numBlocks &&
		(!m_usageMask || IsBlockAllocated(index));
}

UE_INLINE void ueGenericPool::SetBlockAllocated(const Block* block)
{
	const u32 blockIndex = GetIndex_Unsafe(block);
	const u32 int32Index = blockIndex >> 5;
	const u32 int32Bit = blockIndex & 31;
	m_usageMask[int32Index] |= 1 << int32Bit;
}

UE_INLINE void ueGenericPool::SetBlockFree(const Block* block)
{
	const u32 blockIndex = GetIndex_Unsafe(block);
	const u32 int32Index = blockIndex >> 5;
	const u32 int32Bit = blockIndex & 31;
	m_usageMask[int32Index] &= ~(1 << int32Bit);
}

UE_INLINE ueBool ueGenericPool::IsBlockAllocated(u32 blockIndex) const
{
	UE_ASSERT(blockIndex < m_numBlocks);
	UE_ASSERT(m_usageMask);
	const u32 int32Index = blockIndex >> 5;
	const u32 int32Bit = blockIndex & 31;
	return m_usageMask[int32Index] != 0;
}

UE_INLINE ueSize ueGenericPool::GetBlockSize() const
{
	return m_blockSize;
}

UE_INLINE u32 ueGenericPool::Size() const
{
	return m_numUsedBlocks;
}

UE_INLINE ueBool ueGenericPool::HasFreeBlock() const
{
	return m_numUsedBlocks < m_numBlocks;
}

UE_INLINE void* ueGenericPool::GetMemory() const
{
	return m_memory;
}

UE_INLINE ueSize ueGenericPool::GetMemorySize() const
{
	return m_memorySize;
}

// New / delete operators

UE_INLINE void* operator new(size_t size, ueGenericPool& pool)
{
	UE_ASSERT(size == pool.GetBlockSize());
	return pool.Alloc();
}

// This is here just to make compiler happy
UE_INLINE void operator delete(void*, ueGenericPool&)
{
	UE_ASSERT(0);
}

template <class TYPE>
UE_INLINE void ueDelete(TYPE* ptr, ueGenericPool& pool)
{
	ptr->~TYPE();
	return pool.Free(ptr);
}

#endif // UE_GENERIC_POOL_H

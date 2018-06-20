#ifndef UE_POOL_H
#define UE_POOL_H

#include "Base/Containers/ueGenericPool.h"

//! Pool of equally sized elements
template <class TYPE>
class uePool
{
public:
	//! Pool iterator
	class Iterator
	{
		friend class uePool;
	public:
		UE_INLINE Iterator() {}
		//! Initializes iterator for given pool
		UE_INLINE Iterator(uePool& pool) : m_iterator(pool.m_pool) {}
		//! Gets next element in a pool; returns NULL when there's no more elements
		TYPE* Next() { return (TYPE*) m_iterator.Next(); }
	private:
		ueGenericPool::Iterator m_iterator;
	};

	uePool();
	~uePool();

	//! Initializes pool with allocator
	ueBool Init(ueAllocator* allocator, u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Calculates pool memory requirements
	static ueSize CalcMemReq(u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Initializes pool with given memory
	void InitMem(void* memory, ueSize memorySize, u32 numBlocks, ueSize blockAlignment = UE_DEFAULT_ALIGNMENT, ueBool supportIterator = UE_FALSE);
	//! Deinitializes pool
	void Deinit();

	//! Gets number of elements
	u32 Size() const;

	//! Allocates new element; returns pointer to newly allocated element or NULL if failed
	TYPE* Alloc();
	//! Allocates new element; returns index of the newly allocated element or U32_MAX if failed
	u32 AllocIndex();
	//! Frees element
	void Free(TYPE* element);
	//! Frees element at given index
	void FreeIndex(u32 index);

	//! Tells whether pool supports iterator (parameter or the Init*() functions)
	ueBool SupportsIterator() const;

	//! Gets total number of blocks (allocated or not)
	u32 GetNumBlocks() const;
	//! Tells whether block at given index is allocated
	ueBool IsBlockAllocated(u32 blockIndex) const;
	//! Gets element at given index
	TYPE& operator [] (u32 index);
	//! Gets element at given index
	const TYPE& operator [] (u32 index) const;
	//! Tells whether given pointer points to valid allocated block
	ueBool IsValidBlock(const TYPE* block) const;

	//! Gets block size
	ueSize GetBlockSize() const;

	//! Gets internal memory that stores all elements
	void* GetMemory() const;
	//! Gets internal memory size
	ueSize GetMemorySize() const;

private:
	ueGenericPool m_pool; // Internal pool
};

//------------------------- Inline -------------------------

template <class TYPE>
uePool<TYPE>::uePool()
{}

template <class TYPE>
uePool<TYPE>::~uePool()
{
	Deinit();
}

template <class TYPE>
ueBool uePool<TYPE>::Init(ueAllocator* allocator, u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	return m_pool.Init(allocator, sizeof(TYPE), numBlocks, blockAlignment, supportIterator);
}

template <class TYPE>
ueSize uePool<TYPE>::CalcMemReq(u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	return ueGenericPool::CalcMemReq(sizeof(TYPE), numBlocks, blockAlignment, supportIterator);
}

template <class TYPE>
void uePool<TYPE>::InitMem(void* memory, ueSize memorySize, u32 numBlocks, ueSize blockAlignment, ueBool supportIterator)
{
	m_pool.InitMem(memory, memorySize, sizeof(TYPE), numBlocks, blockAlignment, supportIterator);
}

template <class TYPE>
void uePool<TYPE>::Deinit()
{
	UE_ASSERT(Size() == 0);
	m_pool.Deinit();
}

template <class TYPE>
TYPE* uePool<TYPE>::Alloc()
{
	void* ptr = m_pool.Alloc();
	if (!ptr)
		return NULL;
	new(ptr) TYPE;
	return (TYPE*) ptr;
}

template <class TYPE>
void uePool<TYPE>::Free(TYPE* ptr)
{
	UE_ASSERT(IsValidBlock(ptr));
	ptr.~TYPE();
	m_pool.Free(ptr);
}

template <class TYPE>
u32 uePool<TYPE>::AllocIndex()
{
	return m_pool.AllocIndex();
}

template <class TYPE>
void uePool<TYPE>::FreeIndex(u32 index)
{
	m_pool.FreeIndex(index);
}

template <class TYPE>
UE_INLINE ueBool uePool<TYPE>::SupportsIterator() const
{
	return m_pool.SupportsIterator();
}

template <class TYPE>
UE_INLINE u32 uePool<TYPE>::GetNumBlocks() const
{
	return m_pool.GetNumBlocks();
}

template <class TYPE>
UE_INLINE TYPE& uePool<TYPE>::operator [] (u32 index)
{
	return *(TYPE*) m_pool.GetBlock(index);
}

template <class TYPE>
UE_INLINE const TYPE& uePool<TYPE>::operator [] (u32 index) const
{
	return *(const TYPE*) m_pool.GetBlock(index);
}

template <class TYPE>
UE_INLINE ueBool uePool<TYPE>::IsValidBlock(const TYPE* block) const
{
	return m_pool.IsValidBlock(block);
}

template <class TYPE>
UE_INLINE ueBool uePool<TYPE>::IsBlockAllocated(u32 blockIndex) const
{
	return m_pool.IsBlockAllocated(blockIndex);
}

template <class TYPE>
UE_INLINE ueSize uePool<TYPE>::GetBlockSize() const
{
	return sizeof(TYPE);
}

template <class TYPE>
UE_INLINE u32 uePool<TYPE>::Size() const
{
	return m_pool.Size();
}

template <class TYPE>
UE_INLINE void* uePool<TYPE>::GetMemory() const
{
	return m_pool.GetMemory();
}

template <class TYPE>
UE_INLINE ueSize uePool<TYPE>::GetMemorySize() const
{
	return m_pool.GetMemorySize();
}

// New / delete operators

template <class TYPE>
UE_INLINE void* operator new(size_t size, uePool<TYPE>& pool)
{
	UE_ASSERT(size == pool.GetBlockSize());
	return pool.AllocMem();
}

template <class TYPE>
UE_INLINE void ueDelete(TYPE* ptr, uePool<TYPE>& pool)
{
	ptr->~TYPE();
	return pool.FreeMem(ptr);
}

#endif // UE_POOL_H

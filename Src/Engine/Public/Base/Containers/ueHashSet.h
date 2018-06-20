#ifndef UE_HASH_SET_H
#define UE_HASH_SET_H

#include "Base/ueHash.h"
#include "Base/ueCmp.h"
#include "Base/Containers/ueGenericPool.h"

//! Hash set
template <class KEY_TYPE, class KEY_HASH_PRED = ueCalcHashPredicate<KEY_TYPE>, class KEY_CMP_PRED = ueCmpPredicate<KEY_TYPE> >
class ueHashSet
{
public:
	//! Hash set iterator
	class Iterator
	{
	public:
		//! Initializes iterator
		UE_INLINE Iterator(ueHashSet& set) : m_set(set), m_index(~0), m_elem(~0) {}
		//! Gets next element; returns NULL when iteration is finished
		UE_INLINE KEY_TYPE* Next();
	private:
		ueHashSet& m_set;
		u32 m_index;
		u32 m_elem;
	};

	ueHashSet();
	~ueHashSet();

	//! Initializes hash set with allocator
	ueBool Init(ueAllocator* allocator, u32 maxElements, u32 numBuckets = 0);
	//! Calculates hash set memory requirements
	static ueSize CalcMemReq(u32 maxElements, u32 numBuckets = 0);
	//! Initializes hash set with given memory
	void InitMem(void* memory, ueSize memorySize, u32 maxElements, u32 numBuckets = 0);
	//! Deinitializes hash set
	void Deinit();

	//! Removes all elements
	void Clear();

	//! Inserts new element; returns pointer to newly added value; it is safe to use returned memory until element is not removed
	KEY_TYPE* Insert(KEY_TYPE key);
	//! Removes element
	void Remove(KEY_TYPE key);
	//! Finds element equal to given and returns pointer to it or NULL if not found
	KEY_TYPE* Find(KEY_TYPE key);

	//! Gets number of elements
	UE_INLINE u32 Size() const { return m_size; }

	//! Gets hash set statistics
	void GetStats(ueHashContainerStats& stats) const;

private:
	struct Elem
	{
		u32 m_hash;
		u32 m_next;
		u8 m_key[sizeof(KEY_TYPE)];
	};

	ueAllocator* m_allocator;

	u32 m_size;
	u32 m_capacity;
	u32 m_numBuckets;
	u32* m_array;
	Elem* m_elems;
	ueGenericPool m_elemsPool;
};

// Implementation

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::ueHashSet() :
	m_allocator(NULL),
	m_array(NULL)
{}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::~ueHashSet()
{
	Deinit();
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueBool ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Init(ueAllocator* allocator, u32 maxElements, u32 numBuckets)
{
	const ueSize memorySize = CalcMemReq(maxElements, numBuckets);
	void* memory = allocator->Alloc(memorySize);
	if (!memory)
		return UE_FALSE;

	InitMem(memory, memorySize, maxElements, numBuckets);
	m_allocator = allocator;
	return UE_TRUE;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueSize ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::CalcMemReq(u32 maxElements, u32 numBuckets)
{
	numBuckets = (numBuckets == 0) ? maxElements : numBuckets;
	return ueGenericPool::CalcMemReq(sizeof(Elem), maxElements) + sizeof(u32) * numBuckets;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::InitMem(void* memory, ueSize memorySize, u32 maxElements, u32 numBuckets)
{
	UE_ASSERT(memory);
	UE_ASSERT(!m_array);

	numBuckets = (numBuckets == 0) ? maxElements : numBuckets;
	const ueSize bucketsSize = sizeof(u32) * numBuckets;

	m_size = 0;
	m_capacity = maxElements;
	m_numBuckets = numBuckets;

	u8* ptr = (u8*) memory;

	ueSize elemsPoolSize = memorySize - bucketsSize;
	m_elemsPool.InitMem(ptr, elemsPoolSize, sizeof(Elem), m_capacity);
	ptr += elemsPoolSize;
	m_elems = (Elem*) m_elemsPool.GetMemory();

	m_array = (u32*) ptr;
	ptr += bucketsSize;
	for (u32 i = 0; i < m_numBuckets; i++)
		m_array[i] = U32_MAX;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Deinit()
{
	if (!m_array)
		return;

	Clear();

	if (m_allocator)
	{
		m_allocator->Free(m_elems);
		m_allocator = NULL;
	}
	m_elems = NULL;

	m_array = NULL;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
KEY_TYPE* ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Insert(KEY_TYPE key)
{
	UE_ASSERT(m_array);
	UE_ASSERT(m_size < m_capacity);

	const u32 hash = KEY_HASH_PRED()(key);
	const u32 hashMod = hash % m_numBuckets;

	// Search for existing entry

	u32 elemIndex = m_array[hashMod];
	while (elemIndex != U32_MAX)
	{
		if (hash == m_elems[elemIndex].m_hash &&
			!KEY_CMP_PRED()(key, *(KEY_TYPE*) &m_elems[elemIndex].m_key))
			return (KEY_TYPE*) &m_elems[elemIndex].m_key;

		elemIndex = m_elems[elemIndex].m_next;
	}

	// Create new entry

	elemIndex = m_elemsPool.AllocIndex();
	Elem* elem = &m_elems[elemIndex];
	elem->m_hash = hash;
	new(elem->m_key) KEY_TYPE;
	*(KEY_TYPE*) &elem->m_key = key;

	elem->m_next = m_array[hashMod];
	m_array[hashMod] = elemIndex;

	m_size++;
	return (KEY_TYPE*) &elem->m_key;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Remove(KEY_TYPE key)
{
	UE_ASSERT(m_array);
	const u32 hash = KEY_HASH_PRED()(key);
	const u32 hashMod = hash % m_numBuckets;

	// Search for existing entry

	u32 prevElemIndex = U32_MAX;
	u32 elemIndex = m_array[hashMod];
	while (elemIndex != U32_MAX)
	{
		if (hash == m_elems[elemIndex].m_hash &&
			!KEY_CMP_PRED()(key, *(KEY_TYPE*) &m_elems[elemIndex].m_key))
		{
			Elem* elem = &m_elems[elemIndex];

			if (prevElemIndex == U32_MAX)
				m_array[hashMod] = elem->m_next;
			else
				m_elems[prevElemIndex].m_next = elem->m_next;

			((KEY_TYPE*) &elem->m_key)->~KEY_TYPE();
			m_elemsPool.Free(elem);

			m_size--;
			return;
		}

		elemIndex = m_elems[elemIndex].m_next;
	}
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
KEY_TYPE* ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Find(KEY_TYPE key)
{
	UE_ASSERT(m_array);
	const u32 hash = KEY_HASH_PRED()(key);
	const u32 hashMod = hash % m_numBuckets;

	// Search for existing entry

	u32 elemIndex = m_array[hashMod];
	while (elemIndex != U32_MAX)
	{
		if (hash == m_elems[elemIndex].m_hash &&
			!KEY_CMP_PRED()(key, *(KEY_TYPE*) &m_elems[elemIndex].m_key))
			return (KEY_TYPE*) &m_elems[elemIndex].m_key;

		elemIndex = m_elems[elemIndex].m_next;
	}
	return NULL;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Clear()
{
	UE_ASSERT(m_array);

	Iterator i(*this);
	while (KEY_TYPE* key = i.Next())
		key->~KEY_TYPE();
	m_elemsPool.Clear();

	for (u32 i = 0; i < m_numBuckets; i++)
		m_array[i] = U32_MAX;
	m_size = 0;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::GetStats(ueHashContainerStats& stats) const
{
	UE_ASSERT(m_array);
	stats.m_longestList = 0;

	u32 numUsedSlots = 0;
	for (u32 i = 0; i < m_numBuckets; i++)
	{
		numUsedSlots += (m_array[i] != U32_MAX) ? 1 : 0;

		u32 listLength = 0;
		u32 elem = m_array[i];
		while (elem != U32_MAX)
		{
			listLength++;
			elem = m_elems[elem].m_next;
		}
		stats.m_longestList = ueMax(stats.m_longestList, listLength);
	}
	stats.m_numCollisions = m_size - numUsedSlots;
	stats.m_collisionsPercentage = 100.0f * (f32) stats.m_numCollisions / (f32) m_size;
}

template <class KEY_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
KEY_TYPE* ueHashSet<KEY_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Iterator::Next()
{
	if (m_index == m_set.m_numBuckets) return NULL;
	if (m_index == U32_MAX)
	{
		while (++m_index < m_set.m_numBuckets && m_set.m_array[m_index] == U32_MAX);
		if (m_index == m_set.m_numBuckets) return NULL;
		m_elem = m_set.m_array[m_index];
		return (KEY_TYPE*) &m_set.m_elems[m_elem].m_key;
	}

	m_elem = m_set.m_elems[m_elem].m_next;
	if (m_elem == U32_MAX)
	{
		while (++m_index < m_set.m_numBuckets && m_set.m_array[m_index] == U32_MAX);
		if (m_index == m_set.m_numBuckets) return NULL;
		m_elem = m_set.m_array[m_index];
	}
	return (KEY_TYPE*) &m_set.m_elems[m_elem].m_key;
}

#endif // UE_HASH_SET_H

#ifndef UE_HASH_MAP_H
#define UE_HASH_MAP_H

#include "Base/ueHash.h"
#include "Base/ueCmp.h"
#include "Base/Containers/ueGenericPool.h"

//! Hash map
template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED = ueCalcHashPredicate<KEY_TYPE>, class KEY_CMP_PRED = ueCmpPredicate<KEY_TYPE> >
class ueHashMap
{
public:
	class Iterator
	{
	public:
		Iterator(ueHashMap& map) : m_map(map), m_index(~0), m_elem(~0) {}
		ueBool Next();
		KEY_TYPE* Key();
		VALUE_TYPE* Value();
	private:
		ueHashMap& m_map;
		u32 m_index;
		u32 m_elem;
	};

	ueHashMap();
	~ueHashMap();

	//! Initializes container using allocator
	ueBool Init(ueAllocator* allocator, u32 maxElements, u32 numBuckets = 0);
	//! Gets required memory
	static ueSize CalcMemReq(u32 maxElements, u32 numBuckets = 0);
	//! Initializes container using given memory
	void InitMem(void* memory, ueSize memorySize, u32 maxElements, u32 numBuckets = 0);
	//! Deinitializes container
	void Deinit();
	//! Gets whether container has been initialized
	ueBool IsInitialized() const;

	//! Removes all elements
	void Clear();
	//! Gets number of elements
	UE_INLINE u32 Size() const { return m_size; }
	//! Gets capacity (max number of elements)
	UE_INLINE u32 Capacity() const { return m_capacity; }

	//! Inserts new element; returns poinetr to inserted key (this will not memory-move)
	KEY_TYPE* Insert(KEY_TYPE key, const VALUE_TYPE& value);
	//! Removes element at given key; optionally gets value of removed element; returns UE_TRUE if element was removed, UE_FALSE otherwise
	ueBool Remove(KEY_TYPE key, VALUE_TYPE* value = NULL);
	//! Finds value at given key; returns NULL if not found
	VALUE_TYPE* Find(const KEY_TYPE& key);

	//! Gets usage statistics
	void GetStats(ueHashContainerStats& stats) const;

private:
	struct Elem
	{
		u32 m_hash;
		u32 m_next;
		u8 m_key[sizeof(KEY_TYPE)];
		u8 m_value[sizeof(VALUE_TYPE)];
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

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::ueHashMap() :
	m_allocator(NULL),
	m_array(NULL)
{}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::~ueHashMap()
{
	Deinit();
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueBool ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Init(ueAllocator* allocator, u32 maxElements, u32 numBuckets)
{
	const ueSize memorySize = CalcMemReq(maxElements, numBuckets);
	void* memory = allocator->Alloc(memorySize);
	if (!memory)
		return UE_FALSE;

	InitMem(memory, memorySize, maxElements, numBuckets);
	m_allocator = allocator;
	return UE_TRUE;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueSize ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::CalcMemReq(u32 maxElements, u32 numBuckets)
{
	numBuckets = (numBuckets == 0) ? maxElements : numBuckets;
	return ueGenericPool::CalcMemReq(sizeof(Elem), maxElements) + sizeof(u32) * numBuckets;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::InitMem(void* memory, ueSize memorySize, u32 maxElements, u32 numBuckets)
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

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Deinit()
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

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
KEY_TYPE* ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Insert(KEY_TYPE key, const VALUE_TYPE& value)
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
		{
			(*(VALUE_TYPE*) &m_elems[elemIndex].m_value) = value;
			return (KEY_TYPE*) &m_elems[elemIndex].m_key;
		}

		elemIndex = m_elems[elemIndex].m_next;
	}

	// Create new entry

	elemIndex = m_elemsPool.AllocIndex();
	Elem* elem = &m_elems[elemIndex];
	elem->m_hash = hash;
	new(elem->m_key) KEY_TYPE;
	*(KEY_TYPE*) &elem->m_key = key;
	new(elem->m_value) VALUE_TYPE;
	*(VALUE_TYPE*) &elem->m_value = value;

	elem->m_next = m_array[hashMod];
	m_array[hashMod] = elemIndex;

	m_size++;
	return (KEY_TYPE*) &elem->m_key;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueBool ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Remove(KEY_TYPE key, VALUE_TYPE* value)
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

			if (value)
				*value = *(VALUE_TYPE*) &elem->m_value;

			((KEY_TYPE*) &elem->m_key)->~KEY_TYPE();
			((VALUE_TYPE*) &elem->m_value)->~VALUE_TYPE();
			m_elemsPool.Free(elem);

			m_size--;
			return UE_TRUE;
		}

		prevElemIndex = elemIndex;
		elemIndex = m_elems[elemIndex].m_next;
	}
	return UE_FALSE;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
VALUE_TYPE* ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Find(const KEY_TYPE& key)
{
	UE_ASSERT(m_array);
	const u32 hash = KEY_HASH_PRED()(key);
	const u32 hashMod = hash % m_numBuckets;

	// Search for existing entry

	u32 elemIndex = m_array[hashMod];
	while (elemIndex != U32_MAX)
	{
		if (hash == m_elems[elemIndex].m_hash &&
			!KEY_CMP_PRED()(key, *(const KEY_TYPE*) &m_elems[elemIndex].m_key))
			return (VALUE_TYPE*) &m_elems[elemIndex].m_value;

		elemIndex = m_elems[elemIndex].m_next;
	}
	return NULL;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Clear()
{
	UE_ASSERT(m_array);

	Iterator i(*this);
	while (i.Next())
	{
		KEY_TYPE* key = i.Key();
		VALUE_TYPE* value = i.Value();

		key->~KEY_TYPE();
		value->~VALUE_TYPE();
	}
	m_elemsPool.Clear();

	for (u32 i = 0; i < m_numBuckets; i++)
		m_array[i] = U32_MAX;
	m_size = 0;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
void ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::GetStats(ueHashContainerStats& stats) const
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

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
ueBool ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Iterator::Next()
{
	if (m_index == m_map.m_numBuckets) return UE_FALSE;
	if (m_index == U32_MAX)
	{
		while (++m_index < m_map.m_numBuckets && m_map.m_array[m_index] == U32_MAX);
		if (m_index == m_map.m_numBuckets) return UE_FALSE;
		m_elem = m_map.m_array[m_index];
		return UE_TRUE;
	}

	m_elem = m_map.m_elems[m_elem].m_next;
	if (m_elem == U32_MAX)
	{
		while (++m_index < m_map.m_numBuckets && m_map.m_array[m_index] == U32_MAX);
		if (m_index == m_map.m_numBuckets) return UE_FALSE;
		m_elem = m_map.m_array[m_index];
		return UE_TRUE;
	}
	return UE_TRUE;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
KEY_TYPE* ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Iterator::Key()
{
	UE_ASSERT(m_index != U32_MAX && m_index != m_map.m_numBuckets);
	return (KEY_TYPE*) &m_map.m_elems[m_elem].m_key;
}

template <class KEY_TYPE, class VALUE_TYPE, class KEY_HASH_PRED, class KEY_CMP_PRED>
VALUE_TYPE* ueHashMap<KEY_TYPE, VALUE_TYPE, KEY_HASH_PRED, KEY_CMP_PRED>::Iterator::Value()
{
	UE_ASSERT(m_index != U32_MAX && m_index != m_map.m_numBuckets);
	return (VALUE_TYPE*) &m_map.m_elems[m_elem].m_value;
}

#endif // UE_HASH_MAP_H

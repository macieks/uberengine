#ifndef UE_VECTOR_H
#define UE_VECTOR_H

#include "Base/ueBase.h"
#include "Base/ueTypeInfo.h"

//!	Fixed size vector of elements (can only be resized manually)
template <class TYPE>
class ueVector
{
public:
	ueVector();
	~ueVector();

	//! Initializes vector with allocator
	ueBool Init(ueAllocator* allocator, u32 maxElements);
	//! Calculates vector memory requirements
	static ueSize CalcMemReq(u32 maxElements);
	//! Initializes vector with given memory
	void InitMem(void* data, ueSize dataSize, u32 maxElements);
	//! Deinitializes vector
	void Deinit();

	//! Removes all elements
	void Clear();
	//! Resizes vector; only allowed when initialized with allocator
	void Resize(u32 size);
	//! Resizes vector; only allowed when initialized with allocator
	void Resize(u32 size, TYPE value);

	//! Gets number of elements
	UE_INLINE u32 Size() const;
	//! Gets capacity
	UE_INLINE u32 Capacity() const;
	//! Tells whether container is full
	UE_INLINE ueBool IsFull() const;

	//! Gets internal data
	UE_INLINE const TYPE* GetData() const;
	//! Gets internal data
	UE_INLINE TYPE* GetData();

	//! Gets element at given index
	UE_INLINE const TYPE& operator [] (u32 index) const;
	//! Gets element at given index
	UE_INLINE TYPE& operator [] (u32 index);
	//! Gets last element
	UE_INLINE const TYPE& Last() const;
	//! Gets last elements
	UE_INLINE TYPE& Last();

	//! Adds element to the end of vector
	UE_INLINE void PushBack(const TYPE& element);
	//! Adds element to the end of vector; returns pointer to newly added element
	UE_INLINE TYPE* PushBack();
	//! Pops element from the back
	UE_INLINE void PopBack();

	//! Returns first found element equal to given; returns UE_TRUE if such element was found, UE_FALSE otherwise
	ueBool RemoveFirstEqual(const TYPE& element);
	//! Inserts new element at given index; return pointer to newly added element or NULL if insertion failed
	TYPE* InsertAt(u32 index);
	//! Removes element at given index; if exchangeWithLast is set to UE_TRUE, the last element is moved into removed slot, otherwise order of elements is preserved
	void RemoveAt(u32 index, ueBool preserveOrder = UE_TRUE);

	//! Finds element equal to given; returns pointer to found element or NULL if not found
	TYPE* Find(const TYPE& element);

	//! Compares two vectors
	ueBool operator == (const ueVector& other) const;

private:
	ueAllocator* m_allocator;
	u32 m_capacity;
	u32 m_size;
	TYPE* m_data;
};

template <class TYPE>
ueVector<TYPE>::ueVector() :
	m_allocator(NULL),
	m_size(0),
	m_capacity(0),
	m_data(NULL)
{}

template <class TYPE>
ueVector<TYPE>::~ueVector()
{
	Deinit();
}

template <class TYPE>
ueBool ueVector<TYPE>::Init(ueAllocator* allocator, u32 maxElements)
{
	const ueSize memorySize = CalcMemReq(maxElements);
	void* memory = allocator->Alloc(memorySize);
	if (!memory)
		return UE_FALSE;

	m_allocator = allocator;
	InitMem(memory, memorySize, maxElements);
	return UE_TRUE;
}

template <class TYPE>
ueSize ueVector<TYPE>::CalcMemReq(u32 maxElements)
{
	return maxElements * sizeof(TYPE);
}

template <class TYPE>
void ueVector<TYPE>::InitMem(void* memory, ueSize memorySize, u32 maxElements)
{
	UE_ASSERT(!m_data);
	UE_ASSERT(memory);
	UE_ASSERT(memorySize == CalcMemReq(maxElements));

	m_data = (TYPE*) memory;
	m_size = 0;
	m_capacity = maxElements;
}

template <class TYPE>
void ueVector<TYPE>::Deinit()
{
	if (m_data)
	{
		TTypeInfo<TYPE>::DestructArray(m_data, m_size);

		if (m_allocator)
		{
			m_allocator->Free(m_data);
			m_allocator = NULL;
		}

		m_data = NULL;
		m_size = m_capacity = 0;
	}
}

template <class TYPE>
void ueVector<TYPE>::Clear()
{
	TTypeInfo<TYPE>::DestructArray(m_data, m_size);
	m_size = 0;
}

template <class TYPE>
UE_INLINE u32 ueVector<TYPE>::Size() const { return m_size; }

template <class TYPE>
UE_INLINE u32 ueVector<TYPE>::Capacity() const { return m_capacity; }

template <class TYPE>
UE_INLINE ueBool ueVector<TYPE>::IsFull() const { return m_size == m_capacity; }

template <class TYPE>
UE_INLINE const TYPE* ueVector<TYPE>::GetData() const { return m_data; }

template <class TYPE>
UE_INLINE TYPE* ueVector<TYPE>::GetData() { return m_data; }

template <class TYPE>
UE_INLINE const TYPE& ueVector<TYPE>::operator [] (u32 index) const
{
	UE_ASSERT(index < m_size);
	return m_data[index];
}

template <class TYPE>
UE_INLINE TYPE& ueVector<TYPE>::operator [] (u32 index)
{
	UE_ASSERT(index < m_size);
	return m_data[index];
}

template <class TYPE>
UE_INLINE const TYPE& ueVector<TYPE>::Last() const
{
	UE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}

template <class TYPE>
UE_INLINE TYPE& ueVector<TYPE>::Last()
{
	UE_ASSERT(m_size > 0);
	return m_data[m_size - 1];
}

template <class TYPE>
UE_INLINE void ueVector<TYPE>::PushBack(const TYPE& element)
{
	UE_ASSERT(m_size < m_capacity);
	TTypeInfo<TYPE>::CopyConstruct(&m_data[m_size++], &element);
}

template <class TYPE>
UE_INLINE TYPE* ueVector<TYPE>::PushBack()
{
	UE_ASSERT(m_size < m_capacity);
	TYPE* ptr = &m_data[m_size++];
	TTypeInfo<TYPE>::Construct(ptr);
	return ptr;
}

template <class TYPE>
UE_INLINE void ueVector<TYPE>::PopBack()
{
	UE_ASSERT(m_size > 0);
	TTypeInfo<TYPE>::Destruct(&m_data[--m_size]);
}

template <class TYPE>
ueBool ueVector<TYPE>::RemoveFirstEqual(const TYPE& element)
{
	for (u32 i = 0; i < m_size; ++i)
		if (m_data[i] == element)
		{
			--m_size;
			if (i != m_size)
				m_data[i] = m_data[m_size];
			TTypeInfo<TYPE>::Destruct(&m_data[m_size]);
			return UE_TRUE;
		}
	return UE_FALSE;
}

template <class TYPE>
TYPE* ueVector<TYPE>::Find(const TYPE& element)
{
	for (u32 i = 0; i < m_size; ++i)
		if (m_data[i] == element)
			return &m_data[i];
	return NULL;
}

template <class TYPE>
TYPE* ueVector<TYPE>::InsertAt(u32 index)
{
	UE_ASSERT(m_size < m_capacity);
	UE_ASSERT(index <= m_size);
	if (index < m_size)
		TTypeInfo<TYPE>::Move(
			&m_data[index + 1],
			&m_data[index],
			m_size - index);
	TTypeInfo<TYPE>::Construct(&m_data[index]);
	++m_size;
	return &m_data[index];
}

template <class TYPE>
void ueVector<TYPE>::RemoveAt(u32 index, ueBool preserveOrder)
{
	UE_ASSERT(index < m_size);

	// Only move the last element

	if (!preserveOrder)
	{
		if (m_size > 1)
			m_data[index] = m_data[m_size - 1];
		TTypeInfo<TYPE>::Destruct(&m_data[m_size - 1]);
	}

	// Move down all elements after <index>

	else
	{
		TTypeInfo<TYPE>::Destruct(&m_data[index]);
		if (index + 1 < m_size)
			TTypeInfo<TYPE>::Move(
				&m_data[index],
				&m_data[(index + 1)],
				m_size - index - 1);
	}
	--m_size;
}

template <class TYPE>
void ueVector<TYPE>::Resize(u32 size)
{
	UE_ASSERT(size <= m_capacity);
	if (size > m_size)
		TTypeInfo<TYPE>::ConstructArray(&m_data[m_size], size - m_size);
	else if (size < m_size)
		TTypeInfo<TYPE>::DestructArray(&m_data[size], m_size - size);
	m_size = size;
}

template <class TYPE>
void ueVector<TYPE>::Resize(u32 size, TYPE value)
{
	const u32 prevSize = m_size;
	Resize(size);
	for (u32 i = prevSize; i < size; i++)
		m_data[i] = value;
}

template <class TYPE>
ueBool ueVector<TYPE>::operator == (const ueVector& other) const
{
	if (m_size != other.m_size)
		return UE_FALSE;
	for (u32 i = 0; i < m_size; i++)
		if (m_data[i] != other.m_data[i])
			return UE_FALSE;
	return UE_TRUE;
}

#define UE_ALLOC_VECTOR_ON_STACK(varName, elementType, maxCount) \
	u8 varName_##memory[sizeof(elementType) * maxCount]; \
	ueVector<elementType> varName; \
	varName.InitMem(&varName_##memory, sizeof(elementType) * maxCount, maxCount);

#endif // UE_VECTOR_H

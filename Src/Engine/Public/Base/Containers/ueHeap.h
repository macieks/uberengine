#ifndef UE_HEAP_H
#define UE_HEAP_H

//!	Standard heap (priority queue)
template <class TYPE, class CMP_PRED = ueCmpPredicate<TYPE> >
class ueHeap
{
public:
	ueHeap();
	~ueHeap();

	//! Initializes heap with an allocator
	ueBool Init(ueAllocator* allocator, u32 capacity);
	//! Deinitializes heap
	void Deinit();

	//! Removes all elements
	void Clear();

	//! Gets number of elements
	UE_INLINE u32 Size() const { return m_size; }

	//! Adds new value to heap
	void Push(TYPE value);
	//! Pops minimal value from the heap; mustn't be empty
	TYPE PopMin();

private:
	ueAllocator* m_allocator; //!< Allocator used
	TYPE* m_elements; //!< Stored elements
	u32 m_size; //!< Number of elements stored in a heap
	u32 m_capacity; //!< Capacity of the array of elements
};

/*--------------------- ueHeap implementation -------------------------*/

template <class TYPE, class CMP_PRED>
ueHeap<TYPE, CMP_PRED>::ueHeap() :
	m_allocator(NULL),
	m_elements(NULL),
	m_size(0),
	m_capacity(0)
{}

template <class TYPE, class CMP_PRED>
ueHeap<TYPE, CMP_PRED>::~ueHeap()
{
	Deinit();
}

template <class TYPE, class CMP_PRED>
ueBool ueHeap<TYPE, CMP_PRED>::Init(ueAllocator* allocator, u32 capacity)
{
	UE_ASSERT(!m_elements);
	m_allocator = allocator;
	m_capacity = capacity;
	m_size = 0;
	m_elements = ueNewArray<TYPE>(m_allocator, m_capacity);
	return m_elements != NULL;
}

template <class TYPE, class CMP_PRED>
void ueHeap<TYPE, CMP_PRED>::Deinit()
{
	if (!m_elements)
		return;
	ueDeleteArray(m_elements, m_allocator, m_capacity);
	m_elements = NULL;
	m_allocator = NULL;
}

template <class TYPE, class CMP_PRED>
void ueHeap<TYPE, CMP_PRED>::Clear()
{
	UE_ASSERT(m_elements);
	m_size = 0;
}

template <class TYPE, class CMP_PRED>
void ueHeap<TYPE, CMP_PRED>::Push(TYPE value)
{
	UE_ASSERT(m_size < m_capacity);

	++m_size;

	// Pull up

	s32 index = m_size - 1;
	if (index > 0)
	{
		s32 parentIndex = (m_size >> 1) - 1;
		while (CMP_PRED()(value, m_elements[parentIndex]) > 0)
		{
			m_elements[index] = m_elements[parentIndex];

			if (parentIndex == 0) break;

			index = parentIndex;
			parentIndex = ((parentIndex + 1) >> 1) - 1;
		}
	}

	// Set up the new element

	m_elements[index] = value;
}

template <class TYPE, class CMP_PRED>
TYPE ueHeap<TYPE, CMP_PRED>::PopMin()
{
	UE_ASSERT(m_size > 0);

	TYPE result = m_elements[0];

	--m_size;
	if (m_size == 0)
		return result;

	// Move last element to the top

	m_elements[0] = m_elements[m_size];

	// Pull down the top element

	s32 index = 0;
	while (index < (s32) m_size)
	{
		const s32 rightIndex = (index + 1) << 1;
		const s32 leftIndex = rightIndex - 1;

		s32 minIndex = index;
		if (leftIndex < (s32) m_size)
		{
			if (CMP_PRED()(m_elements[leftIndex], m_elements[index]) > 0)
				minIndex = leftIndex;
			if (rightIndex < (s32) m_size && CMP_PRED()(m_elements[rightIndex], m_elements[minIndex]) > 0)
				minIndex = rightIndex;
		}

		if (index == minIndex) break;

		TYPE temp = m_elements[index];
		m_elements[index] = m_elements[minIndex];
		m_elements[minIndex] = temp;

		index = minIndex;
	}

	return result;
}

#endif // UE_HEAP_H

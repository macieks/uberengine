#ifndef UE_LIST_H
#define UE_LIST_H

#include "Base/ueBase.h"

//!	Doubly linked intrusive list
template <class TYPE>
class ueList
{
public:
	//! Base list node class user is supposed to derive from
	class Node
	{
	public:
		UE_INLINE Node() : m_prev(NULL), m_next(NULL) {}
		UE_INLINE ~Node() { UE_ASSERT(!IsInList()); }
		//! Gets next element on the list
		UE_INLINE TYPE* Next() const { return m_next; }
		//! Gets previous element on the list
		UE_INLINE TYPE* Previous() const { return m_prev->m_prev ? m_prev : NULL; }
		//! Tells whether element is on the list
		UE_INLINE ueBool IsInList() const { return m_prev != NULL; }

	private:
		TYPE* m_prev;
		TYPE* m_next;

		friend class ueList;
	};

	UE_INLINE ueList();
	UE_INLINE ~ueList();

	//! Empties list
	void Clear();

	//! Gets list length
	UE_INLINE u32 Length() const;
	//! Tells whether list is empty
	UE_INLINE ueBool IsEmpty() const;

	//! Assigns elements from another list to this list (and so, empties that other list)
	void CopyFrom(ueList& other);
	//! Appends elements of other list to this list (and so, empties that other list)
	void Append(ueList& other);

	//! Gets element from the front of the list
	UE_INLINE TYPE* Front() const;
	//! Gets element from the back of the list
	UE_INLINE TYPE* Back() const;

	//! Pushes element onto the list's front
	UE_INLINE void PushFront(TYPE* element);
	//! Pushes element onto the list's back
	UE_INLINE void PushBack(TYPE* element);

	//! Pops element from the front
	UE_INLINE TYPE* PopFront();
	//! Pops element from the back
	UE_INLINE TYPE* PopBack();
	//! Removes element from the list
	UE_INLINE void Remove(TYPE* node);

	//! Replaces source object (must be on the list) with destination object (mustn't be on the list)
	void Replace(TYPE* src, TYPE* dst);
	//! Inserts new node after given node
	void InsertAfter(TYPE* newNode, TYPE* node);

	//! Checks whether given element is on the list by iterating over all elements
	ueBool IsInListSlow(const TYPE* node) const;

private:
	Node m_head;
	TYPE* m_last;
	u32 m_length;
};

// Implementation
// ---------------------------------

template <class TYPE>
UE_INLINE ueList<TYPE>::ueList() : m_last(NULL), m_length(0) {}

template <class TYPE>
UE_INLINE ueList<TYPE>::~ueList() { UE_ASSERT_MSGP(m_length == 0, "List not empty while destroying (length = %u)", m_length); }

template <class TYPE>
UE_INLINE u32 ueList<TYPE>::Length() const { return m_length; }

template <class TYPE>
void ueList<TYPE>::Clear()
{
	m_head.m_next = m_last = NULL;
	m_length = 0;
}

template <class TYPE>
void ueList<TYPE>::CopyFrom(ueList& other)
{
	UE_ASSERT(IsEmpty());

	m_head = other.m_head;
	if (m_head.m_next)
		m_head.m_next->m_prev = (TYPE*) &m_head;
	m_last = other.m_last;
	m_length = other.m_length;

	// Empty other list

	other.m_head.m_next = NULL;
	other.m_last = NULL;
	other.m_length = 0;
}

template <class TYPE>
void ueList<TYPE>::Append(ueList& other)
{
	if (other.IsEmpty())
		return;

	if (IsEmpty())
		CopyFrom(other);
	else
	{
		TYPE* tail = Back();
		tail->m_next = other.m_head.m_next;
		if (tail->m_next)
		{
			tail->m_next->m_prev = tail;
			m_last = other.m_last;
		}
		m_length += other.m_length;

		// Empty other list

		other.m_head.m_next = NULL;
		other.m_last = NULL;
		other.m_length = 0;
	}
}

template <class TYPE>
UE_INLINE ueBool ueList<TYPE>::IsEmpty() const { return !m_head.m_next; }

template <class TYPE>
TYPE* ueList<TYPE>::Front() const { return m_head.m_next; }

template <class TYPE>
TYPE* ueList<TYPE>::Back() const { return m_last; }

template <class TYPE>
void ueList<TYPE>::PushFront(TYPE* element)
{
	UE_ASSERT(!element->IsInList());

	element->m_prev = (TYPE*) &m_head;
	element->m_next = m_head.m_next;
	if (m_head.m_next) m_head.m_next->m_prev = element;
	m_head.m_next = element;
	if (!m_last) m_last = element;

	m_length++;
}

template <class TYPE>
void ueList<TYPE>::PushBack(TYPE* element)
{
	UE_ASSERT(!element->IsInList());

	TYPE* last = m_last ? m_last : (TYPE*) &m_head;
	last->m_next = element;
	element->m_prev = last;
	element->m_next = NULL;

	m_last = element;
	m_length++;
}

template <class TYPE>
void ueList<TYPE>::Remove(TYPE* node)
{
	if (!node->m_prev && !node->m_next)
		return;

	if (m_last == node)
		m_last = node->m_prev;

	if (node->m_next)
		node->m_next->m_prev = node->m_prev;
	node->m_prev->m_next = node->m_next;

	node->m_prev = node->m_next = NULL;

	if (--m_length == 0)
		m_last = NULL;
}

template <class TYPE>
void ueList<TYPE>::Replace(TYPE* src, TYPE* dst)
{
	dst->m_next = src->m_next;
	if (dst->m_next)
		dst->m_next->m_prev = dst;

	dst->m_prev = src->m_prev;
	dst->m_prev->m_next = dst;

	if (m_last == src)
		m_last = dst;

	src->m_prev = src->m_next = NULL;
}

template <class TYPE>
void ueList<TYPE>::InsertAfter(TYPE* newNode, TYPE* node)
{
	newNode->m_prev = node;
	newNode->m_next = node->m_next;

	if (node->m_next)
		node->m_next->m_prev = newNode;
	node->m_next = newNode;

	if (m_last == node)
		m_last = newNode;

	m_length++;
}

template <class TYPE>
TYPE* ueList<TYPE>::PopFront()
{
	if (!m_head.m_next)
		return NULL;

	TYPE* front = m_head.m_next;
	Remove(front);
	return front;
}

template <class TYPE>
TYPE* ueList<TYPE>::PopBack()
{
	if (!m_last)
		return NULL;

	TYPE* back = m_last;
	Remove(back);
	return back;
}

template <class TYPE>
ueBool ueList<TYPE>::IsInListSlow(const TYPE* node) const
{
	TYPE* curr = Front();
	while (curr && curr != node)
		curr = curr->m_next;
	return curr == node;
}

#endif // UE_LIST_H

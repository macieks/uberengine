#ifndef UE_RED_BLACK_TREE_MAP_H
#define UE_RED_BLACK_TREE_MAP_H

#include "Base/ueCmp.h"
#include "Base/Containers/ueGenericPool.h"

//! Red black tree map (ordered)
template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED = ueCmpPredicate<KEY_TYPE> >
class ueRedBlackTreeMap
{
	class Node;
public:
	//! An iterator that walks map's elements in order as specified by compare predicate
	class Iterator
	{
	public:
		UE_INLINE Iterator();
		//! Starts iteration from given node
		UE_INLINE Iterator(Node* node);
		//! Starts iteration of the map; if there's at least 1 element, an iterator will be set to first valid element
		UE_INLINE Iterator(const ueRedBlackTreeMap& tree);
		//! Tells whether there's valid element being visited currently
		UE_INLINE ueBool Valid() const;
		//! Gets key
		UE_INLINE const KEY_TYPE& Key() const;
		//! Gets value
		UE_INLINE VALUE_TYPE& Value() const;
		//! Moves to the next element
		UE_INLINE void operator ++ ();
		//! Moves to the previous element
		UE_INLINE void operator -- ();

		UE_INLINE void operator = (const Iterator& iterator);

	private:
		Node* m_currNode;

		friend class ueRedBlackTreeMap;
	};

	ueRedBlackTreeMap();
	~ueRedBlackTreeMap();

	//! Initializes map with allocator
	ueBool Init(ueAllocator* allocator, u32 maxElements);
	//! Calculates map's memory requirements
	static ueSize CalcMemReq(u32 maxElements);
	//! Initializes map with given memory
	void InitMem(void* memory, ueSize memorySize, u32 maxElements);
	//! Deinitializes map
	void Deinit();

	//! Removes all elements from map
	void Clear();

	//! Gets number of elements
	UE_INLINE u32 Size() const;

	//! Inserts key,value pair to map
	void Insert(KEY_TYPE key, VALUE_TYPE value);
	//! Removes element with given key from map
	void Remove(KEY_TYPE key);
	//! Removes element pointed by given iterator
	void Remove(Iterator iterator);

	//! Finds element with given key
	Iterator Find(KEY_TYPE key);
	//! Determines lower bound for the key-ordered set
	Iterator LowerBound(KEY_TYPE key);
	//! Determines upper bound for the key-ordered set
	Iterator UpperBound(KEY_TYPE key);

private:

	#define UE_RBT_COLOR_RED		0
	#define UE_RBT_COLOR_BLACK		1
	#define UE_RBT_COLOR_MASK		(UE_RBT_COLOR_BLACK | UE_RBT_COLOR_RED)
	#define UE_RBT_NIL				2

	class Node
	{
	public:
		Node(const KEY_TYPE key, const VALUE_TYPE value, Node* nil, u32 color = UE_RBT_NIL | UE_RBT_COLOR_BLACK)
		{
			m_key = key;
			m_value = value;
			m_color = color;
			m_left = nil;
			m_right = nil;
			m_parent = nil;
		}

		Node(Node* nil, u32 color = UE_RBT_NIL | UE_RBT_COLOR_BLACK)
		{
			m_color = color;
			m_left = nil;
			m_right = nil;
			m_parent = nil;
		}

		UE_INLINE ueBool IsRed() const { return (m_color & UE_RBT_COLOR_MASK) == UE_RBT_COLOR_RED; }
		UE_INLINE ueBool IsBlack() const { return (m_color & UE_RBT_COLOR_MASK) == UE_RBT_COLOR_BLACK; }
		UE_INLINE void SetRed() { m_color &= ~UE_RBT_COLOR_MASK; m_color |= UE_RBT_COLOR_RED; }
		UE_INLINE void SetBlack() { m_color &= ~UE_RBT_COLOR_MASK; m_color |= UE_RBT_COLOR_BLACK; }
		UE_INLINE void CopyColor(const Node* n)
		{
			u32 color = n->m_color;
			color &= UE_RBT_COLOR_MASK;
			m_color &= ~UE_RBT_COLOR_MASK;
			m_color |= color;
		}

		UE_INLINE ueBool IsNil() const { return (m_color & UE_RBT_NIL) != 0; }
		UE_INLINE ueBool NotNil() const { return (m_color & UE_RBT_NIL) == 0; }

		// Rotates subtree left around this node.
		void LeftRotate(Node** root)
		{
			Node* tmp = m_right;
			m_right = tmp->m_left;
			if (tmp->m_left->NotNil())
				tmp->m_left->m_parent = this;
			tmp->m_parent = m_parent;
			if (m_parent->IsNil())
				*root = tmp;
			else if (this == m_parent->m_left)
				m_parent->m_left = tmp;
			else
				m_parent->m_right = tmp;
			tmp->m_left = this;
			m_parent = tmp;
		}

		// Rotates subtree right around this node.
		void RightRotate(Node** root)
		{
			Node* tmp = m_left;
			m_left = tmp->m_right;
			if (tmp->m_right->NotNil())
				tmp->m_right->m_parent = this;
			tmp->m_parent = m_parent;
			if (m_parent->IsNil())
				*root = tmp;
			else if (this == m_parent->m_right)
				m_parent->m_right = tmp;
			else
				m_parent->m_left = tmp;
			tmp->m_right = this;
			m_parent = tmp;
		}

		// Returns minimal node in a subtree of this node. this != m_nil
		Node* ThisMinimal()
		{
			Node* tmp = this;
			while (tmp->m_left->NotNil())
				tmp = tmp->m_left;
			return tmp;
		}

		// Returns maximal node in a subtree of this node. this != m_nil
		Node* ThisMaximal()
		{
			Node* tmp = this;
			while (tmp->m_right->NotNil())
				tmp = tmp->m_right;
			return tmp;
		}

		// Returns Next node in the tree or m_nil-node
		Node* Next()
		{
			Node* tmp1, *tmp2 = this;
			if (m_right->NotNil())
				return m_right->ThisMinimal();
			else
			{
				tmp1 = m_parent;
				while (tmp1->NotNil() && tmp2 == tmp1->m_right)
				{
					tmp2 = tmp1;
					tmp1 = tmp1->m_parent;
				}
				return tmp1;
			}
		}

		// Returns Previous node in the tree or m_nil-node
		Node* Previous()
		{
			Node* tmp1, *tmp2 = this;
			if (m_left->NotNil())
				return m_left->ThisMaximal();
			else
			{
				tmp1 = m_parent;
				while (tmp1->NotNil() && tmp2 == tmp1->m_left)
				{
					tmp2 = tmp1;
					tmp1 = tmp1->m_parent;
				}
				return tmp1;
			}
		}

		KEY_TYPE	m_key;
		VALUE_TYPE	m_value;
		u32		m_color;
		Node*	m_left;
		Node*	m_right;
		Node*	m_parent;
	};

	void RemoveSubtrees(Node* node);
	UE_INLINE void DeleteNode(Node* node);

	Node*	m_root;							//!< Pointer to the root of the tree
	Node	m_nil;							//!< Guardian in the tree
	ueGenericPool m_nodesPool;				//!< Nodes pool

	friend class Iterator;
};

// Implementation

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::ueRedBlackTreeMap() :
	m_nil(&m_nil),
	m_root(&m_nil)
{}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::~ueRedBlackTreeMap()
{
	Deinit();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueBool ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Init(ueAllocator* allocator, u32 maxElements)
{
	return m_nodesPool.Init(allocator, sizeof(Node), maxElements);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueSize ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::CalcMemReq(u32 maxElements)
{
	return ueGenericPool::CalcMemReq(sizeof(Node), maxElements);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::InitMem(void* memory, ueSize memorySize, u32 maxElements)
{
	m_nodesPool.InitMem(memory, memorySize, sizeof(Node), maxElements);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Deinit()
{
	Clear();
	m_nodesPool.Deinit();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE u32 ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Size() const
{
	return m_nodesPool.Size();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Clear()
{
	if (!m_root->NotNil())
		return;
	RemoveSubtrees(m_root);
	DeleteNode(m_root);
	m_root = &m_nil;
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
typename ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Find(KEY_TYPE object)
{
	Node* tmp = m_root;
	while (tmp->NotNil())
		if (!KEY_CMP_PRED()(tmp->m_key, object))
			return Iterator(tmp);
		else if (KEY_CMP_PRED()(tmp->m_key, object) < 0)
			tmp = tmp->m_right;
		else
			tmp = tmp->m_left;
	return Iterator(&m_nil);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
typename ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::LowerBound(KEY_TYPE object)
{
	Node* prev = &m_nil;
	Node* curr = m_root;
	while (curr->NotNil())
		if (KEY_CMP_PRED()(curr->m_key, object) < 0)
			curr = curr->m_right;
		else
		{
			prev = curr;
			curr = curr->m_left;
		}
	return Iterator(prev);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Insert(KEY_TYPE key, VALUE_TYPE value)
{
	Node* tmp = m_root, *tmpParent = &m_nil;
	// insert as a leaf of the tree going down
	while (tmp->NotNil()) {
		tmpParent = tmp;
		if (!KEY_CMP_PRED()(key, tmp->m_key)) {
			tmp->m_value = value;
			return;
		} else if (KEY_CMP_PRED()(key, tmp->m_key) < 0)
			tmp = tmp->m_left;
		else
			tmp = tmp->m_right;
	}; // while
	Node* newNode = new(m_nodesPool) Node(key, value, &m_nil, UE_RBT_COLOR_RED);
	newNode->m_parent = tmpParent;
	if (tmpParent->IsNil())
		m_root = newNode;
	else if (KEY_CMP_PRED()(key, tmpParent->m_key) < 0)
		tmpParent->m_left = newNode;
	else
		tmpParent->m_right = newNode;
	// repair the tree going up
	while (newNode != m_root && newNode->m_parent->IsRed()) {
		if (newNode->m_parent == newNode->m_parent->m_parent->m_left) {
			tmp = newNode->m_parent->m_parent->m_right;
			if (tmp->IsRed()) {
				// only recolouring routine
				newNode->m_parent->SetBlack();
				tmp->SetBlack();
				newNode->m_parent->m_parent->SetRed();
				newNode = newNode->m_parent->m_parent;
			} else {
				if (newNode == newNode->m_parent->m_right) {
					// oops, we need to do rotation
					newNode = newNode->m_parent;
					newNode->LeftRotate(&m_root);
				}; // if
				newNode->m_parent->SetBlack();
				newNode->m_parent->m_parent->SetRed();
				newNode->m_parent->m_parent->RightRotate(&m_root);
			}; // else
		} else {
			tmp = newNode->m_parent->m_parent->m_left;
			if (tmp->IsRed()) {
				// only recolouring routine
				newNode->m_parent->SetBlack();
				tmp->SetBlack();
				newNode->m_parent->m_parent->SetRed();
				newNode = newNode->m_parent->m_parent;
			} else {
				if (newNode == newNode->m_parent->m_left) {
					// oops, we need to do rotation
					newNode = newNode->m_parent;
					newNode->RightRotate(&m_root);
				}; // if
				newNode->m_parent->SetBlack();
				newNode->m_parent->m_parent->SetRed();
				newNode->m_parent->m_parent->LeftRotate(&m_root);
			}; // else
		}; // else
	}; // while
	m_root->SetBlack();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Remove(KEY_TYPE key)
{
	Node* tmp3 = m_root;
	// find node in the tree
	while (tmp3->NotNil())
		if (!KEY_CMP_PRED()(tmp3->m_key, key))
			break;
		else if (KEY_CMP_PRED()(tmp3->m_key, key) < 0)
			tmp3 = tmp3->m_right;
		else
			tmp3 = tmp3->m_left;
	UE_ASSERT(!tmp3->IsNil());
	Remove(Iterator(tmp3));
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Remove(Iterator iter)
{
	UE_ASSERT(iter.Valid());
	Node *tmp1, *tmp2, *tmp3 = iter.m_currNode;
	if (tmp3->m_left->IsNil() || tmp3->m_right->IsNil())
		tmp2 = tmp3;
	else
		tmp2 = tmp3->Next();
	if (tmp2->m_left->NotNil())
		tmp1 = tmp2->m_left;
	else
		tmp1 = tmp2->m_right;
	tmp1->m_parent = tmp2->m_parent;
	if (tmp2->m_parent->IsNil())
		m_root = tmp1;
	else if (tmp2 == tmp2->m_parent->m_left)
		tmp2->m_parent->m_left = tmp1;
	else
		tmp2->m_parent->m_right = tmp1;
	if (tmp2 != tmp3) {
		//delete(tmp3->m_key);
		tmp3->m_key = tmp2->m_key;
		tmp3->m_value = tmp2->m_value;
		//tmp2->m_key = NULL;
	};
	if (tmp2->IsBlack()) {
		// repair tree if needed
		while (tmp1 != m_root && tmp1->IsBlack()) {
			if (tmp1 == tmp1->m_parent->m_left) {
				tmp3 = tmp1->m_parent->m_right;
				if (tmp3->IsRed()) {
					tmp3->SetBlack();
					tmp1->m_parent->SetRed();
					tmp1->m_parent->LeftRotate(&m_root);
					tmp3 = tmp1->m_parent->m_right;
				}; // if
				if (tmp3->m_left->IsBlack() && tmp3->m_right->IsBlack()) {
					tmp3->SetRed();
					tmp1 = tmp1->m_parent;
				} else {
					if (tmp3->m_right->IsBlack()) {
						tmp3->m_left->SetBlack();
						tmp3->SetRed();
						tmp3->RightRotate(&m_root);
						tmp3 = tmp1->m_parent->m_right;
					}; // if
					tmp3->CopyColor(tmp1->m_parent);
					tmp1->m_parent->SetBlack();
					tmp3->m_right->SetBlack();
					tmp1->m_parent->LeftRotate(&m_root);
					tmp1 = m_root;
				}; // else
			} else {
				tmp3 = tmp1->m_parent->m_left;
				if (tmp3->IsRed()) {
					tmp3->SetBlack();
					tmp1->m_parent->SetRed();
					tmp1->m_parent->RightRotate(&m_root);
					tmp3 = tmp1->m_parent->m_left;
				}; // if
				if (tmp3->m_right->IsBlack() && tmp3->m_left->IsBlack()) {
					tmp3->SetRed();
					tmp1 = tmp1->m_parent;
				} else {
					if (tmp3->m_left->IsBlack()) {
						tmp3->m_right->SetBlack();
						tmp3->SetRed();
						tmp3->LeftRotate(&m_root);
						tmp3 = tmp1->m_parent->m_left;
					}; // if
					tmp3->CopyColor(tmp1->m_parent);
					tmp1->m_parent->SetBlack();
					tmp3->m_left->SetBlack();
					tmp1->m_parent->RightRotate(&m_root);
					tmp1 = m_root;
				}; // else
			};
		}; // while
		tmp1->SetBlack();
	}; // if
	DeleteNode(tmp2);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::RemoveSubtrees(Node* node)
{
	if (node->m_left->NotNil())
	{
		RemoveSubtrees(node->m_left);
		DeleteNode(node->m_left);
		node->m_left = &m_nil;
	}
	if (node->m_right->NotNil())
	{
		RemoveSubtrees(node->m_right);
		DeleteNode(node->m_right);
		node->m_right = &m_nil;
	}
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::DeleteNode(Node* node)
{
	node->~Node();
	m_nodesPool.Free(node);
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Iterator() :
	m_currNode(NULL)
{}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Iterator(Node* node) :
	m_currNode(node)
{}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Iterator(const ueRedBlackTreeMap& tree)
{
	m_currNode = tree.m_root;
	if (m_currNode->NotNil())
		m_currNode = m_currNode->ThisMinimal();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE const KEY_TYPE& ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Key() const
{
	UE_ASSERT(m_currNode->NotNil());
	return (const KEY_TYPE&) m_currNode->m_key;
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE VALUE_TYPE& ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Value() const
{
	UE_ASSERT(m_currNode->NotNil());
	return (VALUE_TYPE&) m_currNode->m_value;
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::operator ++ ()
{
	if (m_currNode->NotNil())
		m_currNode = m_currNode->Next();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::operator -- ()
{
	if (m_currNode->NotNil())
		m_currNode = m_currNode->Previous();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE ueBool ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::Valid() const
{
	return m_currNode->NotNil();
}

template<class KEY_TYPE, class VALUE_TYPE, class KEY_CMP_PRED>
UE_INLINE void ueRedBlackTreeMap<KEY_TYPE, VALUE_TYPE, KEY_CMP_PRED>::Iterator::operator = (const Iterator& iterator)
{
	m_currNode = iterator->m_currNode;
}

#endif // UE_RED_BLACK_TREE_MAP_H

#include "Base/Containers/ueABTree.h"

//! User object encapsulation
struct ueABTreeUserObject
{
	ueBox m_box;			//!< Object bounding box
	void* m_userData;		//!< Pointer to user data

	u32 m_nodeId;			//!< Owning node id

	u32 m_prevObjectId;		//!< Previous object id on the list of objects
	u32 m_nextObjectId;		//!< Next object id on the list of objects

	ueBool m_isOnTempList;	//!< Whether the object is on "temp list" (see ueABTreeNode class)
};

//! Tree node representation
struct ueABTreeNode
{
	ueBox m_box;				//!< ueABTreeNode box
	ueVec3 m_boxCenter;			//!< ueABTreeNode box center

	u32 m_parentNodeId;			//!< Id of the parent node

	u32 m_numChildren;			//!< Number of valid child nodes
	u32 m_childIds[6];			//!< Indices to child nodes

	u32 m_firstObjectId;		//!< List of "permanent" objects that wouldn't fit into any of the child nodes
	u32 m_numObjects;			//!< Length of the list of "permanent" objects

	u32 m_firstTempObjectId;	//!< List of objects where each object would fit into some of the child nodes, but it's preferred not to create a child node to avoid deep tree hierarchy
	u32 m_numTempObjects;		//!< Length of the list of "temp objects"
};

//! Currently performed query description
struct ueABTreeQuery
{
	ueFrustum m_frustum;
	ueBox m_box;
	void** m_objects;
	u32 m_numObjects;
	u32 m_maxObjects;
};

struct ueABTree
{
	ueABTreeQuery m_query;

	uePool<ueABTreeNode> m_nodesPool; //!< Pool of the nodes
	uePool<ueABTreeUserObject> m_userObjectsPool; //!< Pool of the user objects

	u32 m_rootNodeId; //!< Id of the root node

#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
	ueABTreeQueryStats m_lastQueryStats;
#endif

	ueABTreeDesc m_desc; //!< Tree description
};

void ueABTree_DumpNode(ueABTree* tree, u32 level, u32 nodeId);

void ueABTree_InternalInsert(ueABTree* tree, u32 newObjectId);
void ueABTree_InternalRemove(ueABTree* tree, u32 objectId);

void ueABTree_InternalFrustumCull(ueABTree* tree, u32 nodeId, u32 planeCullFlags);
void ueABTree_InternalBoxCull(ueABTree* tree, u32 nodeId);
void ueABTree_InternalCullAll(ueABTree* tree, u32 nodeId);

void ueABTree_RemoveFromNode(ueABTree* tree, ueABTreeNode& node, u32 objectId);
void ueABTree_InsertIntoNode(ueABTree* tree, u32 nodeId, u32 objectId, u32 level);

UE_INLINE void ueABTree_CalculateChildBox(const ueVec3& nodeBoxCenter, u32 childIndex, ueBox& nodeBox)
{
	switch (childIndex)
	{
		case 0: nodeBox.m_min[0] = nodeBoxCenter[0]; return;
		case 1: nodeBox.m_max[0] = nodeBoxCenter[0]; return;
		case 2: nodeBox.m_min[1] = nodeBoxCenter[1]; return;
		case 3: nodeBox.m_max[1] = nodeBoxCenter[1]; return;
		case 4: nodeBox.m_min[2] = nodeBoxCenter[2]; return;
		case 5: nodeBox.m_max[2] = nodeBoxCenter[2]; return;
		UE_INVALID_CASE(childIndex);
	}
}

UE_INLINE u32 ueABTree_DetermineChildIndex(const ueVec3& nodeBoxCenter, const ueBox& objectBox, u32 level)
{
	for (u32 i = 0; i < 3; i++)
	{
		// Depending on the tree level the axis is being chosen in turns: 0, 1, 2, 0, 1, 2 and so on
		// for better statistical distribution
		const u32 axis = (i + level) % 3;

		if (nodeBoxCenter[axis] <= objectBox.m_min[axis]) return axis * 2;
		if (objectBox.m_max[axis] <= nodeBoxCenter[axis]) return axis * 2 + 1;
	}

	return U32_MAX;
}

void ueABTreeNode_Reset(ueABTreeNode* node)
{
	node->m_numChildren = 0;
	for (u32 i = 0; i < 6; i++)
		node->m_childIds[i] = U32_MAX;

	node->m_firstObjectId = U32_MAX;
	node->m_numObjects = 0;

	node->m_firstTempObjectId = U32_MAX;
	node->m_numTempObjects = 0;
}

ueABTree* ueABTree_Create(ueABTreeDesc* desc)
{
	UE_ASSERT(desc->m_stackAllocator);
	UE_ASSERT(desc->m_maxNodes > 0 && desc->m_maxObjects > 0);

	const ueSize nodesPoolSize = uePool<ueABTreeNode>::CalcMemReq(desc->m_maxNodes);
	const ueSize userObjectsPoolSize = uePool<ueABTreeUserObject>::CalcMemReq(desc->m_maxObjects);
	const ueSize memoryRequired = sizeof(ueABTree) + nodesPoolSize + userObjectsPoolSize;
	u8* memory = (u8*) desc->m_stackAllocator->Alloc(memoryRequired);
	if (!memory)
		return NULL;

	ueABTree* tree = new(memory) ueABTree();
	memory += sizeof(ueABTree);

	tree->m_desc = *desc;

	tree->m_nodesPool.InitMem(memory, nodesPoolSize, desc->m_maxNodes);
	memory += nodesPoolSize;
	tree->m_userObjectsPool.InitMem(memory, userObjectsPoolSize, desc->m_maxObjects);

	tree->m_rootNodeId = tree->m_nodesPool.AllocIndex();
	ueABTreeNode* root = &tree->m_nodesPool[tree->m_rootNodeId];

	root->m_box = desc->m_spaceBox;
	root->m_box.GetCenter(root->m_boxCenter);
	root->m_parentNodeId = U32_MAX;
	ueABTreeNode_Reset(root);

	return tree;
}

void ueABTree_Destroy(ueABTree* tree)
{
	tree->m_desc.m_stackAllocator->Free(tree);
}

u32 ueABTree_Insert(ueABTree* tree, const ueBox& box, void* userData)
{
	// Create new object to be inserted into the tree

	const u32 newObjectId = tree->m_userObjectsPool.AllocIndex();
	ueABTreeUserObject* newObject = &tree->m_userObjectsPool[newObjectId];
	newObject->m_box = box;
	newObject->m_userData = userData;

	// Actually insert the node

	ueABTree_InternalInsert(tree, newObjectId);

	return newObjectId;
}

void ueABTree_InternalInsert(ueABTree* tree, u32 newObjectId)
{
	// Create new object to be inserted into the tree

	ueABTreeUserObject* newObject = &tree->m_userObjectsPool[newObjectId];

	// Find best node to insert new object to

	u32 level = 0;
	u32 nodeId = tree->m_rootNodeId;
	while (true)
	{
		ueABTreeNode* node = &tree->m_nodesPool[nodeId];

		UE_ASSERT( node->m_box.Contains(newObject->m_box) );

		// Determine which child could shelter the object

		const u32 childIndex = ueABTree_DetermineChildIndex(node->m_boxCenter, newObject->m_box, level);

		// No child node could shelter this object (or we already hit max. depth level)? - add it to this node

		if (level == tree->m_desc.m_maxTreeDepth || childIndex == U32_MAX)
		{
			if (node->m_firstObjectId != U32_MAX)
				tree->m_userObjectsPool[node->m_firstObjectId].m_prevObjectId = newObjectId;
			newObject->m_nodeId = nodeId;
			newObject->m_nextObjectId = node->m_firstObjectId;
			newObject->m_prevObjectId = U32_MAX;
			newObject->m_isOnTempList = UE_FALSE;
			node->m_firstObjectId = newObjectId;
			node->m_numObjects++;
			return;
		}

		// Found a leaf?

		if (node->m_numChildren == 0)
		{
			// If the number of objects in a single node is too high
			// split set of "temp objects" in current node

			if (node->m_numTempObjects < tree->m_desc.m_maxObjectsInNode)
			{
				if (node->m_firstTempObjectId != U32_MAX)
					tree->m_userObjectsPool[node->m_firstTempObjectId].m_prevObjectId = newObjectId;
				newObject->m_nodeId = nodeId;
				newObject->m_nextObjectId = node->m_firstTempObjectId;
				newObject->m_prevObjectId = U32_MAX;
				newObject->m_isOnTempList = true;
				node->m_firstTempObjectId = newObjectId;
				node->m_numTempObjects++;

				UE_ASSERT(U32_MAX != ueABTree_DetermineChildIndex(node->m_boxCenter, newObject->m_box, 0));
				return;
			}

			// Split the node

			u32 tempObjectId = node->m_firstTempObjectId;
			while (tempObjectId != U32_MAX)
			{
				ueABTreeUserObject* tempObject = &tree->m_userObjectsPool[tempObjectId];

				UE_ASSERT( node->m_box.Contains(tempObject->m_box) );

				// Determine child node to put this object to

				const u32 splitNodeIndex = ueABTree_DetermineChildIndex(node->m_boxCenter, tempObject->m_box, level + 1);
				UE_ASSERT(splitNodeIndex != U32_MAX);

				// Create child node if necessary

				ueABTreeNode* childNode = NULL;
				if (node->m_childIds[splitNodeIndex] == U32_MAX)
				{
					// Create new node

					const u32 newNodeId = tree->m_nodesPool.AllocIndex();
					node = &tree->m_nodesPool[nodeId]; // Fix up the node pointer (might become invalid after pool's re-allocation)

					// Set up new node

					ueABTreeNode* newNode = &tree->m_nodesPool[newNodeId];
					ueABTreeNode_Reset(newNode);
					newNode->m_parentNodeId = nodeId;
					newNode->m_box = node->m_box;
					ueABTree_CalculateChildBox(node->m_boxCenter, splitNodeIndex, newNode->m_box);
					newNode->m_box.GetCenter(newNode->m_boxCenter);

					// Update the parent node

					node->m_childIds[splitNodeIndex] = newNodeId;
					node->m_numChildren++;

					childNode = newNode;
				}
				else
					childNode = &tree->m_nodesPool[ node->m_childIds[splitNodeIndex] ];

				// Insert the object onto the list

				const u32 nextObjectId = tempObject->m_nextObjectId;
				ueABTree_InsertIntoNode(tree, node->m_childIds[splitNodeIndex], tempObjectId, level + 1);

				// Go to next object

				tempObjectId = nextObjectId;
			}

			// After splitting there's no objects in the temp list

			node->m_firstTempObjectId = U32_MAX;
			node->m_numTempObjects = 0;
		}

		// Required child node doesn't exist? - create it

		else if (node->m_childIds[childIndex] == U32_MAX)
		{
			// Create new node

			const u32 newNodeId = tree->m_nodesPool.AllocIndex();
			node = &tree->m_nodesPool[nodeId]; // Fix up the node pointer (might become invalid after pool's re-allocation)

			// Set up new node

			ueABTreeNode* newNode = &tree->m_nodesPool[newNodeId];
			ueABTreeNode_Reset(newNode);
			newNode->m_parentNodeId = nodeId;
			newNode->m_box = node->m_box;
			ueABTree_CalculateChildBox(node->m_boxCenter, childIndex, newNode->m_box);
			newNode->m_box.GetCenter(newNode->m_boxCenter);

			// Update the parent node

			node->m_childIds[childIndex] = newNodeId;
			node->m_numChildren++;

			// Recurse into new node

			nodeId = newNodeId;

			UE_ASSERT( newNode->m_box.Contains(newObject->m_box) );
		}

		// Required child node exists - recurse into it

		else
			nodeId = node->m_childIds[childIndex];

		level++;
	}
}

void ueABTree_Update(ueABTree* tree, u32 objectId, const ueBox& newBox)
{
	// Check whether the bounding box changes at all

	ueABTreeUserObject& tempObject = tree->m_userObjectsPool[objectId];
	if (tempObject.m_box == newBox)
		return;

	// Check whether the node still contains new box - if so, there's no need to move the object

	ueABTreeNode& node = tree->m_nodesPool[tempObject.m_nodeId];
	if (node.m_box.Contains(newBox))
	{
		tempObject.m_box = newBox;

		// Make sure after box change the object is still on appropriate list

		const u32 childNodeIndex = ueABTree_DetermineChildIndex(node.m_boxCenter, newBox, 0);
		if ((childNodeIndex != U32_MAX) != tempObject.m_isOnTempList)
		{
			const u32 nodeId = tempObject.m_nodeId;

			// Remove object from one list and insert into another one

			ueABTree_RemoveFromNode(tree, node, objectId);
			const u32 randomLevelIndex = childNodeIndex;
			ueABTree_InsertIntoNode(tree, nodeId, objectId, randomLevelIndex);
		}
		return;
	}

	// Remove and insert the object
	// TODO: This can be optimized - in most cases the object should be moved to sibling node

	ueABTree_InternalRemove(tree, objectId);
	tempObject.m_box = newBox;
	ueABTree_InternalInsert(tree, objectId);
}

void ueABTree_RemoveFromNode(ueABTree* tree, ueABTreeNode& node, u32 objectId)
{
	ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];

	if (object.m_isOnTempList)
		node.m_numTempObjects--;
	else
		node.m_numObjects--;

	// Fix up next object on the list

	if (object.m_nextObjectId != U32_MAX)
		tree->m_userObjectsPool[object.m_nextObjectId].m_prevObjectId = object.m_prevObjectId;

	// Fix up previous object on the list (user object or node)

	if (object.m_prevObjectId != U32_MAX)
		tree->m_userObjectsPool[object.m_prevObjectId].m_nextObjectId = object.m_nextObjectId;
	else
	{
		if (node.m_firstObjectId == objectId)
			node.m_firstObjectId = object.m_nextObjectId;
		else
		{
			UE_ASSERT(node.m_firstTempObjectId == objectId);
			node.m_firstTempObjectId = object.m_nextObjectId;
		}
	}
}

void ueABTree_InsertIntoNode(ueABTree* tree, u32 nodeId, u32 objectId, u32 level)
{
	ueABTreeNode& node = tree->m_nodesPool[nodeId];
	ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];

	// Figure out whether object would fit into any child of the child node

	const u32 childIndex = ueABTree_DetermineChildIndex(node.m_boxCenter, object.m_box, level + 1);

	// Add user object to the child node (either to "temp list" or the other)

	object.m_nodeId = nodeId;
	object.m_prevObjectId = U32_MAX;

	if (childIndex == U32_MAX) // An object doesn't fit into any child's child
	{
		if (node.m_firstObjectId != U32_MAX)
			tree->m_userObjectsPool[node.m_firstObjectId].m_prevObjectId = objectId;

		object.m_nextObjectId = node.m_firstObjectId;
		object.m_isOnTempList = UE_FALSE;
		node.m_firstObjectId = objectId;
		node.m_numObjects++;
	}
	else // An object fits into some child's child
	{
		if (node.m_firstTempObjectId != U32_MAX)
			tree->m_userObjectsPool[node.m_firstTempObjectId].m_prevObjectId = objectId;

		object.m_nextObjectId = node.m_firstTempObjectId;
		object.m_isOnTempList = true;
		node.m_firstTempObjectId = objectId;
		node.m_numTempObjects++;
	}
}

void ueABTree_Remove(ueABTree* tree, u32 objectId)
{
	ueABTree_InternalRemove(tree, objectId);
	tree->m_userObjectsPool.FreeIndex(objectId);
}

void ueABTree_InternalRemove(ueABTree* tree, u32 objectId)
{
	ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];
	u32 nodeId = object.m_nodeId;
	ueABTreeNode* node = &tree->m_nodesPool[nodeId];

	// Remove object from the node's list

	ueABTree_RemoveFromNode(tree, *node, objectId);

	// Remove chain of empty nodes

	while (nodeId != tree->m_rootNodeId && (node->m_numChildren + node->m_numObjects + node->m_numTempObjects == 0))
	{
		const u32 parentNodeId = node->m_parentNodeId;

		// Remove link from parent node

		ueABTreeNode* parentNode = &tree->m_nodesPool[parentNodeId];
		u32 childIndex;
		for (childIndex = 0; childIndex < 6; childIndex++)
			if (parentNode->m_childIds[childIndex] == nodeId)
			{
				parentNode->m_childIds[childIndex] = U32_MAX;
				parentNode->m_numChildren--;
				break;
			}
		UE_ASSERT(childIndex < 6);

		// Remove the node

		tree->m_nodesPool.FreeIndex(nodeId);

		// Move up the tree

		nodeId = parentNodeId;
		node = parentNode;
	}

	// Collapse all siblings if there's too few objects in them

	if (nodeId == tree->m_rootNodeId)
		return;
	while (node->m_parentNodeId != U32_MAX)
	{
		// Count all temp objects under parent node

		ueABTreeNode* parentNode = &tree->m_nodesPool[ node->m_parentNodeId ];
		u32 numTempObjects = parentNode->m_numTempObjects;
		for (u32 i = 0; i < 6; i++)
			if (parentNode->m_childIds[i] != U32_MAX)
			{
				ueABTreeNode* child = &tree->m_nodesPool[ parentNode->m_childIds[i] ];

				// Stop if any child of the parent still has child nodes

				if (child->m_numChildren > 0)
					return;

				numTempObjects += child->m_numTempObjects;
				numTempObjects += child->m_numObjects;

				// Is collapsing still worth doing it?

				if (numTempObjects >= tree->m_desc.m_minObjectsInNode)
					return;
			}

		// Collapse all children

		for (u32 i = 0; i < 6; i++)
			if (parentNode->m_childIds[i] != U32_MAX)
			{
				ueABTreeNode* child = &tree->m_nodesPool[ parentNode->m_childIds[i] ];

				// For each object in child node: move it onto parent's node list

				for (u32 j = 0; j < 2; j++)
				{
					u32 tempObjectId = (j == 0) ? child->m_firstObjectId : child->m_firstTempObjectId;
					while (tempObjectId != U32_MAX)
					{
						ueABTreeUserObject* tempObject = &tree->m_userObjectsPool[tempObjectId];

						const u32 nextObjectId = tempObject->m_nextObjectId;

						const u32 randomLevel = i;
						ueABTree_InsertIntoNode(tree, node->m_parentNodeId, tempObjectId, randomLevel);

						tempObjectId = nextObjectId;
					}
				}

				tree->m_nodesPool.FreeIndex( parentNode->m_childIds[i] );
				parentNode->m_childIds[i] = U32_MAX;
				parentNode->m_numChildren = 0;
			}

		// Go level up
		node = parentNode;
	}
}

void ueABTree_Cull(ueABTree* tree, const ueFrustum& frustum, void** objects, u32 maxObjects, u32& numObjects)
{
#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
	tree->m_lastQueryStats.Reset();
#endif

	tree->m_query.m_frustum = frustum;
	tree->m_query.m_objects = objects;
	tree->m_query.m_maxObjects = maxObjects;
	tree->m_query.m_numObjects = 0;

	ueABTree_InternalFrustumCull(tree, tree->m_rootNodeId, ueFrustum::PlaneFlags);

	numObjects = tree->m_query.m_numObjects;
}

void ueABTree_InternalFrustumCull(ueABTree* tree, u32 nodeId, u32 planeCullFlags)
{
#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
	tree->m_lastQueryStats.m_numNodesVisited++;
#endif

	const ueABTreeNode& node = tree->m_nodesPool[nodeId];

	// Cull all objects in current node

	for (u32 i = 0; i < 2; i++)
	{
		u32 objectId = (i == 0) ? node.m_firstObjectId : node.m_firstTempObjectId;
		while (objectId != U32_MAX && tree->m_query.m_numObjects < tree->m_query.m_maxObjects)
		{
			const ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];

			if (!planeCullFlags || tree->m_query.m_frustum.Intersect(object.m_box, planeCullFlags))
			{
				tree->m_query.m_objects[tree->m_query.m_numObjects++] = object.m_userData;
#if !defined(UE_AB_TREE_GATHER_QUERY_STATS)
			}
#else
				if (!planeCullFlags)
					tree->m_lastQueryStats.m_numTriviallyAcceptedFrustumTests++;
				else
					tree->m_lastQueryStats.m_numFrustumTests++;
			}
			else
				tree->m_lastQueryStats.m_numFrustumTests++;
#endif

			objectId = object.m_nextObjectId;
		}
	}

	// Recurse into child nodes

	if (node.m_numChildren > 0)
		for (u32 i = 0; i < 6; i++)
			if (node.m_childIds[i] != U32_MAX)
			{
				const u32 childNodeId = node.m_childIds[i];
				const ueABTreeNode& childNode = tree->m_nodesPool[ childNodeId ];

				u32 childPlaneCullFlags = planeCullFlags;
				if (tree->m_query.m_frustum.Intersect(childNode.m_box, planeCullFlags, childPlaneCullFlags))
				{
					if (childPlaneCullFlags)
						ueABTree_InternalFrustumCull(tree, childNodeId, childPlaneCullFlags);
					else
						ueABTree_InternalCullAll(tree, childNodeId);
#if !defined(UE_AB_TREE_GATHER_QUERY_STATS)
				}
#else
					if (!planeCullFlags)
						tree->m_lastQueryStats.m_numTriviallyAcceptedFrustumTests++;
					else
						tree->m_lastQueryStats.m_numFrustumTests++;
				}
				else
					tree->m_lastQueryStats.m_numFrustumTests++;
#endif

			}
}

void ueABTree_Cull(ueABTree* tree, const ueBox& box, void** objects, u32 maxObjects, u32& numObjects)
{
#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
	tree->m_lastQueryStats.Reset();
#endif

	tree->m_query.m_box = box;
	tree->m_query.m_objects = objects;
	tree->m_query.m_maxObjects = maxObjects;
	tree->m_query.m_numObjects = 0;

	ueABTree_InternalBoxCull(tree, tree->m_rootNodeId);

	numObjects = tree->m_query.m_numObjects;
}

void ueABTree_InternalBoxCull(ueABTree* tree, u32 nodeId)
{
	const ueABTreeNode& node = tree->m_nodesPool[nodeId];

	// Cull all objects in current node

	for (u32 i = 0; i < 2; i++)
	{
		u32 objectId = (i == 0) ? node.m_firstObjectId : node.m_firstTempObjectId;
		while (objectId != U32_MAX && tree->m_query.m_numObjects < tree->m_query.m_maxObjects)
		{
			ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];

			if (object.m_box.Intersect(tree->m_query.m_box))
				tree->m_query.m_objects[tree->m_query.m_numObjects++] = object.m_userData;

			objectId = object.m_nextObjectId;
		}
	}

	// Recurse into child nodes

	if (node.m_numChildren > 0)
		for (u32 i = 0; i < 6; i++)
			if (node.m_childIds[i] != U32_MAX)
			{
				const u32 childNodeId = node.m_childIds[i];
				const ueABTreeNode& childNode = tree->m_nodesPool[ childNodeId ];

				if (tree->m_query.m_box.Contains(childNode.m_box))
					ueABTree_InternalCullAll(tree, childNodeId);
				else if (childNode.m_box.Intersect(tree->m_query.m_box))
					ueABTree_InternalBoxCull(tree, childNodeId);
			}
}

void ueABTree_InternalCullAll(ueABTree* tree, u32 nodeId)
{
	const ueABTreeNode& node = tree->m_nodesPool[nodeId];

	// Cull all objects in current node

	for (u32 i = 0; i < 2; i++)
	{
		u32 objectId = (i == 0) ? node.m_firstObjectId : node.m_firstTempObjectId;
		while (objectId != U32_MAX && tree->m_query.m_numObjects < tree->m_query.m_maxObjects)
		{
			ueABTreeUserObject& object = tree->m_userObjectsPool[objectId];
			tree->m_query.m_objects[tree->m_query.m_numObjects++] = object.m_userData;
			objectId = object.m_nextObjectId;
		}
	}

	// Recurse into child nodes

	if (node.m_numChildren > 0)
		for (u32 i = 0; i < 6; i++)
			if (node.m_childIds[i] != U32_MAX)
				ueABTree_InternalCullAll(tree, node.m_childIds[i]);
}

void ueABTree_GetStats(ueABTree* tree, ueABTreeStats* stats)
{
	stats->m_numNodes = tree->m_nodesPool.Size();
	stats->m_numObjects = tree->m_userObjectsPool.Size();
}

const ueABTreeDesc* ueABTree_GetDesc(ueABTree* tree)
{
	return &tree->m_desc;
}

#if defined(UE_AB_TREE_GATHER_QUERY_STATS)

const ueABTreeQueryStats* ueABTree_GetLastQueryStats(ueABTree* tree)
{
	return &tree->m_lastQueryStats;
}

#endif

void ueABTree_Dump(ueABTree* tree)
{
	ueABTree_DumpNode(tree, 0, tree->m_rootNodeId);
}

void ueABTree_DumpNode(ueABTree* tree, u32 level, u32 nodeId)
{
	const ueABTreeNode& node = tree->m_nodesPool[nodeId];

	char indentBuffer[64];
	UE_ASSERT(level < UE_ARRAY_SIZE(indentBuffer));
	ueMemSet(indentBuffer, ' ', level);
	indentBuffer[level] = 0;

	ueLogI("%sNODE %d (objs: %d + %d) (%.1f, %.1f, %.1f) -> (%.1f, %.1f, %.1f)",
		indentBuffer,
		nodeId,
		node.m_numObjects, node.m_numTempObjects,
		node.m_box.m_min[0], node.m_box.m_min[1], node.m_box.m_min[2],
		node.m_box.m_max[0], node.m_box.m_max[1], node.m_box.m_max[2]);

	for (u32 i = 0; i < 6; i++)
	{
		if (node.m_childIds[i] == U32_MAX)
			continue;

#if 0
		const ueABTreeNode& childNode = m_nodesPool[ node.m_childIds[i] ];
		UE_ASSERT( node.m_box.Contains(childNode.m_box) );
#endif

		ueABTree_DumpNode(tree, level + 1, node.m_childIds[i]);
	}

#if 0
	// Verify all child objects are really contained in the node's bbox
	for (u32 i = 0; i < 2; i++)
	{
		u32 objectId = i == 0 ? node.m_firstObjectId : node.m_firstTempObjectId;
		while (objectId != U32_MAX)
		{
			const ueABTreeUserObject& object = m_userObjectsPool[objectId];
			UE_ASSERT( node.m_box.Contains(object.m_box) );
			objectId = object.m_nextObjectId;
		}
	}
#endif
}

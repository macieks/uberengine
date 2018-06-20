#ifndef UE_AB_TREE_H
#define UE_AB_TREE_H

#include "Base/Containers/uePool.h"
#include "Base/ueShapes.h"

/**
 * class ueABTree
 *
 *	Adaptive Binary Tree (ABT) with space-evenly-partitioning scheme.
 *
 *	The idea for AB tree is mix of multiple tree concepts:
 *	- splitting along single axis (plane) only at each recursion level - taken from BSP trees
 *	- even space partitioning - taken from Octrees
 *	- rotating split axis at consecutive recursion levels - a'la KD-trees
 *
 *		Details:
 *
 *		In fact the splitting axis is not strictly chosen to be X, Y, Z, X, Y, .... and so on.
 *		The rotation only determines the preference.
 *
 *		Another unusual feature related to this tree is that each node can have up to 6 children.
 *		Two for split along X, two along Y and two along Z axis.
 *		Which ones are actually used depends on the data.
 *
 *	- trying to create as few as possible nodes by not creating full chain of nodes
 *
 *		Details:
 *
 *		With regular octree implementation inserting single zero-extent box would result
 *		in multiple nodes being created - from the root, down to smallest possible leaf.
 *
 *		What happens with AB tree is that the box gets inserted to root node's "temporary list
 *		of objects". This list stores objects that would fit into some of the child nodes, but since there
 *		is not many objects in that node we store the object here preventing "redundant" nodes
 *		from being created.
 *
 *		Once number of objects in root node exceeds fixed number, the root is split into
 *		up to six children. If any of these children still have too too many objects in them, then
 *		they are split again and so on.
 *
 *		As the nodes are removed from the tree, the child nodes get collapsed to bring the tree
 *		back to its good shape. While splitting happens when objects count reaches X, the collapsing
 *		happens when objects count reaches X / 2 (this can be overriden though) which prevents the
 *		tree from constantly being split and collapsed - the cost of these operations is O(n).
 *
 *		All that makes AB tree have much less nodes and eventually making all tree operations
 *		(searching in particular) faster.
 *
 *	Notes: It's up to user to make sure every object id passed in is valid.
 */
struct ueABTree;

//! AB tree description
struct ueABTreeDesc
{
	ueAllocator* m_stackAllocator; //!< Prefarably stack allocator

	ueBox m_spaceBox;		//!< AB-tree space box
	u32 m_maxTreeDepth;		//!< Maximal allowed depth level of the tree
	u32 m_minObjectsInNode;	//!< Minimal recommended number of objects in a node (below that number the neighbour nodes are collapsed into one)
	u32 m_maxObjectsInNode;	//!< Maximal recommended number of objects in a node (above that number the node is split)

	u32 m_maxNodes;			//!< Max. nodes stored
	u32 m_maxObjects;		//!< Max. objects stored

	ueABTreeDesc() :
		m_spaceBox(-1000.0f, -1000.0f, -1000.0f, 1000.0f, 1000.0f, 1000.0f),
		m_maxTreeDepth(10),
		m_minObjectsInNode(10),
		m_maxObjectsInNode(20),
		m_maxNodes(0),
		m_maxObjects(0)
	{}
};

//! AB tree statistics
struct ueABTreeStats
{
	u32 m_numNodes; //!< Number of currently used nodes
	u32 m_numObjects; //!< Number of currently stored objects
};

//! Single query statistics
struct ueABTreeQueryStats
{
	u32 m_numNodesVisited; //!< No. nodes visited during query
	u32 m_numFrustumTests; //!< No. frustum tests done during query
	u32 m_numTriviallyAcceptedFrustumTests; //!< No. trivially accepted frustum tests (the ones which don't need to test against any plane)
	u32 m_numPlanesTest; //!< No. plane tests done during frustum tests (full frustum test involves 6 tests)

	void Reset()
	{
		m_numNodesVisited = 0;
		m_numFrustumTests = 0;
		m_numTriviallyAcceptedFrustumTests = 0;
		m_numPlanesTest = 0;
	}
};

//! Creates AB tree
ueABTree*	ueABTree_Create(ueABTreeDesc* desc);
//! Destroys AB tree
void		ueABTree_Destroy(ueABTree* tree);

//! Inserts object into AB tree; returns inserted object id on success or U32_MAX on failure
u32			ueABTree_Insert(ueABTree* tree, const ueBox& box, void* userData);
//! Updates object's bounding box within AB tree
void		ueABTree_Update(ueABTree* tree, u32 objectId, const ueBox& newBox);
//! Removes object from AB tree
void		ueABTree_Remove(ueABTree* tree, u32 objectId);

//! Culls objects in AB tree within given frustum
void		ueABTree_Cull(ueABTree* tree, const ueFrustum& frustum, void** objects, u32 maxObjects, u32& numObjects);
//! Culls objects in AB tree within given box
void		ueABTree_Cull(ueABTree* tree, const ueBox& box, void** objects, u32 maxObjects, u32& numObjects);

//! Gets tree description
const ueABTreeDesc* ueABTree_GetDesc(ueABTree* tree);
//! Retrieves tree statistics
void		ueABTree_GetStats(ueABTree* tree, ueABTreeStats* stats);
//! Dumps the tree structure to logs
void		ueABTree_Dump(ueABTree* tree);

#if defined(UE_AB_TREE_GATHER_QUERY_STATS)
	//! Retrieves statistics of the last query
	const ueABTreeQueryStats* ueABTree_GetLastQueryStats(ueABTree* tree);
#endif

#endif // UE_AB_TREE_H
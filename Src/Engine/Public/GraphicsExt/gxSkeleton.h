#ifndef GX_SKELETON_H
#define GX_SKELETON_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueMath.h"
#include "Base/Containers/ueList.h"
#include "GraphicsExt/gxAnimation.h"

//! Skeleton node
struct gxSkeletonNode
{
	const char* m_name;				//!< Node name
	u32 m_parentNodeIndex;			//!< Parent node (~0 if no parent)
	ueMat44 m_localTransform;		//!< Local transform
	ueMat44 m_invBindPose;			//!< Inversed global (skeleton's) bind pose transform
};

//! Binding between skeleton and animation
struct gxSkeletonAnimationBinding : ueList<gxSkeletonAnimationBinding>::Node
{
	ueResourceHandle<gxAnimation> m_animation;	//!< References animation
	u32* m_animationNodeIndices;				//!< Mapping: gxSkeleton::m_nodes -> gcAnimation::m_nodes
	u32 m_refCount;
};

//! Skeleton (node hierarchy)
struct gxSkeleton
{
	u32 m_numNodes;						//!< Number of nodes
	gxSkeletonNode* m_nodes;			//!< Nodes stored in DFS order (parents before children)

	ueList<gxSkeletonAnimationBinding> m_bindings;//!< Bindings (auto-managed)

	void DestroyAllBindings();

	gxSkeletonAnimationBinding* GetBinding(gxAnimation* animation);
	void ReleaseBinding(gxSkeletonAnimationBinding* binding);

private:
	gxSkeletonAnimationBinding* AddBinding(gxAnimation* animation);
	void RemoveBinding(gxSkeletonAnimationBinding* binding);
	void RemoveBinding(gxAnimation* animation);
};

// @}

#endif // GX_SKELETON_H

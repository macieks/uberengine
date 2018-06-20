#ifndef GX_MODEL_INSTANCE_PRIVATE_H
#define GX_MODEL_INSTANCE_PRIVATE_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxAnimationInstance.h"
#include "GraphicsExt/gxModelInstance.h"

struct phScene;
struct gxCollisionGeometryInstance;

struct gxModelInstanceLOD
{
	ueBool m_nodeTransformsDirty;	//!< Indicates whether skeleton node transforms are dirty
	ueMat44* m_nodeTransforms;		//!< Skeleton node transforms (m_model->m_LOD[i].m_skeleton->m_nodes)

	gxCollisionGeometryInstance* m_collGeomInstance;	//!< Collision geometry instance
};

//! Model instance
struct gxModelInstance
{
	static const u32 MAX_BONES = 1024;

	ueResourceHandle<gxModel> m_model;	//!< References model

	ueMat44 m_baseTransform;			//!< Base (world) transformation
	gxModelInstanceLOD* m_LODs;			//!< Level of detail instances

	ueList<gxAnimationInstance> m_skeletalAnimationInstances; //!< Set of currently played skeletal animations

	// Static array of per-node weights used to accumulate weights while blending animations
	static f32 m_nodeWeightsAccumulator[MAX_BONES];
};

// @}

#endif // GX_MODEL_INSTANCE_PRIVATE_H

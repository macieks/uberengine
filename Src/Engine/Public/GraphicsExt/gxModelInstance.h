#ifndef GX_MODEL_INSTANCE_H
#define GX_MODEL_INSTANCE_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "GraphicsExt/gxAnimationInstance.h"

struct gxModel;
struct gxAnimation;
struct phScene;

/**
 *	@struct gxModelInstance
 *	@brief Model instance; created via gxModel::CreateInstance()
 */
struct gxModelInstance;

//! Updates model instance
void gxModelInstance_Update(gxModelInstance* mi, f32 dt);
//! Destroys the model instance
void gxModelInstance_Destroy(gxModelInstance* mi);
//! Gets model
gxModel* gxModelInstance_GetModel(gxModelInstance* mi);

// Transformations

//! Sets base transform (model instance transformation)
void gxModelInstance_SetBaseTransform(gxModelInstance* mi, const ueMat44& baseTransform);
//! Gets base transform (model instance transformation)
const ueMat44* gxModelInstance_GetBaseTransform(gxModelInstance* mi);

//! Gets number of nodes
u32 gxModelInstance_GetNumNodes(gxModelInstance* mi, u32 lodIndex);
//! Gets node transformations
const ueMat44* gxModelInstance_GetNodeTransforms(gxModelInstance* mi, u32 lodIndex);
//! Gets skinning node transformations
void gxModelInstance_GetSkinningNodeTransforms(gxModelInstance* mi, u32 lodIndex, u32 numNodes, const u32* nodeIndices, ueMat44* dstArray);

// Skeletal animation

//! Starts playing animation
gxAnimationInstance* gxModelInstance_PlayAnimation(gxModelInstance* mi, gxAnimation* animation, const gxAnimationConfig* config = &gxAnimationConfig::Default);
//! Destroys all animations
void gxModelInstance_DestroyAllAnimations(gxModelInstance* mi);

// Physics

//! Adds model instance to physics scene
void gxModelInstance_AddToScene(gxModelInstance* mi, phScene* scene);
//! Removes model from physics scene
void gxModelInstance_RemoveFromScene(gxModelInstance* mi);

// Rendering

void gxModelInstance_SubmitRenderPackets(gxModelInstance* mi, gxFrame* frame);

// @}

#endif // GX_MODEL_INSTANCE_H

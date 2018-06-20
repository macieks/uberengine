#ifndef GX_ANIMATION_INSTANCE_PRIVATE_H
#define GX_ANIMATION_INSTANCE_PRIVATE_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/Containers/ueList.h"
#include "GraphicsExt/gxAnimation.h"
#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxAnimationInstance.h"

struct gxModelInstance;

struct gxAnimationInstance : public ueList<gxAnimationInstance>::Node
{
	gxModelInstance* m_modelInstance;
	gxAnimation* m_animation;

	struct LOD
	{
		gxSkeletonAnimationBinding* m_binding;
	};
	LOD* m_LODs;

	f32 m_realTime;		//!< Advances proportionally to real-time

	f32 m_time;			//!< Within min and max motion time values
	f32 m_totalTime;	//!< Total animation time

	f32 m_numLoops;		//!< Number of loops to play (doesn't need to be an integer)

	f32 m_weight;
	f32 m_targetWeight;
	f32 m_blendInTime;
	f32 m_blendOutTime;

	f32 m_speed;

	gxAnimationMixMode m_mixMode;
	gxAnimationPlayMode m_playMode;

	gxAnimationInstanceState m_state;
	ueBool m_isPlayingForward;

	ueBool m_destroyWhenStops;
};

gxAnimationInstance* gxAnimationInstance_Create(gxModelInstance* modelInstance, gxAnimation* animation, const gxAnimationConfig* config);
void gxAnimationInstance_Destroy(gxAnimationInstance* ai);

void gxAnimationInstance_Update(gxAnimationInstance* ai, f32 deltaTime);
void gxAnimationInstance_AccumulateBoneTransforms(gxAnimationInstance* ai, u32 lodIndex);

UE_INLINE ueBool gxAnimationInstance_ToBeDestroyed(gxAnimationInstance* ai) { return ai->m_state == gxAnimationInstanceState_Stopped && ai->m_destroyWhenStops; }

// @}

#endif // GX_ANIMATION_INSTANCE_PRIVATE_H

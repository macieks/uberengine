#ifndef GX_ANIMATION_INSTANCE_H
#define GX_ANIMATION_INSTANCE_H

#include "Base/ueBase.h"

/**
 *	@addtogroup gx
 *	@{
 */

struct gxModelInstance;

//! Animation mix modes
enum gxAnimationMixMode
{
	gxAnimationMixMode_Overwrite = 0,	//!< Fade in smoothly overwriting all other animations
	gxAnimationMixMode_Additive,		//!< Add this animation to other ones

	gxAnimationMixMode_MAX
};

//! Animation play modes
enum gxAnimationPlayMode
{
	gxAnimationPlayMode_Forward = 0,	//!< Forward playing - default
	gxAnimationPlayMode_Backward,		//!< Backward playing
	gxAnimationPlayMode_PingPong,		//!< Forward until finished, then backwards

	gxAnimationPlayMode_MAX
};

//! Animation playback configuration
struct gxAnimationConfig
{
	//! Default forward looped animation playback configuration
	static gxAnimationConfig Default;

	gxAnimationMixMode m_mixMode;	//!< Animation mix mode
	gxAnimationPlayMode m_playMode;	//!< Animation play mode

	f32 m_startTimeOffset;			//!< Animation start time offset in seconds
	f32 m_numLoops;					//!< Number of loops to play (not necessarily integer value)

	f32 m_weight;					//!< Animation weight
	f32 m_blendInTime;				//!< Blend in time
	f32 m_blendOutTime;				//!< Blend out time

	f32 m_speed;					//!< Playback speed

	ueBool m_destroyWhenStops;		//!< Indicates whether to destroy the animation when done

	gxAnimationConfig() :
		m_mixMode(gxAnimationMixMode_Overwrite),
		m_playMode(gxAnimationPlayMode_Forward),
		m_startTimeOffset(0),
		m_numLoops(0),
		m_weight(1.0f),
		m_blendInTime(0.3f),
		m_blendOutTime(0.3f),
		m_speed(1.0f),
		m_destroyWhenStops(UE_TRUE)
	{}
};

//! Animation instance states
enum gxAnimationInstanceState
{
	gxAnimationInstanceState_Playing = 0,	//!< Playing now
	gxAnimationInstanceState_Stopping,		//!< Being stopped
	gxAnimationInstanceState_Stopped,		//!< Already stopped

	gxAnimationInstanceState_MAX
};

/**
 *	@struct gxAnimationInstance
 *	@brief Skeletal animation instance.
 */
struct gxAnimationInstance;

//! Stops animation instance
void gxAnimationInstance_Stop(gxAnimationInstance* ai, f32 blendOutTime, ueBool destroyWhenStops);
//! Gets current animation instance time
f32 gxAnimationInstance_GetTime(gxAnimationInstance* ai);
//! Gets current animation instance state
gxAnimationInstanceState gxAnimationInstance_GetState(gxAnimationInstance* ai);

// @}

#endif // GX_ANIMATION_INSTANCE_H

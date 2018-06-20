#ifndef GX_MULTI_POSTPROCESS_H
#define GX_MULTI_POSTPROCESS_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "GraphicsExt/gxPostprocesses.h"

//! Flags for all supported post-processing effects within multi-postprocess
enum gxMultiPostprocessEffect
{
	gxMultiPostprocessEffect_Bloom				= UE_POW2(0),	//!< Bloom effect
	gxMultiPostprocessEffect_DepthOfFieldLQ		= UE_POW2(1),	//!< Low quality depth of field
	gxMultiPostprocessEffect_DepthOfFieldHQ		= UE_POW2(2),	//!< High quality depth of field
	gxMultiPostprocessEffect_OldTV				= UE_POW2(3),	//!< Complex old tv effect
	gxMultiPostprocessEffect_RainyGlass			= UE_POW2(4),	//!< Rain on the glass
	gxMultiPostprocessEffect_CameraMotionBlur	= UE_POW2(5),	//!< Camera based motion blur

	gxMultiPostprocessEffect_All				= gxMultiPostprocessEffect_Bloom |
													gxMultiPostprocessEffect_DepthOfFieldLQ |
													gxMultiPostprocessEffect_DepthOfFieldHQ |
													gxMultiPostprocessEffect_OldTV |
													gxMultiPostprocessEffect_RainyGlass |
													gxMultiPostprocessEffect_CameraMotionBlur,

	gxMultiPostprocessEffect_MAX
};

//! Combined descriptions for multiple post-processing effects
struct gxMultiPostprocessStartupParams
{
	ueAllocator* m_allocator;		//!< Allocator

	u32 m_effectsToStartup;			//!< Bit mask indicating which effects to initialize

	gxPostprocess_Bloom::StartupParams m_bloomParams;

	gxPostprocess_DepthOfField_LowQuality::StartupParams m_dofLQParams;

	gxPostprocess_DepthOfField_HighQuality::StartupParams m_dofHQParams;

	gxPostprocess_OldTV::StartupParams m_oldTVParams;

	gxPostprocess_RainyGlass::StartupParams m_rainyGlassParams;
	gxPostprocess_RainyGlass::InitParams m_rainyGlassInitParams;

	gxPostprocess_CameraMotionBlur::StartupParams m_cameraMotionBlurParams;
	f32 m_cameraMotionBlurPeriodSecs; //!< Motion blurred period length (from now back in the past)

	gxMultiPostprocessStartupParams() :
		m_allocator(NULL),
		m_effectsToStartup(gxMultiPostprocessEffect_All),
		m_cameraMotionBlurPeriodSecs(1.0f / 25.0f)
	{}
};

//! Combined update parameters for multiple post-processing effects
struct gxMultiPostprocessUpdateParams
{
	f32 m_dt;						//!< Delta time
	const ueMat44* m_view;			//!< View transform
	const ueMat44* m_proj;			//!< Projection transform
	gxPostprocess_RainyGlass::UpdateParams m_rainyGlassParams;

	gxMultiPostprocessUpdateParams() :
		m_dt(1.0f / 60.0f),
		m_view(NULL),
		m_proj(NULL)
	{}
};

//! Combined draw parameters for multiple post-processing effects
struct gxMultiPostprocessDrawParams
{
	glCtx* m_ctx;				//!< Context to be used for drawing

	glRenderGroup* m_group;		//!< Render group to be used for drawing
	ueRect* m_dstRect;			//!< Destination rectangle to draw to

	glTextureBuffer* m_color;	//!< Optional color buffer to use
	glTextureBuffer* m_depth;	//!< Optional depth buffer to use
	glTextureBuffer* m_blur;	//!< Optional blur buffer to use

	gxPostprocess_Bloom::DrawParams m_bloomParams;
	gxPostprocess_DepthOfField_LowQuality::DrawParams m_dofLQParams;
	gxPostprocess_DepthOfField_HighQuality::DrawParams m_dofHQParams;
	gxPostprocess_OldTV::DrawParams m_oldTVParams;
	gxPostprocess_RainyGlass::DrawParams m_rainyGlassParams;
	gxPostprocess_CameraMotionBlur::DrawParams m_cameraMotionBlurParams;

	gxMultiPostprocessDrawParams() :
		m_ctx(NULL),
		m_group(NULL),
		m_dstRect(NULL),
		m_color(NULL),
		m_depth(NULL),
		m_blur(NULL)
	{}
};

//! Starts up multi-postprocess
void	gxMultiPostprocess_Startup(gxMultiPostprocessStartupParams* params);
//! Shuts down multi-postprocess
void	gxMultiPostprocess_Shutdown();

//! Updates multi-postprocess
void	gxMultiPostprocess_Update(gxMultiPostprocessUpdateParams* params);

//! Indicates whether color buffer is needed
ueBool	gxMultiPostprocess_NeedsColor();
//! Indicates whether depth buffer is needed
ueBool	gxMultiPostprocess_NeedsDepth();
//! Indicates whether velocity buffer is needed
ueBool	gxMultiPostprocess_NeedsVelocity();
//! Indicates whether there's anything to draw for current multi-postprocess
ueBool	gxMultiPostprocess_WouldDraw();
//! Draw multi-postprocess; possibly does nothing
void	gxMultiPostprocess_Draw(gxMultiPostprocessDrawParams* params);

//! Enables multi-postprocess
void	gxMultiPostprocess_SetEnabled(u32 effectsMask);
//! Gets enabled postprocessing effects within multi-postprocess; returns bit mask containing gxMultiPostprocessEffect_* flags
u32		gxMultiPostprocess_GetEnabled();
//! Gets whether multi-postprocess is enabled
ueBool	gxMultiPostprocess_IsEnabled(u32 effectsMask);

//! Enables specific postprocessing effects (parameter specifies bit mask containing gxMultiPostprocessEffect_* flags)
void	gxMultiPostprocess_Enable(u32 effectsMask);
//! Disables specific postprocessing effects (parameter specifies bit mask containing gxMultiPostprocessEffect_* flags)
void	gxMultiPostprocess_Disable(u32 effectsMask);
//! Toggles on & off specific postprocessing effects (parameter specifies bit mask containing gxMultiPostprocessEffect_* flags)
ueBool	gxMultiPostprocess_ToggleOnOff(u32 effectsMask);

//! Resets motion blur effect; useful after camera teleportation (resets previous transform)
void	gxMultiPostprocess_ResetMotionBlur();
//! Gets whether camera based motion blur effect would draw (only if camera has been moving during certain time)
ueBool	gxMultiPostprocess_WouldCameraMotionDraw();

// @}

#endif // GX_MULTI_POSTPROCESS_H

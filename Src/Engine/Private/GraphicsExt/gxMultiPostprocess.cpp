#include "GraphicsExt/gxMultiPostprocess.h"

#define MAX_TRANSFORM_HISTORY 64

struct gxMultiPostprocessData
{
	ueAllocator* m_allocator;

	u32 m_effectsInitialized;
	u32 m_effectsEnabled;

	// Current state

	gxPostprocess_RainyGlass m_rainyGlass;

	f32 m_oldTVTimer;

	u32 m_firstTransform;
	u32 m_numTransforms;
	ueMat44 m_view[MAX_TRANSFORM_HISTORY];
	f32 m_dt[MAX_TRANSFORM_HISTORY];
	ueMat44 m_proj;

	ueMat44 m_viewProj;
	ueMat44 m_viewProjInv;

	ueMat44 m_prevMotionBlurViewProj;
	f32 m_cameraMotionBlurPeriodSecs;

	// Rendering state

	u32 m_numEffectsLeftToDraw;
	glTextureBuffer* m_color;
};

glTextureBuffer*	gxMultiPostprocess_DrawBlur(gxMultiPostprocessDrawParams* multiParams);
void				gxMultiPostprocess_DrawBloom(gxMultiPostprocessDrawParams* multiParams);
void				gxMultiPostprocess_DrawOldTV(gxMultiPostprocessDrawParams* multiParams);
void				gxMultiPostprocess_DrawDepthOfField(gxMultiPostprocessDrawParams* multiParams);
void				gxMultiPostprocess_DrawRainyGlass(gxMultiPostprocessDrawParams* multiParams);
void				gxMultiPostprocess_DrawCameraMotionBlur(gxMultiPostprocessDrawParams* multiParams);

static gxMultiPostprocessData* s_data = NULL;

void gxMultiPostprocess_Startup(gxMultiPostprocessStartupParams* params)
{
	UE_ASSERT(!s_data);

	s_data = new(params->m_allocator) gxMultiPostprocessData();
	UE_ASSERT(s_data);
	s_data->m_allocator = params->m_allocator;
	s_data->m_effectsEnabled = 0;

	s_data->m_firstTransform = 0;
	s_data->m_numTransforms = 0;

	gxPostprocess_Down2x2::Startup(TempCPtr(gxPostprocess_Down2x2::StartupParams()));
	gxPostprocess_Blur::Startup(TempCPtr(gxPostprocess_Blur::StartupParams()));
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_Bloom)
		gxPostprocess_Bloom::Startup(&params->m_bloomParams);
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_OldTV)
	{
		gxPostprocess_OldTV::Startup(&params->m_oldTVParams);
		s_data->m_oldTVTimer = 0.0f;
	}
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_DepthOfFieldLQ)
		gxPostprocess_DepthOfField_LowQuality::Startup(&params->m_dofLQParams);
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_DepthOfFieldHQ)
		gxPostprocess_DepthOfField_HighQuality::Startup(&params->m_dofHQParams);
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_RainyGlass)
	{
		gxPostprocess_RainyGlass::Startup(&params->m_rainyGlassParams);
		if (!params->m_rainyGlassInitParams.m_allocator)
			params->m_rainyGlassInitParams.m_allocator = params->m_allocator;
		s_data->m_rainyGlass.Init(&params->m_rainyGlassInitParams);
	}
	if (params->m_effectsToStartup & gxMultiPostprocessEffect_CameraMotionBlur)
	{
		gxPostprocess_CameraMotionBlur::Startup(&params->m_cameraMotionBlurParams);
		s_data->m_cameraMotionBlurPeriodSecs = params->m_cameraMotionBlurPeriodSecs;
	}

	s_data->m_effectsInitialized = params->m_effectsToStartup;
}

void gxMultiPostprocess_Shutdown()
{
	UE_ASSERT(s_data);

	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_CameraMotionBlur)
		gxPostprocess_CameraMotionBlur::Shutdown();
	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_RainyGlass)
	{
		s_data->m_rainyGlass.Deinit();
		gxPostprocess_RainyGlass::Shutdown();
	}
	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_DepthOfFieldHQ)
		gxPostprocess_DepthOfField_HighQuality::Shutdown();
	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_DepthOfFieldLQ)
		gxPostprocess_DepthOfField_LowQuality::Shutdown();
	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_OldTV)
		gxPostprocess_OldTV::Shutdown();
	if (s_data->m_effectsInitialized & gxMultiPostprocessEffect_Bloom)
		gxPostprocess_Bloom::Shutdown();
	gxPostprocess_Blur::Shutdown();
	gxPostprocess_Down2x2::Shutdown();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxMultiPostprocess_UpdatePrevMotionBlurTransform()
{
	UE_ASSERT(s_data->m_numTransforms > 0);

#define IND(index) ((s_data->m_firstTransform + (index)) & (MAX_TRANSFORM_HISTORY - 1))

	// If history is empty use current transform as a previous

	if (s_data->m_numTransforms == 1)
		s_data->m_prevMotionBlurViewProj = s_data->m_viewProj;

	// If history isn't empty, attempt to calculate transform from some fixed time ago

	else
	{
		// Find previous transform index

		u32 prevIndex = s_data->m_numTransforms - 1;
		f32 dtAcc = 0.0f;
		f32 lastDT = 0.0f;
		while (prevIndex > 0 && dtAcc < s_data->m_cameraMotionBlurPeriodSecs)
		{
			const f32 dt = s_data->m_dt[IND(prevIndex)];

			dtAcc += dt;
			lastDT = dt;

			prevIndex--;
		}

		ueMat44 view;

		// Not found? - simply use last one

		if (dtAcc <= s_data->m_cameraMotionBlurPeriodSecs)
			view = s_data->m_view[IND(0)];

		// Found!

		else
			ueMat44::Lerp(
				view,
				s_data->m_view[IND(prevIndex)],
				s_data->m_view[IND(prevIndex + 1)],
				(dtAcc - s_data->m_cameraMotionBlurPeriodSecs) / lastDT);

		ueMat44::Mul(s_data->m_prevMotionBlurViewProj, view, s_data->m_proj);
	}
}

void gxMultiPostprocess_Update(gxMultiPostprocessUpdateParams* params)
{
	// Update transforms

	const u32 transformIndex = (s_data->m_firstTransform + s_data->m_numTransforms) & (MAX_TRANSFORM_HISTORY - 1);

	s_data->m_view[transformIndex] = *params->m_view;
	s_data->m_dt[transformIndex] = params->m_dt;
	if (s_data->m_numTransforms < MAX_TRANSFORM_HISTORY)
		s_data->m_numTransforms++;
	else
		s_data->m_firstTransform = (s_data->m_firstTransform + 1) & (MAX_TRANSFORM_HISTORY - 1);

	s_data->m_proj = *params->m_proj;
	ueMat44::Mul(s_data->m_viewProj, *params->m_view, *params->m_proj);
	ueMat44::Invert(s_data->m_viewProjInv, s_data->m_viewProj);

	// Update motion blur transform

	gxMultiPostprocess_UpdatePrevMotionBlurTransform();

	// Update old tv

	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_OldTV)
	{
		const f32 effectLoopTime = 60.0f * 10.0f;
		s_data->m_oldTVTimer = ueMod(s_data->m_oldTVTimer + params->m_dt, effectLoopTime);
	}

	// Update rain

	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_RainyGlass)
	{
		s_data->m_rainyGlass.SetUpdateParams(&params->m_rainyGlassParams);
		s_data->m_rainyGlass.Update(params->m_dt);
	}
}

ueBool gxMultiPostprocess_NeedsColor()
{
	u32 colorEffectsEnabled = s_data->m_effectsEnabled;

//	colorEffectsEnabled &= ~gxMultiPostprocessEffect_SSAO;

	if (!gxMultiPostprocess_WouldCameraMotionDraw())
		colorEffectsEnabled &= ~gxMultiPostprocessEffect_CameraMotionBlur;

	return colorEffectsEnabled != 0;
}

ueBool gxMultiPostprocess_NeedsDepth()
{
	if (gxMultiPostprocess_WouldCameraMotionDraw())
		return UE_TRUE;

	return (s_data->m_effectsEnabled &
		(gxMultiPostprocessEffect_DepthOfFieldLQ |
		gxMultiPostprocessEffect_DepthOfFieldHQ)) != 0;
}

ueBool gxMultiPostprocess_NeedsBlur()
{
	return (s_data->m_effectsEnabled & (gxMultiPostprocessEffect_Bloom | gxMultiPostprocessEffect_DepthOfFieldLQ)) != 0;
}

ueBool gxMultiPostprocess_WouldDraw()
{
	if (s_data->m_effectsEnabled == gxMultiPostprocessEffect_CameraMotionBlur)
		return s_data->m_viewProj != s_data->m_prevMotionBlurViewProj;
	return s_data->m_effectsEnabled != 0;
}

ueBool gxMultiPostprocess_WouldCameraMotionDraw()
{
	return
		(s_data->m_effectsEnabled & gxMultiPostprocessEffect_CameraMotionBlur) &&
		s_data->m_viewProj != s_data->m_prevMotionBlurViewProj;
}

void gxMultiPostprocess_Draw(gxMultiPostprocessDrawParams* params)
{
	UE_ASSERT(gxMultiPostprocess_WouldDraw());

	s_data->m_color = params->m_color;

	// Generate blur if needed

	glTextureBuffer* generatedBlur = NULL;
	if (gxMultiPostprocess_NeedsBlur() && !params->m_blur)
		params->m_blur = generatedBlur = gxMultiPostprocess_DrawBlur(params);

	// Calculate number of post-processes to draw

	s_data->m_numEffectsLeftToDraw = ueCountBits(s_data->m_effectsEnabled);
	if ((s_data->m_effectsEnabled & gxMultiPostprocessEffect_CameraMotionBlur) && !gxMultiPostprocess_WouldCameraMotionDraw())
		s_data->m_numEffectsLeftToDraw--;

	// Draw

	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_Bloom)
		gxMultiPostprocess_DrawBloom(params);
	if ((s_data->m_effectsEnabled & gxMultiPostprocessEffect_CameraMotionBlur) && gxMultiPostprocess_WouldCameraMotionDraw())
		gxMultiPostprocess_DrawCameraMotionBlur(params);
	if (s_data->m_effectsEnabled & (gxMultiPostprocessEffect_DepthOfFieldLQ | gxMultiPostprocessEffect_DepthOfFieldHQ))
		gxMultiPostprocess_DrawDepthOfField(params);
	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_OldTV)
		gxMultiPostprocess_DrawOldTV(params);
	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_RainyGlass)
		gxMultiPostprocess_DrawRainyGlass(params);

	// Release generated blur

	if (generatedBlur)
	{
		glRenderBufferPool_ReleaseTextureBuffer(generatedBlur);
		params->m_blur = NULL;
	}
}

void gxMultiPostprocess_SetEnabled(u32 effectsMask)
{
	UE_ASSERT((effectsMask & ~s_data->m_effectsInitialized) == 0);
	s_data->m_effectsEnabled = effectsMask;
}

u32 gxMultiPostprocess_GetEnabled()
{
	return s_data->m_effectsEnabled;
}

ueBool gxMultiPostprocess_IsEnabled(u32 effectsMask)
{
	return (s_data->m_effectsEnabled & effectsMask) == effectsMask;
}

ueBool gxMultiPostprocess_ToggleOnOff(u32 effectsMask)
{
	UE_ASSERT((effectsMask & ~s_data->m_effectsInitialized) == 0);
	if (gxMultiPostprocess_IsEnabled(effectsMask))
	{
		gxMultiPostprocess_Disable(effectsMask);
		return UE_FALSE;
	}
	else
	{
		gxMultiPostprocess_Enable(effectsMask);
		return UE_TRUE;
	}
}

void gxMultiPostprocess_ResetMotionBlur()
{
	s_data->m_numTransforms = 0;
}

void gxMultiPostprocess_Enable(u32 effectsMask)
{
	UE_ASSERT((effectsMask & ~s_data->m_effectsInitialized) == 0);
	s_data->m_effectsEnabled |= effectsMask;
}

void gxMultiPostprocess_Disable(u32 effectsMask)
{
	UE_ASSERT((effectsMask & ~s_data->m_effectsInitialized) == 0);
	s_data->m_effectsEnabled &= ~effectsMask;
}

// Effects

glTextureBuffer* gxMultiPostprocess_DrawBlur(gxMultiPostprocessDrawParams* multiParams)
{
	UE_PROF_SCOPE("post-process blur generation");

	const u32 numHalfsizeDownscalesForBlur = 2;
	const u32 numBlurIterations = 1;
	const f32 blurKernel = 1.0f;

	// Downscale

	glTextureBuffer* color = s_data->m_color;

	for (u32 i = 0; i < numHalfsizeDownscalesForBlur; i++)
	{
		gxPostprocess_Down2x2::DrawParams down2x2Params;
		down2x2Params.m_ctx = multiParams->m_ctx;
		down2x2Params.m_srcColor = color;

		gxPostprocess_Down2x2::Draw(&down2x2Params);

		if (color != s_data->m_color) // Do not release input buffer
			glRenderBufferPool_ReleaseTextureBuffer(color);
		color = down2x2Params.m_dstColor;
	}

	// Blur

	for (u32 i = 0; i < numBlurIterations * 2; i++)
	{
		gxPostprocess_Blur::DrawParams blurParams;
		blurParams.m_ctx = multiParams->m_ctx;
		blurParams.m_srcColor = color;
		blurParams.m_blurKernel = blurKernel;
		blurParams.m_type = (i & 1) ? gxPostprocess_Blur::DrawParams::Type_Vertical : gxPostprocess_Blur::DrawParams::Type_Horizontal;

		gxPostprocess_Blur::Draw(&blurParams);

		glRenderBufferPool_ReleaseTextureBuffer(color);
		color = blurParams.m_dstColor;
	}

	return color;
}

void gxMultiPostprocess_DrawBloom(gxMultiPostprocessDrawParams* multiParams)
{
	// Set up params

	gxPostprocess_Bloom::DrawParams& params = multiParams->m_bloomParams;
	params.m_ctx = multiParams->m_ctx;
	if (s_data->m_numEffectsLeftToDraw == 1)
	{
		params.m_group = multiParams->m_group;
		params.m_dstRect = multiParams->m_dstRect;
	}
	params.m_srcColor = s_data->m_color;
	params.m_srcBlur = multiParams->m_blur;

	// Draw

	gxPostprocess_Bloom::Draw(&params);

	// Get result

	glRenderBufferPool_ReleaseTextureBuffer(s_data->m_color);
	s_data->m_color = params.m_dstColor;

	s_data->m_numEffectsLeftToDraw--;
}

void gxMultiPostprocess_DrawOldTV(gxMultiPostprocessDrawParams* multiParams)
{
	// Set up params

	gxPostprocess_OldTV::DrawParams& params = multiParams->m_oldTVParams;
	params.m_ctx = multiParams->m_ctx;
	if (s_data->m_numEffectsLeftToDraw == 1)
		params.m_group = multiParams->m_group;
	params.m_srcColor = s_data->m_color;
	params.m_time = s_data->m_oldTVTimer;

	// Draw

	gxPostprocess_OldTV::Draw(&params);

	// Get result

	glRenderBufferPool_ReleaseTextureBuffer(s_data->m_color);
	s_data->m_color = params.m_dstColor;

	s_data->m_numEffectsLeftToDraw--;
}

void gxMultiPostprocess_DrawDepthOfField(gxMultiPostprocessDrawParams* multiParams)
{
	// Set up params

	gxPostprocess_DepthOfField_LowQuality::DrawParams& lqParams = multiParams->m_dofLQParams;
	gxPostprocess_DepthOfField_HighQuality::DrawParams& hqParams = multiParams->m_dofHQParams;
	gxPostprocess_DepthOfField_Base::DrawParams& params =
		(s_data->m_effectsEnabled & gxMultiPostprocessEffect_DepthOfFieldHQ) ?
		*(gxPostprocess_DepthOfField_Base::DrawParams*) &hqParams :
		*(gxPostprocess_DepthOfField_Base::DrawParams*) &lqParams;

	params.m_ctx = multiParams->m_ctx;
	if (s_data->m_numEffectsLeftToDraw == 1)
		params.m_group = multiParams->m_group;
	params.m_srcColor = s_data->m_color;
	params.m_srcDepth = multiParams->m_depth;

	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_DepthOfFieldLQ)
		lqParams.m_srcBlur = multiParams->m_blur;

	// Draw

	if (s_data->m_effectsEnabled & gxMultiPostprocessEffect_DepthOfFieldHQ)
		gxPostprocess_DepthOfField_HighQuality::Draw(&hqParams);
	else
		gxPostprocess_DepthOfField_LowQuality::Draw(&lqParams);

	// Get result

	glRenderBufferPool_ReleaseTextureBuffer(s_data->m_color);
	s_data->m_color = params.m_dstColor;

	s_data->m_numEffectsLeftToDraw--;
}

void gxMultiPostprocess_DrawRainyGlass(gxMultiPostprocessDrawParams* multiParams)
{
	// Set up params

	gxPostprocess_RainyGlass::DrawParams& params = multiParams->m_rainyGlassParams;
	params.m_ctx = multiParams->m_ctx;
	if (s_data->m_numEffectsLeftToDraw == 1)
		params.m_group = multiParams->m_group;
	params.m_srcColor = s_data->m_color;

	// Draw

	s_data->m_rainyGlass.Draw(&params);

	// Get result

	glRenderBufferPool_ReleaseTextureBuffer(s_data->m_color);
	s_data->m_color = params.m_dstColor;

	s_data->m_numEffectsLeftToDraw--;
}

void gxMultiPostprocess_DrawCameraMotionBlur(gxMultiPostprocessDrawParams* multiParams)
{
	// Set up params

	gxPostprocess_CameraMotionBlur::DrawParams& params = multiParams->m_cameraMotionBlurParams;
	params.m_ctx = multiParams->m_ctx;
	if (s_data->m_numEffectsLeftToDraw == 1)
		params.m_group = multiParams->m_group;
	params.m_srcColor = s_data->m_color;
	params.m_srcDepth = multiParams->m_depth;
	params.m_viewProjInv = &s_data->m_viewProjInv;
	params.m_prevViewProj = &s_data->m_prevMotionBlurViewProj;

	// Draw

	gxPostprocess_CameraMotionBlur::Draw(&params);

	// Get result

	glRenderBufferPool_ReleaseTextureBuffer(s_data->m_color);
	s_data->m_color = params.m_dstColor;

	s_data->m_numEffectsLeftToDraw--;
}

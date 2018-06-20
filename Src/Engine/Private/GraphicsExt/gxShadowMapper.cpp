#include "GraphicsExt/gxShadowMapper.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

gxShadowMapper::gxShadowMapper() :
	m_isRendering(UE_FALSE)
{
}

gxShadowMapper::~gxShadowMapper()
{
	UE_ASSERT(!m_isRendering);
}

ueBool gxShadowMapper::DrawShadowMask(glCtx* context, const LightDesc& light, const CameraDesc& camera, const ShadowMethodDesc& methodDesc, ExtraData& extraData)
{
	UE_ASSERT(!m_isRendering);
	UE_ASSERT(extraData.m_drawDepthFunction && extraData.m_drawShadowFunction);

	m_isRendering = UE_TRUE;

	// Copy parameters
	m_ctx = context;
	m_light = light;
	m_camera = camera;
	m_methodDesc = methodDesc;
	m_extraData = extraData;

	// Check whether hardware can handle multiple splits with full pass per split method

#if defined(GL_D3D9) || defined(GL_OPENGL)
	const ueBool hasTextureRenderTargets = UE_TRUE;
#else
	const ueBool hasTextureRenderTargets = UE_FALSE;
#endif

	if (!hasTextureRenderTargets &&
		m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit &&
		m_methodDesc.m_numShadowSplits > 1)
	{
		ueLogD("Can't render shadow maps to shadow mask one after another: hardware requires render target resolve.\n");
		m_methodDesc.m_drawOrder = DrawOrder_AllShadowMapsFirst_ShadowMaskPassPerSplit;
	}

	// Handle constraints for omni directional lights
	if (m_light.m_lightProjection == LightProjection_OmniCube || m_light.m_lightProjection == LightProjection_OmniDualParaboloidal)
	{
		// No support for multiple splits
		if (m_methodDesc.m_numShadowSplits > 1)
		{
			ueLogD("Omni directional lights can't use multiple shadow splits.\n");
			m_methodDesc.m_numShadowSplits = 1;
		}

		// Force the only supported draw order
		m_methodDesc.m_drawOrder = DrawOrder_AllShadowMapsFirst_SingleShadowMaskPass;

		// No support for non-standard warp method
		if (m_methodDesc.m_warpMethod != WarpMethod_Standard)
		{
			ueLogD("Omni directional lights can't use any other than standard shadow warp method.\n");
			m_methodDesc.m_warpMethod = WarpMethod_Standard;
		}
	}

	// Check whether to use depth texture (only if hardware allows)
	m_useDepthTexture = m_methodDesc.m_hintUseDepthTexture && glDevice_GetCaps()->m_formatInfos[glBufferFormat_D24S8].m_supportedAsRTAndTexture;

	// Pre-calculate necessary stuff
	m_cameraProj.SetPerspectiveFov(m_camera.m_FOV, m_camera.m_aspect, m_camera.m_nearPlane, m_camera.m_farPlane, UE_TRUE);
	ueMat44::Mul(m_cameraViewProj, m_camera.m_view, m_cameraProj);

	ueMat44 cameraViewInv;
	ueMat44::Invert(cameraViewInv, m_camera.m_view);
	m_cameraPosition = cameraViewInv.GetTranslation();

	m_cameraViewDir = -m_camera.m_view.GetFrontVec();

	// Determine splitting information

	if (!CalculateDrawPassesInfo())
		return UE_FALSE;

	// Create shadow mask render target groups

	glRenderGroupDesc shadowMaskDesc;
	shadowMaskDesc.m_width = m_methodDesc.m_shadowMaskWidth;
	shadowMaskDesc.m_height = m_methodDesc.m_shadowMaskHeight;
	shadowMaskDesc.m_needColorContent = UE_TRUE;
	shadowMaskDesc.m_numColorBuffers = 1;
	shadowMaskDesc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
	if (!m_methodDesc.m_sampleSceneDepth)
	{
		shadowMaskDesc.m_hasDepthStencil = UE_TRUE;
		if (m_extraData.m_sceneDepthTexture)
		{
			UE_ASSERT(UE_FALSE);
			// FIXME: Set this buffer before each shadow mask pass
			//shadowMaskDesc.m_buffer = m_extraData.m_sceneDepthTexture; // Reuse existing depth
		}
		else
			shadowMaskDesc.m_depthStencilDesc.m_format = glBufferFormat_D24S8;
	}
	m_shadowMaskGroup = glRenderGroup_Create(&shadowMaskDesc);
	UE_ASSERT(m_shadowMaskGroup);

	// Create shadow map render target groups
	glRenderGroupDesc shadowMapDesc;
	shadowMapDesc.m_width = m_methodDesc.m_shadowMapSize;
	shadowMapDesc.m_height = m_methodDesc.m_shadowMapSize;
	shadowMapDesc.m_hasDepthStencil = UE_TRUE;
	shadowMapDesc.m_depthStencilDesc.m_format = glBufferFormat_D24S8;

	if (m_useDepthTexture)
		shadowMapDesc.m_needDepthStencilContent = UE_TRUE;
	else
	{
		shadowMapDesc.m_needColorContent = UE_TRUE;
		shadowMapDesc.m_numColorBuffers = 1;
		shadowMapDesc.m_colorDesc[0].m_format = glBufferFormat_R32F;
	}

	switch (m_light.m_lightProjection)
	{
		case LightProjection_Directional:
			m_numShadowMapPasses = m_methodDesc.m_numShadowSplits;
			m_numShadowMaps = m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit ? 1 : m_methodDesc.m_numShadowSplits;
			break;
		case LightProjection_Spot:
			m_numShadowMapPasses = m_methodDesc.m_numShadowSplits;
			m_numShadowMaps = m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit ? 1 : m_methodDesc.m_numShadowSplits;
			break;
		case LightProjection_OmniCube:
			m_numShadowMapPasses = 6;
			m_numShadowMaps = 1;
			UE_ASSERT(UE_FALSE);
//			shadowMapDesc.m_isCube = UE_TRUE;
			break;
		case LightProjection_OmniDualParaboloidal:
			m_numShadowMapPasses = 2;
			m_numShadowMaps = 2;
			break;
	}

	m_shadowMapGroup = glRenderGroup_Create(&shadowMapDesc);
	UE_ASSERT(m_shadowMapGroup);

	// Generate scene depth texture if necessary
	ueBool generatedSceneDepth = UE_FALSE;
	if (m_methodDesc.m_sampleSceneDepth)
	{
		if (m_extraData.m_sceneDepthTexture)
			m_sceneDepthTexture = m_extraData.m_sceneDepthTexture;
		else
		{
			m_sceneDepthTexture = DrawSceneDepth();
			generatedSceneDepth = UE_TRUE;
		}
	}

	// Create variance shadow map if needed
	if (m_methodDesc.m_enableVarianceShadows)
	{
		glRenderGroupDesc varianceSMDesc;
		varianceSMDesc.m_width = m_methodDesc.m_shadowMapSize;
		varianceSMDesc.m_height = m_methodDesc.m_shadowMapSize;
		varianceSMDesc.m_needColorContent = UE_TRUE;
		varianceSMDesc.m_numColorBuffers = 1;
		varianceSMDesc.m_colorDesc[0].m_format =
			glBufferFormat_R32G32B32A32F; // Just testing
			// glBufferFormat_R16G16F; // Issues with 16F (need to bias offset to -1..1)

		m_varianceShadowMapGroup = glRenderGroup_Create(&varianceSMDesc);
		UE_ASSERT(m_varianceShadowMapGroup);
	}

	// Draw shadow maps and shadow mask
	for (u32 i = 0; i < m_numShadowMapPasses; i++)
	{
		ShadowMapPass(i);
		if (m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit)
			ShadowMaskPass(i);
	}

	if (m_methodDesc.m_drawOrder != DrawOrder_FullPassPerSplit)
		ShadowMaskPass(0);

	// Release all resources
	if (generatedSceneDepth)
	{
		glRenderBufferPool_ReleaseTextureBuffer(m_sceneDepthTexture);
		m_sceneDepthTexture = NULL;
	}

	glRenderGroup_Destroy(m_shadowMaskGroup);
	m_shadowMaskGroup = NULL;

	glRenderGroup_Destroy(m_shadowMapGroup);
	m_shadowMapGroup = NULL;

	for (u32 i = 0; i < m_numShadowMaps; i++)
		if (!m_extraData.m_allowShadowMapAccess)
		{
			glRenderBufferPool_ReleaseTextureBuffer(m_shadowMaps[i]);
			m_shadowMaps[i] = NULL;
		}

	if (m_methodDesc.m_enableVarianceShadows)
	{
		glRenderGroup_Destroy(m_varianceShadowMapGroup);
		m_varianceShadowMapGroup = NULL;

		for (u32 i = 0; i < m_numShadowMaps; i++)
		{
			glRenderBufferPool_ReleaseTextureBuffer(m_varianceShadowMaps[i]);
			m_varianceShadowMaps[i] = NULL;
		}
	}

	m_isRendering = UE_FALSE;
	return UE_TRUE;
}

void gxShadowMapper::ShadowMapPass(u32 passIndex)
{
	const u32 shadowMapIndex = (m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit) ? 0 : passIndex;

	// Begin rendering to shadow map
	glCtx_SetClearDepthStencil(m_ctx, 1.0f, 0);
	if (!m_useDepthTexture)
		glCtx_SetClearColor(m_ctx, 1, 1, 1, 1);

	switch (m_light.m_lightProjection)
	{
		case LightProjection_OmniCube:
		{
			// Continue rendering to same cube texture; just switch cube face
			if (passIndex > 0)
				glRenderGroup_SetColorRenderTarget(m_shadowMapGroup, 0, m_shadowMaps[shadowMapIndex]);

			UE_ASSERT(UE_FALSE);
			//m_shadowMapGroup->BeginCube(m_ctx, passIndex, (!m_useDepthTexture ? glClearFlag_Color : 0) | glClearFlag_Depth);
			break;
		}
		default:
			glRenderGroup_Begin(m_shadowMapGroup, m_ctx, (!m_useDepthTexture ? glClearFlag_Color : 0) | glClearFlag_Depth);
			break;
	}

	glCtx_SetColorWrite(m_ctx, 0, m_useDepthTexture ? 0 : glColorMask_All);

	// Set up draw callback parameters
	gxShadowMapper::DrawDepthFuncData funcData;
	funcData.m_shadowMapper = this;
	funcData.m_frustum = m_shadowMapPassInfos[passIndex].m_frustum;
	funcData.m_view = m_shadowMapPassInfos[passIndex].m_lightView;
	funcData.m_proj = m_shadowMapPassInfos[passIndex].m_lightProj;
	funcData.m_cullMode = m_methodDesc.m_enableVarianceShadows ? glCullMode_None : glCullMode_CW;

	// Draw the depth to shadow map
	m_extraData.m_drawDepthFunction(funcData);

	// End rendering to shadow map
	glRenderGroup_EndDrawing(m_shadowMapGroup);
	m_shadowMaps[shadowMapIndex] =
		m_useDepthTexture ?
		glRenderGroup_AcquireDepthStencilOutput(m_shadowMapGroup) :
		glRenderGroup_AcquireColorOutput(m_shadowMapGroup, 0);
	glRenderGroup_End(m_shadowMapGroup);

	glCtx_SetColorWrite(m_ctx, 0, glColorMask_All);

	// Generate variance shadow map
	if (m_methodDesc.m_enableVarianceShadows)
		GenerateVarianceShadowMap(shadowMapIndex, m_shadowMaps[shadowMapIndex]);
}

void gxShadowMapper::ShadowMaskPass(u32 passIndex)
{
	// Begin rendering to shadow mask
	u32 clearFlags = 0;
	if (passIndex == 0)
	{
		glCtx_SetClearDepthStencil(m_ctx, 1.0f, 0);
		glCtx_SetClearColor(m_ctx, 1, 1, 1, 1);
		clearFlags = glClearFlag_Color | (m_methodDesc.m_sampleSceneDepth ? 0 : glClearFlag_Depth);
	}
	else
	{
		// Continue rendering to same shadow mask textures we did in previous passes
		glRenderGroup_SetDepthStencilRenderTarget(m_shadowMaskGroup, m_shadowMaskDepth);
		glRenderGroup_SetColorRenderTarget(m_shadowMaskGroup, 0, m_shadowMaskColor);
	}

	glRenderGroup_Begin(m_shadowMaskGroup, m_ctx, clearFlags);

	// Set up draw callback parameters and do shadow mask drawing passes
	gxShadowMapper::DrawShadowFuncData funcData;
	funcData.m_shadowMapper = this;
	funcData.m_sceneDepthTexture = m_sceneDepthTexture;

	switch (m_methodDesc.m_drawOrder)
	{
		case DrawOrder_FullPassPerSplit:
		{
			// Do the shadow mask pass using single shadow map
			funcData.m_frustum = m_shadowMaskPassInfos[passIndex].m_frustum;
			funcData.m_cameraView = m_shadowMaskPassInfos[passIndex].m_cameraView;
			funcData.m_cameraProj = m_shadowMaskPassInfos[passIndex].m_cameraProj;

			funcData.m_doSinglePassForAllShadowMaps = UE_FALSE;
			funcData.m_shadowMaps[0] = m_methodDesc.m_enableVarianceShadows ? m_varianceShadowMaps[-1] : m_shadowMaps[0];
			funcData.m_cameraLightTransformations[0] = m_shadowMaskPassInfos[passIndex].m_lightTransformation;
			funcData.m_viewport = m_shadowMaskPassInfos[passIndex].m_shadowMaskViewport;

			if (!(m_extraData.m_skipSplitsBitMask & (1 << passIndex)))
				m_extraData.m_drawShadowFunction(funcData);
			break;
		}

		case DrawOrder_AllShadowMapsFirst_ShadowMaskPassPerSplit:
		{
			// Do all shadow mask passes one after another - one per shadow map
			for (u32 i = 0; i < m_methodDesc.m_numShadowSplits; i++)
			{
				funcData.m_frustum = m_shadowMaskPassInfos[i].m_frustum;
				funcData.m_cameraView = m_shadowMaskPassInfos[i].m_cameraView;
				funcData.m_cameraProj = m_shadowMaskPassInfos[i].m_cameraProj;

				funcData.m_doSinglePassForAllShadowMaps = UE_FALSE;
				funcData.m_shadowMaps[0] = m_methodDesc.m_enableVarianceShadows ? m_varianceShadowMaps[i] : m_shadowMaps[i];
				funcData.m_cameraLightTransformations[0] = m_shadowMaskPassInfos[i].m_lightTransformation;
				funcData.m_viewport = m_shadowMaskPassInfos[i].m_shadowMaskViewport;

				if (!(m_extraData.m_skipSplitsBitMask & (1 << i)))
					m_extraData.m_drawShadowFunction(funcData);
			}
			break;
		}

		case DrawOrder_AllShadowMapsFirst_SingleShadowMaskPass:
		{
			// Draw the shadow mask using all shadow maps at once
			// Note: This is the place where shadows for omni-directional lights always get rendered
			funcData.m_frustum = m_shadowMaskPassInfos[passIndex].m_frustum;
			funcData.m_cameraView = m_shadowMaskPassInfos[passIndex].m_cameraView;
			funcData.m_cameraProj = m_shadowMaskPassInfos[passIndex].m_cameraProj;

			// FIXME: How about dual-paraboloidal method? Does it have 2 light transformations or 1?
			funcData.m_doSinglePassForAllShadowMaps = UE_TRUE;
			for (u32 i = 0; i < m_numShadowMaps; i++)
			{
				funcData.m_shadowMaps[i] = m_shadowMaps[i];
				funcData.m_cameraLightTransformations[i] = m_shadowMaskPassInfos[i].m_lightTransformation;
			}

			if (m_light.m_lightProjection == LightProjection_Directional || m_light.m_lightProjection == LightProjection_Spot)
				for (u32 i = 0; i < m_methodDesc.m_numShadowSplits + 1; i++)
					funcData.m_splittingDistances[i] = m_methodDesc.m_splitDistances[i];

			m_extraData.m_drawShadowFunction(funcData);
			break;
		}
	}

	// End rendering to shadow mask
	glRenderGroup_EndDrawing(m_shadowMaskGroup);

	m_shadowMaskColor = glRenderGroup_AcquireColorTexture(m_shadowMaskGroup, 0);

	if (m_methodDesc.m_drawOrder == DrawOrder_FullPassPerSplit &&
		passIndex + 1 < m_methodDesc.m_numShadowSplits) // Will we need to continue rendering to same depth buffer?
	{
		glRenderBufferPool_ReleaseTextureBuffer(m_shadowMaps[0]);
		m_shadowMaskDepth = glRenderGroup_AcquireDepthStencilOutput(m_shadowMaskGroup);
	}

	glRenderGroup_End(m_shadowMaskGroup);
}

glTextureBuffer* gxShadowMapper::DrawSceneDepth()
{
	// Create render target group
	glRenderGroupDesc sceneDepthGroupDesc;
	sceneDepthGroupDesc.m_width = m_methodDesc.m_shadowMaskWidth;
	sceneDepthGroupDesc.m_height = m_methodDesc.m_shadowMaskHeight;

	if (m_useDepthTexture)
	{
		sceneDepthGroupDesc.m_hasDepthStencil = UE_TRUE;
		sceneDepthGroupDesc.m_needDepthStencilContent = UE_TRUE;
		sceneDepthGroupDesc.m_depthStencilDesc.m_format = glBufferFormat_D24S8;
	}
	else
	{
		sceneDepthGroupDesc.m_needColorContent = UE_TRUE;
		sceneDepthGroupDesc.m_numColorBuffers = 1;
		sceneDepthGroupDesc.m_colorDesc[0].m_format = glBufferFormat_R32F;
		sceneDepthGroupDesc.m_hasDepthStencil = UE_TRUE;
		sceneDepthGroupDesc.m_depthStencilDesc.m_format = glBufferFormat_D24S8;
	}

	glRenderGroup* sceneDepthGroup = glRenderGroup_Create(&sceneDepthGroupDesc);
	UE_ASSERT(sceneDepthGroup);

	// Draw scene depth
	glRenderGroup_Begin(sceneDepthGroup, m_ctx, glClearFlag_Depth | (m_useDepthTexture ? 0 : glClearFlag_Color));

	gxShadowMapper::DrawDepthFuncData funcData;
	funcData.m_shadowMapper = this;
	funcData.m_frustum.Build(m_camera.m_view, m_cameraProj);
	funcData.m_view = m_camera.m_view;
	funcData.m_proj = m_cameraProj;
	funcData.m_cullMode = glCullMode_CCW;

	m_extraData.m_drawDepthFunction(funcData);

	glRenderGroup_EndDrawing(sceneDepthGroup);
	glTextureBuffer* sceneDepthTexture =
		m_useDepthTexture ?
		glRenderGroup_AcquireDepthStencilOutput(sceneDepthGroup) :
		glRenderGroup_AcquireColorOutput(sceneDepthGroup, 0);
	glRenderGroup_End(sceneDepthGroup);

	glRenderGroup_Destroy(sceneDepthGroup);

	return sceneDepthTexture;
}

void gxShadowMapper::GenerateVarianceShadowMap(u32 index, glTextureBuffer* shadowMap)
{
	glRenderGroup_Begin(m_varianceShadowMapGroup, m_ctx, 0);

	glSamplerParams shadowMapSamplerDesc;
	shadowMapSamplerDesc.m_minFilter = glTexFilter_Nearest;
	shadowMapSamplerDesc.m_magFilter = glTexFilter_Nearest;
	shadowMapSamplerDesc.m_mipFilter = glTexFilter_None;
	shadowMapSamplerDesc.m_addressU = glTexAddr_Clamp;
	shadowMapSamplerDesc.m_addressV = glTexAddr_Clamp;
	shadowMapSamplerDesc.m_addressW = glTexAddr_Clamp;
	glCtx_SetSamplerConstant(m_ctx, gxCommonConstants::ShadowMap, shadowMap, &shadowMapSamplerDesc);

	UE_NOT_IMPLEMENTED();
//	glCtx_SetShader(m_ctx, glShaderType_Fragment, m_extraData.m_computeVarianceShader);

	glCtx_SetDepthTest(m_ctx, UE_FALSE);
	glCtx_SetDepthWrite(m_ctx, UE_FALSE);

	UE_NOT_IMPLEMENTED();
//	gxShapeDraw_DrawFullscreenRectangle(NULL, NULL);

	glCtx_SetDepthTest(m_ctx, UE_TRUE);
	glCtx_SetDepthWrite(m_ctx, UE_TRUE);

	glRenderGroup_EndDrawing(m_varianceShadowMapGroup);
	m_varianceShadowMaps[index] = glRenderGroup_AcquireColorOutput(m_varianceShadowMapGroup, 0);
	glRenderGroup_End(m_varianceShadowMapGroup);
}

ueBool gxShadowMapper::CalculateDrawPassesInfo()
{
	switch (m_methodDesc.m_warpMethod)
	{
		case WarpMethod_Standard: return CalculateDrawPassesInfo_WarpStandard();
		case WarpMethod_Perspective: return CalculateDrawPassesInfo_WarpPerspective();
		case WarpMethod_LightSpacePerspective: return CalculateDrawPassesInfo_WarpLightSpacePerspective();
		case WarpMethod_Trapezoidal: return CalculateDrawPassesInfo_WarpTrapezoidal();
	}
	return UE_FALSE;
}

ueBool gxShadowMapper::CalculateDrawPassesInfo_WarpPerspective()
{
	// Not yet supported
	return UE_FALSE;
}

ueBool gxShadowMapper::CalculateDrawPassesInfo_WarpLightSpacePerspective()
{
	// Not yet supported
	return UE_FALSE;
}

ueBool gxShadowMapper::CalculateDrawPassesInfo_WarpTrapezoidal()
{
	// Not yet supported
	return UE_FALSE;
}
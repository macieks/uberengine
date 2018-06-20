#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_DepthOfField_HQ_Data
{
	gxProgram m_downsampleAndCalcCOCProgram;
	gxProgram m_nearCOCProgram;
	gxProgram m_smallBlurProgram;
	gxProgram m_compositeProgram;

	struct ShaderConstants
	{
		glConstantHandle SmallBlurMap;
		glConstantHandle LargeBlurMap;
		glConstantHandle DOFNearParams;
		glConstantHandle DOFFarParams;
		glConstantHandle DOFLerpScale;
		glConstantHandle DOFLerpBias;
	} m_shaderConstants;
};

static gxPostprocess_DepthOfField_HQ_Data s_data;

void gxPostprocess_DepthOfField_HighQuality::Startup(StartupParams* params)
{
	s_data.m_downsampleAndCalcCOCProgram.Create(params->m_downsampleAndCalcCOCVSName, params->m_downsampleAndCalcCOCFSName);
	s_data.m_nearCOCProgram.Create(params->m_nearCOCVSName, params->m_nearCOCFSName);
	s_data.m_smallBlurProgram.Create(params->m_smallBlurVSName, params->m_smallBlurFSName);
	s_data.m_compositeProgram.Create(params->m_compositeVSName, params->m_compositeFSName);

	s_data.m_shaderConstants.DOFNearParams.Init("DOFNearParams", glConstantType_Float2);
	s_data.m_shaderConstants.DOFFarParams.Init("DOFFarParams", glConstantType_Float3);
	s_data.m_shaderConstants.DOFLerpScale.Init("DOFLerpScale", glConstantType_Float4);
	s_data.m_shaderConstants.DOFLerpBias.Init("DOFLerpBias", glConstantType_Float4);
	s_data.m_shaderConstants.SmallBlurMap.Init("SmallBlurMap", glConstantType_Sampler2D);
	s_data.m_shaderConstants.LargeBlurMap.Init("LargeBlurMap", glConstantType_Sampler2D);
}

void gxPostprocess_DepthOfField_HighQuality::Shutdown()
{
	s_data.m_downsampleAndCalcCOCProgram.Destroy();
	s_data.m_nearCOCProgram.Destroy();
	s_data.m_smallBlurProgram.Destroy();
	s_data.m_compositeProgram.Destroy();
}

void gxPostprocess_DepthOfField_HighQuality::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process depth of field (high quality)");

	UE_ASSERT(params->m_srcColor);
	const glTextureBufferDesc* srcDesc = glTextureBuffer_GetDesc(params->m_srcColor);

	glTextureBuffer* colorAndCoc = NULL;

	// Downsample to 1/4 by 1/4 and generate near COC
	{
		UE_PROF_SCOPE("dof downsample and calculate coc");

		// Begin

		glRenderGroupDesc desc;
		desc.m_width = srcDesc->m_width / 4;
		desc.m_height = srcDesc->m_height / 4;
		desc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
		desc.m_needColorContent = UE_TRUE;

		glRenderGroup* group = glRenderGroup_Create(&desc);
		glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

		// Draw

		const f32 nearGradient = -1.0f / (params->m_nearEnd - params->m_nearStart);
		ueVec3 dofNearParams(nearGradient, -params->m_nearEnd * nearGradient, 0.0f);
		glCtx_SetFloat3Constant(params->m_ctx, s_data.m_shaderConstants.DOFNearParams, &dofNearParams);

		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, params->m_srcColor, &glSamplerParams::DefaultClamp);

		ueVec4 colorMapSize;
		glUtils_SetMapSizeConstantValue(params->m_srcColor, colorMapSize);
		glCtx_SetFloat4Constant(params->m_ctx, gxCommonConstants::ColorMapSize, &colorMapSize);

		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::DepthMap, params->m_srcDepth, &glSamplerParams::DefaultPP);

		gxShape_FullscreenRect rect;
		rect.m_program = s_data.m_downsampleAndCalcCOCProgram.GetProgram();
		gxShapeDraw_DrawFullscreenRect(rect);

		// End

		glRenderGroup_EndDrawing(group);
		colorAndCoc = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		glRenderGroup_Destroy(group);
	}

	glTextureBuffer* largeBlur = NULL;

	// Blur
	{
		UE_PROF_SCOPE("dof near coc blur");

		for (u32 i = 0; i < 2; i++)
		{
			gxPostprocess_Blur::DrawParams blurParams;
			blurParams.m_ctx = params->m_ctx;
			blurParams.m_srcColor = i == 0 ? colorAndCoc : largeBlur;
			blurParams.m_blurKernel = params->m_nearBlurRadius;
			blurParams.m_type = (i & 1) ? gxPostprocess_Blur::DrawParams::Type_Vertical : gxPostprocess_Blur::DrawParams::Type_Horizontal;
 
			gxPostprocess_Blur::Draw(&blurParams);

			if (i > 0)
				glRenderBufferPool_ReleaseTextureBuffer(largeBlur);
			largeBlur = blurParams.m_dstColor;
		}
	}

	glTextureBuffer* smallBlur = NULL;

	// Near COC
	{
		UE_PROF_SCOPE("dof near coc");

		// Begin

		glRenderGroupDesc desc;
		desc.m_width = srcDesc->m_width / 4;
		desc.m_height = srcDesc->m_height / 4;
		desc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
		desc.m_needColorContent = UE_TRUE;

		glRenderGroup* group = glRenderGroup_Create(&desc);
		glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

		// Draw

		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, colorAndCoc, &glSamplerParams::DefaultPP);
		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::BlurMap, largeBlur, &glSamplerParams::DefaultPP);

		gxShape_FullscreenRect rect;
		rect.m_program = s_data.m_nearCOCProgram.GetProgram();
		gxShapeDraw_DrawFullscreenRect(rect);

		// End

		glRenderBufferPool_ReleaseTextureBuffer(colorAndCoc);

		glRenderGroup_EndDrawing(group);
		smallBlur = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		glRenderGroup_Destroy(group);
	}

	// Generate small blur
	{
		UE_PROF_SCOPE("dof small blur");

		// Begin

		glRenderGroupDesc desc;
		desc.m_width = srcDesc->m_width / 4;
		desc.m_height = srcDesc->m_height / 4;
		desc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
		desc.m_needColorContent = UE_TRUE;

		glRenderGroup* group = glRenderGroup_Create(&desc);
		glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

		// Draw

		ueVec4 colorMapSize;
		glUtils_SetMapSizeConstantValue(smallBlur, colorMapSize);
		glCtx_SetFloat4Constant(params->m_ctx, gxCommonConstants::ColorMapSize, &colorMapSize);

		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, smallBlur, &glSamplerParams::DefaultClamp);

		gxShape_FullscreenRect rect;
		rect.m_program = s_data.m_smallBlurProgram.GetProgram();
		gxShapeDraw_DrawFullscreenRect(rect);

		// End

		glRenderBufferPool_ReleaseTextureBuffer(smallBlur);

		glRenderGroup_EndDrawing(group);
		smallBlur = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		glRenderGroup_Destroy(group);
	}

	// Final composite DOF pass
	{
		UE_PROF_SCOPE("dof composite");

		// Begin

		glRenderGroup* group = params->m_group;
		if (!group)
		{
			glRenderGroupDesc desc;
			desc.m_width = srcDesc->m_width;
			desc.m_height = srcDesc->m_height;
			desc.m_colorDesc[0].m_format = srcDesc->m_format;
			desc.m_needColorContent = UE_TRUE;

			group = glRenderGroup_Create(&desc);
		}

		glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

		// Draw

		const f32 farGradient = 1.0f / (params->m_farEnd - params->m_farStart);
		const ueVec3 dofFarParams(farGradient, -params->m_farStart * farGradient, params->m_farBlurRadius / params->m_nearBlurRadius);
		glCtx_SetFloat3Constant(params->m_ctx, s_data.m_shaderConstants.DOFFarParams, &dofFarParams);

		const f32 d0 = 1.0f / 3.0f;
		const f32 d1 = 1.0f / 3.0f;
		const f32 d2 = 1.0f / 3.0f;

		const ueVec4 dofLerpScale(-1.0f / d0, -1.0f / d1, -1.0f / d2, 1.0f / d2);
		glCtx_SetFloat4Constant(params->m_ctx, s_data.m_shaderConstants.DOFLerpScale, &dofLerpScale);

		const ueVec4 dofLerpBias(1.0f, (1.0f - d2) / d1, 1.0f / d2, (d2 - 1.0f) / d2);
		glCtx_SetFloat4Constant(params->m_ctx, s_data.m_shaderConstants.DOFLerpBias, &dofLerpBias);

		glCtx_SetSamplerConstant(params->m_ctx, s_data.m_shaderConstants.SmallBlurMap, smallBlur, &glSamplerParams::DefaultClamp);
		glCtx_SetSamplerConstant(params->m_ctx, s_data.m_shaderConstants.LargeBlurMap, largeBlur, &glSamplerParams::DefaultClamp);
		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, params->m_srcColor, &glSamplerParams::DefaultClamp);
		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::DepthMap, params->m_srcDepth, &glSamplerParams::DefaultPP);

		ueVec4 colorMapSize;
		glUtils_SetMapSizeConstantValue(params->m_srcColor, colorMapSize);
		glCtx_SetFloat4Constant(params->m_ctx, gxCommonConstants::ColorMapSize, &colorMapSize);

		gxShape_FullscreenRect rect;
		rect.m_program = s_data.m_compositeProgram.GetProgram();
		gxShapeDraw_DrawFullscreenRect(rect);

		// End

		glRenderBufferPool_ReleaseTextureBuffer(smallBlur);
		glRenderBufferPool_ReleaseTextureBuffer(largeBlur);

		if (!params->m_group)
		{
			glRenderGroup_EndDrawing(group);
			params->m_dstColor = glRenderGroup_AcquireColorOutput(group, 0);
			glRenderGroup_End(group);
			glRenderGroup_Destroy(group);
		}
	}

}
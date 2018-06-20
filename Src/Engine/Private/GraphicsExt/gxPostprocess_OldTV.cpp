#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_OldTVData
{
	gxProgram m_program;

	ueResourceHandle<gxTexture> m_dustMap;
	ueResourceHandle<gxTexture> m_lineMap;
	ueResourceHandle<gxTexture> m_tvAndNoiseMap;

	struct ShaderConstants
	{
		glConstantHandle OverExposureAmount;
		glConstantHandle DustAmount;
		glConstantHandle FrameJitterFrequency;
		glConstantHandle MaxFrameJitter;
		glConstantHandle FilmColor;
		glConstantHandle GrainThicknes;
		glConstantHandle GrainAmount;
		glConstantHandle ScratchesAmount;
		glConstantHandle ScratchesLevel;
		glConstantHandle DustMap;
		glConstantHandle LineMap;
		glConstantHandle TvMap;
		glConstantHandle NoiseMap;
	} m_shaderConstants;
};

static gxPostprocess_OldTVData s_data;

void gxPostprocess_OldTV::Startup(StartupParams* params)
{
	// Get shaders

	s_data.m_program.Create(params->m_VSName, params->m_FSName);

	// Get textures

	s_data.m_dustMap.SetByName(params->m_dustMapName);
	s_data.m_lineMap.SetByName(params->m_lineMapName);
	s_data.m_tvAndNoiseMap.SetByName(params->m_tvAndNoiseMapName);

	// Get shader constant handles

	s_data.m_shaderConstants.OverExposureAmount.Init("OverExposureAmount", glConstantType_Float);
	s_data.m_shaderConstants.DustAmount.Init("DustAmount", glConstantType_Float);
	s_data.m_shaderConstants.FrameJitterFrequency.Init("FrameJitterFrequency", glConstantType_Float);
	s_data.m_shaderConstants.MaxFrameJitter.Init("MaxFrameJitter", glConstantType_Float);
	s_data.m_shaderConstants.FilmColor.Init("FilmColor", glConstantType_Float4);
	s_data.m_shaderConstants.GrainThicknes.Init("GrainThicknes", glConstantType_Float);
	s_data.m_shaderConstants.GrainAmount.Init("GrainAmount", glConstantType_Float);
	s_data.m_shaderConstants.ScratchesAmount.Init("ScratchesAmount", glConstantType_Float);
	s_data.m_shaderConstants.ScratchesLevel.Init("ScratchesLevel", glConstantType_Float);
	s_data.m_shaderConstants.DustMap.Init("DustMap", glConstantType_Sampler2D);
	s_data.m_shaderConstants.LineMap.Init("LineMap", glConstantType_Sampler2D);
	s_data.m_shaderConstants.TvMap.Init("TvMap", glConstantType_Sampler2D);
	s_data.m_shaderConstants.NoiseMap.Init("NoiseMap", glConstantType_Sampler2D);
}

void gxPostprocess_OldTV::Shutdown()
{
	s_data.m_dustMap = NULL;
	s_data.m_lineMap = NULL;
	s_data.m_tvAndNoiseMap = NULL;

	s_data.m_program.Destroy();
}

void gxPostprocess_OldTV::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process old tv");

	// Begin

	glCtx* ctx = params->m_ctx;

	UE_ASSERT(params->m_srcColor);
	const glTextureBufferDesc* srcDesc = glTextureBuffer_GetDesc(params->m_srcColor);

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

	glRenderGroup_Begin(group, ctx, glClearFlag_Color);

	// Set all shader constants

	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.OverExposureAmount, &params->m_overExposureAmount);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.DustAmount, &params->m_dustAmount);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.FrameJitterFrequency, &params->m_frameJitterFrequency);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.MaxFrameJitter, &params->m_maxFrameJitter);
	glCtx_SetFloat4Constant(ctx,s_data.m_shaderConstants.FilmColor, &params->m_filmColor);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.GrainThicknes, &params->m_grainThicknes);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.GrainAmount, &params->m_grainAmount);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.ScratchesAmount, &params->m_scratchesAmount);
	glCtx_SetFloatConstant(ctx, s_data.m_shaderConstants.ScratchesLevel, &params->m_scratchesLevel);

	glSamplerParams dustSamplerDesc;
	dustSamplerDesc.m_minFilter = glTexFilter_Linear;
	dustSamplerDesc.m_magFilter = glTexFilter_Linear;
	dustSamplerDesc.m_mipFilter = glTexFilter_None;
	dustSamplerDesc.m_addressU = glTexAddr_ClampToBorder;
	dustSamplerDesc.m_addressV = glTexAddr_ClampToBorder;
	dustSamplerDesc.m_borderColor = ueColor32::White;
	glCtx_SetSamplerConstant(ctx, s_data.m_shaderConstants.DustMap, gxTexture_GetBuffer(*s_data.m_dustMap), &dustSamplerDesc);

	glSamplerParams lineSampler;
	lineSampler.m_minFilter = glTexFilter_Nearest;
	lineSampler.m_magFilter = glTexFilter_Nearest;
	lineSampler.m_mipFilter = glTexFilter_None;
	lineSampler.m_addressU = glTexAddr_ClampToBorder;
	lineSampler.m_addressV = glTexAddr_Clamp;
	lineSampler.m_borderColor = ueColor32::White;
	glCtx_SetSamplerConstant(ctx, s_data.m_shaderConstants.LineMap, gxTexture_GetBuffer(*s_data.m_lineMap), &lineSampler);

	glSamplerParams tvSampler;
	tvSampler.m_minFilter = glTexFilter_Linear;
	tvSampler.m_magFilter = glTexFilter_Linear;
	tvSampler.m_mipFilter = glTexFilter_None;
	tvSampler.m_addressU = glTexAddr_Clamp;
	tvSampler.m_addressV = glTexAddr_Clamp;
	glCtx_SetSamplerConstant(ctx, s_data.m_shaderConstants.TvMap, gxTexture_GetBuffer(*s_data.m_tvAndNoiseMap), &tvSampler);

	glSamplerParams noiseSampler;
	noiseSampler.m_minFilter = glTexFilter_Linear;
	noiseSampler.m_magFilter = glTexFilter_Linear;
	noiseSampler.m_mipFilter = glTexFilter_None;
	noiseSampler.m_addressU = glTexAddr_Wrap;
	noiseSampler.m_addressV = glTexAddr_Wrap;
	glCtx_SetSamplerConstant(ctx, s_data.m_shaderConstants.NoiseMap, gxTexture_GetBuffer(*s_data.m_tvAndNoiseMap), &noiseSampler);

	glCtx_SetFloatConstant(ctx, gxCommonConstants::Time, &params->m_time);

	// Draw

	gxShape_FullscreenRect rect;
	rect.m_program = s_data.m_program.GetProgram();
	rect.m_colorMap = params->m_srcColor;
	rect.m_colorMapSampler = &glSamplerParams::DefaultPP;
	gxShapeDraw_DrawFullscreenRect(rect);

	// End

	if (!params->m_group)
	{
		glRenderGroup_EndDrawing(group);
		params->m_dstColor = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		glRenderGroup_Destroy(group);
	}
}

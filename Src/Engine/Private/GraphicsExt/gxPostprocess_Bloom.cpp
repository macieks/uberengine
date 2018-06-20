#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_BloomData
{
	gxProgram m_bloomCompositeProgram;

	struct ShaderConstants
	{
		glConstantHandle BlendFactor;
	} m_shaderConstants;
};

static gxPostprocess_BloomData s_data;

void gxPostprocess_Bloom::Startup(StartupParams* params)
{
	s_data.m_bloomCompositeProgram.Create(params->m_bloomCompositeVSName, params->m_bloomCompositeFSName);
	s_data.m_shaderConstants.BlendFactor.Init("BlendFactor", glConstantType_Float);
}

void gxPostprocess_Bloom::Shutdown()
{
	s_data.m_bloomCompositeProgram.Destroy();
}

void gxPostprocess_Bloom::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process bloom");

	glTextureBuffer* blur = params->m_srcBlur;

	if (!blur)
	{
		glTextureBuffer* color = params->m_srcColor;

		// Downscale

		for (u32 i = 0; i < params->m_numHalfsizeDownscalesForBlur; i++)
		{
			gxPostprocess_Down2x2::DrawParams down2x2Params;
			down2x2Params.m_ctx = params->m_ctx;
			down2x2Params.m_srcColor = color;

			gxPostprocess_Down2x2::Draw(&down2x2Params);

			if (color != params->m_srcColor) // Do not release input buffer
				glRenderBufferPool_ReleaseTextureBuffer(color);
			color = down2x2Params.m_dstColor;
		}

		// Blur

		for (u32 i = 0; i < params->m_numBlurIterations * 2; i++)
		{
			gxPostprocess_Blur::DrawParams blurParams;
			blurParams.m_ctx = params->m_ctx;
			blurParams.m_srcColor = color;
			blurParams.m_blurKernel = params->m_blurKernel;
			blurParams.m_type = (i & 1) ? gxPostprocess_Blur::DrawParams::Type_Vertical : gxPostprocess_Blur::DrawParams::Type_Horizontal;

			gxPostprocess_Blur::Draw(&blurParams);

			if (color != params->m_srcColor) // Do not release input buffer
				glRenderBufferPool_ReleaseTextureBuffer(color);
			color = blurParams.m_dstColor;
		}

		blur = color;
	}

	// Composite

	UE_PROF_SCOPE("post-process bloom composite");

	glRenderGroup* group = params->m_group;
	if (!group)
	{
		const glTextureBufferDesc* srcDesc = glTextureBuffer_GetDesc(params->m_srcColor);

		glRenderGroupDesc desc;
		desc.m_width = srcDesc->m_width;
		desc.m_height = srcDesc->m_height;
		desc.m_colorDesc[0].m_format = srcDesc->m_format;
		desc.m_needColorContent = UE_TRUE;

		group = glRenderGroup_Create(&desc);
	}

	glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

	// Draw

	glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::BlurMap, blur);
	glCtx_SetFloatConstant(params->m_ctx, s_data.m_shaderConstants.BlendFactor, &params->m_blendFactor);

#if 0
	gxShape_FullscreenRect rect;
	rect.m_program = s_data.m_bloomCompositeProgram.GetProgram();
	rect.m_colorMap = params->m_srcColor;
	rect.m_colorMapSampler = params->m_srcColorSampler;
	gxShapeDraw_DrawFullscreenRect(rect);
#else
	gxShape_TexturedRect rect;
	rect.m_program = s_data.m_bloomCompositeProgram.GetProgram();
	rect.m_colorMap = params->m_srcColor;
	rect.m_colorMapSampler = params->m_srcColorSampler;
	if (params->m_dstRect)
		rect.m_pos = *params->m_dstRect;
	gxShapeDraw_DrawTexturedRect(rect);
#endif

	// End

	if (blur != params->m_srcBlur) // Do not release input buffer
		glRenderBufferPool_ReleaseTextureBuffer(blur);

	if (!params->m_group)
	{
		glRenderGroup_EndDrawing(group);
		params->m_dstColor = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		glRenderGroup_Destroy(group);
	}
}
#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_DepthOfField_LQ_Data
{
	gxProgram m_program;

	struct ShaderConstants
	{
		glConstantHandle DOFParams;
	} m_shaderConstants;
};

static gxPostprocess_DepthOfField_LQ_Data s_data;

void gxPostprocess_DepthOfField_LowQuality::Startup(StartupParams* params)
{
	s_data.m_program.Create(params->m_VSName, params->m_FSName);
	s_data.m_shaderConstants.DOFParams.Init("DOFParams", glConstantType_Float4);
}

void gxPostprocess_DepthOfField_LowQuality::Shutdown()
{
	s_data.m_program.Destroy();
}

void gxPostprocess_DepthOfField_LowQuality::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process depth of field (low quality)");

	// Begin

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

	glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

	// Draw

	const f32 nearGradient = -1.0f / (params->m_nearEnd - params->m_nearStart);
	const f32 farGradient = 1.0f / (params->m_farEnd - params->m_farStart);
	const ueVec4 dofParams(nearGradient, -params->m_nearEnd * nearGradient, farGradient, -params->m_farStart * farGradient);
	glCtx_SetFloat4Constant(params->m_ctx, s_data.m_shaderConstants.DOFParams, &dofParams);

	glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::BlurMap, params->m_srcBlur, &glSamplerParams::DefaultClamp);
	glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, params->m_srcColor, &glSamplerParams::DefaultPP);
	glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::DepthMap, params->m_srcDepth, &glSamplerParams::DefaultPP);

	gxShape_FullscreenRect rect;
	rect.m_program = s_data.m_program.GetProgram();
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
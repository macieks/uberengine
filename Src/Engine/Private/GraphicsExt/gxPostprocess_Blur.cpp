#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_BlurData
{
	gxProgram m_blurPrograms[gxPostprocess_Blur::DrawParams::Type_MAX];

	struct ShaderConstants
	{
		glConstantHandle BlurKernel;
	} m_shaderConstants;
};

static gxPostprocess_BlurData s_data;

void gxPostprocess_Blur::Startup(StartupParams* params)
{
	s_data.m_blurPrograms[DrawParams::Type_Vertical].Create(params->m_verticalBlurVSName, params->m_verticalBlurFSName);
	s_data.m_blurPrograms[DrawParams::Type_Horizontal].Create(params->m_horizontalBlurVSName, params->m_horizontalBlurFSName);
//	s_data.m_blurPrograms[DrawParams::Type_Radial].Create(params->m_radialBlurVSName, params->m_radialBlurFSName);

	s_data.m_shaderConstants.BlurKernel.Init("BlurKernel", glConstantType_Float);
}

void gxPostprocess_Blur::Shutdown()
{
	for (u32 i = 0; i < DrawParams::Type_MAX; i++)
		s_data.m_blurPrograms[i].Destroy();
}

void gxPostprocess_Blur::Draw(DrawParams* params)
{
#if defined(UE_ENABLE_PROFILER)
	static const u32 blur_profilerScopeId[DrawParams::Type_MAX] =
	{
		ueProfiler_GetScopeId("post-process vertical blur", U32_MAX),
		ueProfiler_GetScopeId("post-process horizontal blur", U32_MAX),
		ueProfiler_GetScopeId("post-process radial blur", U32_MAX)
	};
	ueScopeProfiler scopeProfiler(blur_profilerScopeId[params->m_type]);
#endif

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

	glCtx_SetFloatConstant(params->m_ctx, s_data.m_shaderConstants.BlurKernel, &params->m_blurKernel);

	gxShape_FullscreenRect rect;
	rect.m_program = s_data.m_blurPrograms[params->m_type].GetProgram();
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

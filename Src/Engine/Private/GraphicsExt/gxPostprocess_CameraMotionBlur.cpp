#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"

struct gxPostprocess_CameraMotionBlurData
{
	gxProgram m_program;

	struct ShaderConstants
	{
		glConstantHandle PrevViewProj;
		glConstantHandle ViewProjInv;
		glConstantHandle DepthParams;
		glConstantHandle MaxMotionBlur;
	} m_shaderConstants;
};

static gxPostprocess_CameraMotionBlurData s_data;

void gxPostprocess_CameraMotionBlur::Startup(StartupParams* params)
{
	s_data.m_program.Create(params->m_VSName, params->m_FSName);

	s_data.m_shaderConstants.PrevViewProj.Init("PrevViewProj", glConstantType_Float4x4);
	s_data.m_shaderConstants.ViewProjInv.Init("ViewProjInv", glConstantType_Float4x4);
	s_data.m_shaderConstants.DepthParams.Init("DepthParams", glConstantType_Float2);
	s_data.m_shaderConstants.MaxMotionBlur.Init("MaxMotionBlur", glConstantType_Float);
}

void gxPostprocess_CameraMotionBlur::Shutdown()
{
	s_data.m_program.Destroy();
}

void gxPostprocess_CameraMotionBlur::Draw(DrawParams* params)
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

	ueVec2 depthParams(params->m_nearZ, params->m_farZ / (params->m_farZ - params->m_nearZ));
	glCtx_SetFloat2Constant(params->m_ctx, s_data.m_shaderConstants.DepthParams, &depthParams);

	glCtx_SetFloatConstant(params->m_ctx, s_data.m_shaderConstants.MaxMotionBlur, &params->m_maxMotionBlur);

	glCtx_SetFloat4x4Constant(params->m_ctx, s_data.m_shaderConstants.ViewProjInv, params->m_viewProjInv);
	glCtx_SetFloat4x4Constant(params->m_ctx, s_data.m_shaderConstants.PrevViewProj, params->m_prevViewProj);

	glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::ColorMap, params->m_srcColor, &glSamplerParams::DefaultClamp);
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
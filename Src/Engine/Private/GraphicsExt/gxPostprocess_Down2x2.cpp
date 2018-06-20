#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"

struct gxPostprocess_Down2x2Data
{
	gxProgram m_program;
};

static gxPostprocess_Down2x2Data s_data;

void gxPostprocess_Down2x2::Startup(StartupParams* params)
{
	s_data.m_program.Create(params->m_VSName, params->m_FSName);
}

void gxPostprocess_Down2x2::Shutdown()
{
	s_data.m_program.Destroy();
}

void gxPostprocess_Down2x2::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process down2x2");

	// Begin

	UE_ASSERT(params->m_srcColor);
	const glTextureBufferDesc* srcDesc = glTextureBuffer_GetDesc(params->m_srcColor);

	glRenderGroup* group = params->m_group;
	if (!group)
	{
		glRenderGroupDesc desc;
		desc.m_width = srcDesc->m_width / 2;
		desc.m_height = srcDesc->m_height / 2;
		desc.m_colorDesc[0].m_format = srcDesc->m_format;
		desc.m_needColorContent = UE_TRUE;

		group = glRenderGroup_Create(&desc);
	}

	glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

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
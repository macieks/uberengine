#include "GraphicsExt/gxShadowVolumes.h"
#include "Graphics/glLib.h"

void gxShadowVolumes_BeginDraw(glCtx* ctx, gxShadowVolumesParams* params)
{
	const ueBool isTwoSidedStencilSupported = (glDevice_GetCaps()->m_supportsTwoSidedStencil) != 0;
	UE_ASSERT(params->m_pass != gxShadowVolumesParams::Pass_TwoSided || isTwoSidedStencilSupported);

	glCtx_SetColorWrite(ctx, 0, 0);

	glCtx_SetDepthTest(ctx, UE_TRUE);
	glCtx_SetDepthWrite(ctx, UE_FALSE);
	glCtx_SetDepthFunc(ctx, glCmpFunc_Less);

	glCtx_SetStencilTest(ctx, UE_TRUE);
	if (isTwoSidedStencilSupported)
		glCtx_SetTwoSidedStencilMode(ctx, params->m_pass == gxShadowVolumesParams::Pass_TwoSided);

	glCtx_SetStencilFunc(ctx, glCmpFunc_Always, (u8) U32_MAX, (u8) U32_MAX, 0);

	switch (params->m_mode)
	{
		case gxShadowVolumesParams::Mode_ZPass:

			glCtx_SetStencilOp(ctx,
				glStencilOp_Keep,
				glStencilOp_Keep,
				params->m_pass != gxShadowVolumesParams::Pass_Back ? glStencilOp_Incr : glStencilOp_Decr);

			if (params->m_pass == gxShadowVolumesParams::Pass_TwoSided)
				glCtx_SetCCWStencilOp(ctx, glStencilOp_Keep, glStencilOp_Keep, glStencilOp_Decr);

			break;

		case gxShadowVolumesParams::Mode_ZFail:

			if (params->m_pass == gxShadowVolumesParams::Pass_TwoSided)
			{
				glCtx_SetStencilOp(ctx, glStencilOp_Keep, glStencilOp_Decr, glStencilOp_Keep);
				glCtx_SetCCWStencilOp(ctx, glStencilOp_Keep, glStencilOp_Incr, glStencilOp_Keep);
			}
			else
				glCtx_SetStencilOp(ctx,
					glStencilOp_Keep,
					params->m_pass != gxShadowVolumesParams::Pass_Front ? glStencilOp_Incr : glStencilOp_Decr,
					glStencilOp_Keep);

			break;
	}
}

void gxShadowVolumes_EndDraw(glCtx* ctx)
{
	glCtx_SetColorWrite(ctx, 0, glColorMask_All);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetStencilTest(ctx, UE_FALSE);
	if (glDevice_GetCaps()->m_supportsTwoSidedStencil)
		glCtx_SetTwoSidedStencilMode(ctx, UE_FALSE);
}

void gxShadowVolumes_CalcDepthClampProj(ueMat44* transform)
{
}

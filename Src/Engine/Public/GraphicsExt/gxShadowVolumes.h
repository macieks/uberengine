#ifndef GX_SHADOW_VOLUMES_H
#define GX_SHADOW_VOLUMES_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueMath.h"

struct glCtx;

//! Shadow volumes parameters
struct gxShadowVolumesParams
{
	//! Modes
	enum Mode
	{
		Mode_ZPass = 0,		//!< Z-pass method
		Mode_ZFail,			//!< Z-fail method

		Mode_MAX
	};

	//! Passes
	enum Pass
	{
		Pass_Back = 0,	//!< Back only
		Pass_Front,		//!< Front only
		Pass_TwoSided,	//!< Two-sided (back and front at the same time)

		Pass_MAX
	};

	Mode m_mode;	//!< Mode
	Pass m_pass;	//!< Pass
};

//! Sets up render states necessary for rendering shadow volumes
void gxShadowVolumes_BeginDraw(glCtx* ctx, gxShadowVolumesParams* params);
//! Reverts render states after rendering shadow volumes
void gxShadowVolumes_EndDraw(glCtx* ctx);
//! Calculates depth clamp projection matrix for use with shadow volumes
void gxShadowVolumes_CalcDepthClampProj(ueMat44* transform);

// @}

#endif // GX_SHADOW_VOLUMES_H

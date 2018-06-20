#ifndef GX_COMMON_CONSTANTS_H
#define GX_COMMON_CONSTANTS_H

#include "Graphics/glLib.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Commonly used shader constants
class gxCommonConstants
{
public:
	static ueBool Startup();

	// Matrices
	// ------------------------

	static glConstantHandle World; //!< 4x4 world matrix
	static glConstantHandle View; //!< 4x4 view matrix
	static glConstantHandle Proj; //!< 4x4 projection matrix
	static glConstantHandle ViewProj; //!< 4x4 view-projection matrix
	static glConstantHandle WorldView; //!< 4x4 world-view matrix
	static glConstantHandle WorldViewProj; //!< 4x4 world-view-projection matrix
	static glConstantHandle BoneTransforms; //!< 4x4 bone matrix array

	// Samplers
	// ------------------------

	static glConstantHandle ColorMap; //!< Color texture
	static glConstantHandle ColorMap2; //!< Additional color texture
	static glConstantHandle ColorMapSize; //!< float4(width, height, 1 / width, 1 / height)
	static glConstantHandle DepthMap; //!< Depth texture
	static glConstantHandle NormalMap; //!< Normal texture
	static glConstantHandle SpecularMap; //!< Specular texture
	static glConstantHandle DetailMap; //!< Detail texture
	static glConstantHandle EnvMap; //!< Environment texture
	static glConstantHandle BlurMap; //!< Blur texture

	// Lighting
	// ------------------------

	static glConstantHandle LightDir; //!< 3f normalized light direction

	// Shadows
	// ------------------------

	static glConstantHandle ShadowMap; //!< Shadow map texture
	static glConstantHandle ShadowMapSize; //!< float4(width, height, 1 / width, 1 / height)
	static glConstantHandle ShadowMask; //!< Shadow mask texture
	static glConstantHandle LightMatrix; //!< 4x4 light transform for use by shadow mapping

	// Misc
	// ------------------------

	static glConstantHandle RightVec; //!< 3f normalized camera's right vector
	static glConstantHandle UpVec; //!< 3f normalized camera's up vector
	static glConstantHandle FrontVec; //!< 3f normalized camera's front vector

	static glConstantHandle RenderTargetSize; //!< float4(width, height, 1 / width, 1 / height)
	static glConstantHandle PixelOffset; //!< float2(x pixel offset, y pixel offset) - used to account for DirectX half pixel offset

	static glConstantHandle Color; //!< 4f rgba color
	static glConstantHandle Time; //!< 1f time in seconds
};

// @}

#endif // GX_COMMON_CONSTANTS_H

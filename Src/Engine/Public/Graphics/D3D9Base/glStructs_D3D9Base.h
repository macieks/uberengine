#pragma once

#include "Graphics/glStructs.h"

//! Base D3D9 device startup params
struct glDeviceStartupParams_D3D9Base : public glDeviceStartupParams_Base
{
	u32 m_maxVertexDecls;			//!< Max. number of all vertex declarations
	u32 m_maxVertexShaderDecls;		//!< Max. number of all vertex declaration entries among all shaders (same vertex decl. might be reused for multiple shaders thus creating multiple such objects)

	glDeviceStartupParams_D3D9Base();
};

// Device capabilities
// ------------------------------

struct glCaps_D3D9Base : public glCaps_Base
{
};

// Texture buffer
// ------------------------------

//! Texture buffer flags
enum glTextureBufferFlags
{
	glTextureBufferFlags_IsTexture		= UE_POW2(0),
	glTextureBufferFlags_IsRenderTarget	= UE_POW2(1),
	glTextureBufferFlags_IsDynamic		= UE_POW2(2)
};

//! Render target description within texture buffer
struct glTextureBufferRTDesc
{
	u32 m_MSAALevel;	//!< MSAA level (1 meaning no MSAA)

	glTextureBufferRTDesc() : m_MSAALevel(1) {}

	UE_INLINE ueBool operator == (const glTextureBufferRTDesc& other) const
	{
		return m_MSAALevel == other.m_MSAALevel;
	}
};

//! Texture buffer description
struct glTextureBufferDesc : public glTextureBufferDesc_Base
{
	glTextureBufferRTDesc m_rt;	//!< Optional render target description (when texture buffer is a render target)
	u32 m_flags;				//!< Texture buffer flags; see glTextureBufferFlags

	glTextureBufferDesc() : m_flags(glTextureBufferFlags_IsTexture) {}

	UE_INLINE ueBool operator == (const glTextureBufferDesc& other) const
	{
		return
			glTextureBufferDesc_Base::operator == ((const glTextureBufferDesc_Base&) other) &&
			m_rt == other.m_rt &&
			m_flags == other.m_flags;
	}
};
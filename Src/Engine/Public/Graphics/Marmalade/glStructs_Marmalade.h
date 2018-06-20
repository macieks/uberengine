#ifndef GL_STRUCTS_MARMALADE_H
#define GL_STRUCTS_MARMALADE_H

#include "Graphics/glStructs.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Device startup params
struct glDeviceStartupParams : public glDeviceStartupParams_Base
{
	glDeviceStartupParams() {}
};

//! Device capabilities
struct glCaps : public glCaps_Base
{
};

//! Texture buffer flags
enum glTextureBufferFlags
{
	glTextureBufferFlags_IsTexture		= UE_POW2(0),	//!< Is texture
	glTextureBufferFlags_IsRenderTarget	= UE_POW2(1),	//!< Is render target
	glTextureBufferFlags_IsDynamic		= UE_POW2(2)	//!< Is dynamic (can render to + changes frequently)
};

//! Render target description within texture buffer
struct glTextureBufferRTDesc
{
	u32 m_multisamplingLevel;	//!< Multisampling level (1 meaning no multisampling)

	glTextureBufferRTDesc() : m_multisamplingLevel(1) {}
};

//! Texture buffer description
struct glTextureBufferDesc : public glTextureBufferDesc_Base
{
	glTextureBufferRTDesc m_rt;	//!< Optional render target description (when texture buffer is a render target)
	u32 m_flags;				//!< Texture buffer flags; see glTextureBufferFlags

	glTextureBufferDesc() : m_flags(glTextureBufferFlags_IsTexture) {}
};

// @}

#endif // GL_STRUCTS_MARMALADE_H
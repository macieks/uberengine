#ifndef GL_STRUCTS_OPENGL_H
#define GL_STRUCTS_OPENGL_H

#include "Graphics/glStructs.h"

struct ueWindow;

/**
 *	@addtogroup gl
 *	@{
 */

//! Device startup params
struct glDeviceStartupParams : public glDeviceStartupParams_Base
{
	ueBool m_fullscreen;		//!< Start in fullscreen
	u32 m_adapterIndex;			//!< Index of the display adapter to be used

	u32 m_colorBits;			//!< Number of color bits
	u32 m_redBits;				//!< Number of red bits
	u32 m_greenBits;			//!< Number of green bits
	u32 m_blueBits;				//!< Number of blue bits
	u32 m_alphaBits;			//!< Number of alpha bits
	u32 m_depthBits;			//!< Number of depth buffer bits
	u32 m_stencilBits;			//!< Number of stencil buffer bits

	ueWindow* m_window;			//!< Window

	ueBool m_useHardwarePBOs;	//!< Whether to use hardware PBOs (only used in OpenGL >= 2.1)
	u32 m_maxPBOs;				//!< Max. number of OpenGL's PBOs
	u32 m_maxTotalPBOSize;		//!< Max total size of OpenGL's PBO data

	glDeviceStartupParams();
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
	glTextureBufferFlags_IsDynamic		= UE_POW2(2)	//!< Is dynamic (can render to)
};

//! Render target description within texture buffer
struct glTextureBufferRTDesc
{
	u32 m_MSAALevel;	//!< MSAA level (1 meaning no MSAA)

	glTextureBufferRTDesc() : m_MSAALevel(1) {}
};

//! Texture buffer description
struct glTextureBufferDesc : public glTextureBufferDesc_Base
{
	glTextureBufferRTDesc m_rt;	//!< Optional render target description (when texture buffer is a render target)
	u32 m_flags;				//!< Texture buffer flags; see glTextureBufferFlags

	glTextureBufferDesc() : m_flags(glTextureBufferFlags_IsTexture) {}
};

// @}

#endif // GL_STRUCTS_OPENGL_H

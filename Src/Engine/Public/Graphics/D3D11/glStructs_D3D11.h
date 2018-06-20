#pragma once

#include "Graphics/glLib.h"
#include "Graphics/glStructs.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

struct ueWindow;

/**
 *	@addtogroup gl
 *	@{
 */

//! Available Direct3D 9 device states (see DirectX SDK docs for more info)
enum glDeviceState
{
	glDeviceState_Valid = 0,		//!< Valid
	glDeviceState_Lost,				//!< Device is lost
	glDeviceState_NotReset,			//!< Device is awaiting reset
	glDeviceState_UnknownError,		//!< Unknown / critical error

	glDeviceState_MAX
};

//! Device startup params
struct glDeviceStartupParams : public glDeviceStartupParams_Base
{
	ueBool m_fullscreen;				//!< Start in fullscreen
	ueBool m_hardwareVertexProcessing;	//!< Enable hardware vertex processing
	ueBool m_multithreaded;				//!< Create multi-threaded device
	ueWindow* m_window;					//!< Window
	u32 m_adapterIndex;					//!< Index of the display adapter to be used
	D3D_DRIVER_TYPE m_driverType;		//!< Driver type to use

	u32 m_MSAAQuality;			//!< MSAA quality

	u32 m_maxVertexDecls;				//!< Max. number of all vertex declarations
	u32 m_maxVertexShaderDecls;			//!< Max. number of all vertex declaration entries among all shaders (same vertex decl. might be reused for multiple shaders thus creating multiple such objects)

	u32 m_maxSamplerStates;
	u32 m_maxBlendStates;
	u32 m_maxRasterStates;
	u32 m_maxDepthStencilStates;

	glDeviceStartupParams();
};

//! Device capabilities
struct glCaps : public glCaps_Base
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
};

//! Texture buffer description
struct glTextureBufferDesc : public glTextureBufferDesc_Base
{
	glTextureBufferRTDesc m_rt;	//!< Optional render target description (when texture buffer is a render target)
	u32 m_flags;				//!< Texture buffer flags; see glTextureBufferFlags

	glTextureBufferDesc() : m_flags(glTextureBufferFlags_IsTexture) {}
};

// @}
#pragma once

#include <d3d9types.h>
#include "Graphics/D3D9Base/glStructs_D3D9Base.h"

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
struct glDeviceStartupParams : public glDeviceStartupParams_D3D9Base
{
	ueBool m_fullscreen;				//!< Start in fullscreen
	ueBool m_hardwareVertexProcessing;	//!< Enable hardware vertex processing
	ueBool m_multithreaded;				//!< Create multi-threaded device
	ueWindow* m_window;					//!< Window
	u32 m_adapterIndex;					//!< Index of the display adapter to be used
	D3DDEVTYPE m_deviceType;			//!< Device type to be used

	glDeviceStartupParams();
};

//! Device capabilities
struct glCaps : public glCaps_D3D9Base
{
};

// @}
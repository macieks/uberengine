#pragma once

#include "Graphics/D3D9Base/glLib_D3D9Base.h"
#include "Graphics/D3D9/glStructs_D3D9.h"

/**
 *	@addtogroup gl
 *	@{
 */

// Device
// ------------------------------

//! Resets Direct3D 9 device
ueBool			glDevice_Reset(const glDeviceStartupParams* newParams = NULL);
//! Gets current Direct3D 9 device state
glDeviceState	glDevice_GetState();

// @}
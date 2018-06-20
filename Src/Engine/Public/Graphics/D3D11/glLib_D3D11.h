#pragma once

#include "Graphics/D3D11/glStructs_D3D11.h"

/**
 *	@addtogroup gl
 *	@{
 */

// Device
// ------------------------------

//! Resets Direct3D 11 device
ueBool			glDevice_Reset(const glDeviceStartupParams* newParams = NULL);

// @}
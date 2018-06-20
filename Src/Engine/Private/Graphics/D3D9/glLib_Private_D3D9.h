#pragma once

#include "Graphics/glLib.h"
#include "Graphics/glStructs_Private.h"

#include "Graphics/D3D9Base/glLib_Private_D3D9Base.h"
#include "Graphics/D3D9/glStructs_Private_D3D9.h"

// Device

void	glDevice_SetD3D9PresentationParameters(D3DPRESENT_PARAMETERS* pp, const glDeviceStartupParams* params);

// Index buffer

void	glIndexBuffer_OnLostDevice(glIndexBuffer* vb);
void	glIndexBuffer_OnResetDevice(glIndexBuffer* vb);

// Vertex buffer

void	glVertexBuffer_OnLostDevice(glVertexBuffer* vb);
void	glVertexBuffer_OnResetDevice(glVertexBuffer* vb);

// Texture buffer

void	glTextureBuffer_OnLostDevice(glTextureBuffer* tb);
void	glTextureBuffer_OnResetDevice(glTextureBuffer* tb);

// Queries

void	glOcclusionQuery_OnLostDevice(glOcclusionQuery* query);
void	glOcclusionQuery_OnResetDevice(glOcclusionQuery* query);

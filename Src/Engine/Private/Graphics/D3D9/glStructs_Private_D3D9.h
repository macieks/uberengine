#pragma once

#include "Base/Containers/ueGenericPool.h"
#include "Graphics/D3D9/glStructs_D3D9.h"
#include "Graphics/D3D9Base/glStructs_Private_D3D9Base.h"

// Device

struct glDevice : public glDevice_D3D9Base
{
	glDeviceStartupParams m_params;
	glCaps m_caps;
	f32 m_monitorAspectRatio;
};

// Context

struct glCtx : public glCtx_D3D9Base
{
};

// Texture buffer

struct glTextureBuffer : public glTextureBuffer_D3D9Base
{
};

// Shaders

struct glShader : glShader_Base
{
	IUnknown* m_handle;

	UE_INLINE IDirect3DPixelShader9* GetPSHandle() const { return (IDirect3DPixelShader9*) m_handle; }
	UE_INLINE IDirect3DVertexShader9* GetVSHandle() const { return (IDirect3DVertexShader9*) m_handle; }
};

// Index buffer

struct glIndexBuffer
{
	glIndexBufferDesc m_desc;
	IDirect3DIndexBuffer9* m_handle;
};

// Vertex buffer

struct glVertexBuffer
{
	glVertexBufferDesc m_desc;
	IDirect3DVertexBuffer9* m_handle;
};

struct glRenderGroup : public glRenderGroup_Base
{
};

// Occlusion query

struct glOcclusionQuery
{
	IDirect3DQuery9* m_handle;
};
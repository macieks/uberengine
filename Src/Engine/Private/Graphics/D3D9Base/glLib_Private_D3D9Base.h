#pragma once

#include "Graphics/D3D9Base/glStructs_Private_D3D9Base.h"

// Device

IDirect3D9*			glDevice_GetD3D();
#define	D3D			glDevice_GetD3D()
IDirect3DDevice9*	glDevice_GetD3Dev();
#define D3DEV		glDevice_GetD3Dev()

void glDevice_D3D9Base_Startup(const glDeviceStartupParams* params);
void glDevice_D3D9Base_Shutdown();

// Context

IDirect3DDevice9*	glCtx_GetD3Dev(glCtx* ctx);

void glCtx_D3D9Base_Init(glCtx* ctx);
void glCtx_D3D9Base_Deinit(glCtx* ctx);

void glCtx_D3D9Base_Reset(glCtx_D3D9Base* ctx, u32 resetFlags);

void glCtx_SetRenderState_Immediate(glCtx_D3D9Base* ctx, D3DRENDERSTATETYPE renderState, DWORD value);
void glCtx_SetSamplerState_Immediate(glCtx_D3D9Base* ctx, glShaderType shaderType, u32 samplerIndex, D3DSAMPLERSTATETYPE samplerState, DWORD value);
void glCtx_SetSamplerState_Immediate(glCtx_D3D9Base* ctx, glShaderType shaderType, u32 samplerIndex, const glSamplerParams* desc);
void glCtx_SetTexture_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* textureBuffer);

void glCtx_D3D9Base_SetVertexDeclaration(glCtx_D3D9Base* ctx, glVertexDeclaration);

// Vertex shader input

glVertexDeclaration*	glVertexShaderInput_GetVertexDeclaration(glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats);

// Vertex declaration

glVertexDeclaration*	glVertexDeclaration_Create(const glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats);
void					glVertexDeclaration_Destroy(glVertexDeclaration* vd);

// Texture

IDirect3DTexture9*			glTexture_GetHandle2D(glTextureBuffer* tb);
IDirect3DCubeTexture9*		glTexture_GetHandleCube(glTextureBuffer* tb);
IDirect3DVolumeTexture9*	glTexture_GetHandle3D(glTextureBuffer* tb);

// Utils

ueBool					glUtils_ToD3DFORMAT(glBufferFormat format, D3DFORMAT& d3dFormat);
D3DMULTISAMPLE_TYPE		glUtils_ToD3DMULTISAMPLE_TYPE(u32 MSAALevel);
D3DDECLUSAGE			glUtils_ToD3DDECLUSAGE(glSemantic semantic);
D3DDECLTYPE				glUtils_ToD3DDECLTYPE(ueNumType type, u8 count, u8 normalized);
D3DCMPFUNC				glUtils_ToD3DCMPFUNC(glCmpFunc ueFunc);
D3DBLENDOP				glUtils_ToD3DBLENDOP(glBlendOp blendOp);
D3DBLEND				glUtils_ToD3DBLEND(glBlendingFunc blendFunc);
D3DSTENCILOP			glUtils_ToD3DSTENCILOP(glStencilOperation stencilOp);
D3DCUBEMAP_FACES		glUtils_ToD3DCUBEMAP_FACES(glCubeFace face);
D3DPRIMITIVETYPE		glUtils_ToD3DPRIMITIVETYPE(glPrimitive primitiveType);
D3DTEXTUREADDRESS		glUtils_ToD3DTEXTUREADDRESS(glTexAddr addressMode);
D3DTEXTUREFILTERTYPE	glUtils_ToD3DTEXTUREFILTERTYPE(glTexFilter filter);

ueBool					glUtils_FromD3DFORMAT(glBufferFormat& format, D3DFORMAT d3dFormat);
ueBool					glUtils_FromD3DDECLUSAGE(glSemantic& semantic, D3DDECLUSAGE d3dDeclUsage);

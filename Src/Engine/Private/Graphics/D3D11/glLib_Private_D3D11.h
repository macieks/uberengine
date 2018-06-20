#pragma once

#include "Graphics/glLib.h"
#include "Graphics/D3D11/glStructs_Private_D3D11.h"

// Device

ID3D11Device*		glDevice_GetD3Dev();
#define D3DEV		glDevice_GetD3Dev()
ID3D11DeviceContext*glDevice_GetCtx();
#define D3DCTX		glDevice_GetCtx()

// Context

ID3D11DeviceContext*glCtx_GetCtx(glCtx* ctx);

void glCtx_Init(glCtx* ctx);
void glCtx_Deinit(glCtx* ctx);

void glCtx_Reset(glCtx* ctx, u32 resetFlags);

void glCtx_CommitRenderStates(glCtx* ctx);
void glCtx_SetSamplerState_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, const glSamplerParams* desc);
void glCtx_SetTexture_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* textureBuffer);

void glCtx_SetVertexDeclaration(glCtx* ctx, glVertexDeclaration* decl);

// Index buffer

void	glIndexBuffer_OnLostDevice(glIndexBuffer* vb);
void	glIndexBuffer_OnResetDevice(glIndexBuffer* vb);

// Vertex buffer

void	glVertexBuffer_OnLostDevice(glVertexBuffer* vb);
void	glVertexBuffer_OnResetDevice(glVertexBuffer* vb);

// Texture buffer

void	glTextureBuffer_CreateViews(glTextureBuffer* tb);
void	glTextureBuffer_DetermineD3D11Params(const glTextureBufferDesc* desc, DXGI_FORMAT& format, D3D11_USAGE& usage, UINT& bindFlags, UINT& miscFlags, UINT& CPUAccessFlags);

void	glTextureBuffer_OnLostDevice(glTextureBuffer* tb);
void	glTextureBuffer_OnResetDevice(glTextureBuffer* tb);

ID3D11Texture2D*	glTexture_GetHandle2D(glTextureBuffer* tb);
ID3D11Texture2D*	glTexture_GetHandleCube(glTextureBuffer* tb);
ID3D11Texture3D*	glTexture_GetHandle3D(glTextureBuffer* tb);

// Vertex shader input

glVertexDeclaration*	glVertexShaderInput_GetVertexDeclaration(glShader* vs, u32 numFormats, glStreamFormat** formats);

// Vertex declaration

glVertexDeclaration*	glVertexDeclaration_Create(glShader* vs, u32 numFormats, glStreamFormat** formats);
void					glVertexDeclaration_Destroy(glVertexDeclaration* vd);

// Utils

ueBool					glUtils_ToD3DFORMAT(glBufferFormat format, DXGI_FORMAT& d3dFormat);
DXGI_FORMAT				glUtils_ToD3DInputFormat(ueNumType type, u8 count, u8 isNormalized);
const char*				glUtils_ToD3DSemanticName(glSemantic semantic);
D3D11_COMPARISON_FUNC	glUtils_ToD3DCMPFUNC(glCmpFunc ueFunc);
D3D11_BLEND_OP			glUtils_ToD3DBLENDOP(glBlendOp blendOp);
D3D11_BLEND				glUtils_ToD3DBLEND(glBlendingFunc blendFunc);
D3D11_STENCIL_OP		glUtils_ToD3DSTENCILOP(glStencilOperation stencilOp);
D3D11_TEXTURECUBE_FACE	glUtils_ToD3DCUBEMAP_FACES(glCubeFace face);
D3D_PRIMITIVE_TOPOLOGY	glUtils_ToD3DPRIMITIVETYPE(glPrimitive primitiveType);
D3D11_TEXTURE_ADDRESS_MODE glUtils_ToD3DTEXTUREADDRESS(glTexAddr addressMode);
D3D11_FILTER			glUtils_ToD3DTEXTUREFILTERTYPE(glTexFilter minFilter, glTexFilter magFilter, glTexFilter mipFilter, ueBool comparison);

ueBool					glUtils_FromD3DFORMAT(glBufferFormat& format, DXGI_FORMAT d3dFormat);
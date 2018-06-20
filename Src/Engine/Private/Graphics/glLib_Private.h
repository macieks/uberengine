#ifndef GL_LIB_PRIVATE_H
#define GL_LIB_PRIVATE_H

#include "Graphics/glLib.h"

#if defined(GL_D3D9)
	#include "Graphics/D3D9/glStructs_Private_D3D9.h"
	#include "Graphics/D3D9/glLib_Private_D3D9.h"
#elif defined(GL_D3D11)
	#include "Graphics/D3D11/glStructs_Private_D3D11.h"
	#include "Graphics/D3D11/glLib_Private_D3D11.h"
#elif defined(GL_X360)
	#include "Graphics/X360/glStructs_Private_X360.h"
	#include "Graphics/X360/glLib_Private_X360.h"
#elif defined(GL_OPENGL)
	#include "Graphics/OpenGL/glStructs_Private_OpenGL.h"
	#include "Graphics/OpenGL/glLib_Private_OpenGL.h"
#elif defined(GL_MARMALADE)
	#include "Graphics/Marmalade/glStructs_Private_Marmalade.h"
#endif


// Device

ueBool			glDevice_IsSet();
glDevice*		glDevice_Get();
void			glDevice_Set(glDevice* dev);
#define	GLDEV	glDevice_Get()

ueAllocator*	glDevice_GetFreqAllocator();
ueAllocator*	glDevice_GetStackAllocator();
#define GL_FREQ_ALLOC glDevice_GetFreqAllocator()
#define GL_STACK_ALLOC glDevice_GetStackAllocator()

//! Gets physical monitor aspect ratio
f32				glDevice_GetMonitorAspectRatio();
//! Gets pixel-wise aspect ratio of the main render target
f32				glDevice_GetBufferAspectRatio();

void			glDevice_Base_Startup(const glDeviceStartupParams_Base* params);
void			glDevice_Base_Shutdown();

// Context

void			glCtx_Init(glCtx* ctx);
void			glCtx_Deinit(glCtx* ctx);

void			glCtx_Base_Init(glCtx* ctx);
void			glCtx_Base_Deinit(glCtx* ctx);
void			glCtx_Base_Reset(glCtx* ctx, u32 flags);

void			glCtx_CommitProgram(glCtx* ctx);
void			glCtx_CommitRenderStates(glCtx* ctx);

ueBool			glCtx_PreDraw(glCtx* ctx);

void			glCtx_SetRenderGroup(glCtx* ctx, glRenderGroup* group);
void			glCtx_ResolveRenderTargets(glCtx* ctx);

// Render buffer pool (optional helper)

void			glRenderBufferPool_Startup();
void			glRenderBufferPool_Shutdown();
glTextureBuffer* glRenderBufferPool_GetTextureBuffer(glTextureBufferDesc& desc, ueBool createIfNotFound, glRenderGroup* group);
void			glRenderBufferPool_ReleaseTextureBuffer(glTextureBuffer* tb);

// Texture buffer

void			glTextureBuffer_CreateInPlace(glTextureBuffer* tb, const glTextureInitData* initData = NULL);
void			glTextureBuffer_DestroyInPlace(glTextureBuffer* tb);

// Vertex buffer

void			glVertexBuffer_CreateInPlace(glVertexBuffer* vb, const void* initData);
void			glVertexBuffer_DestroyInPlace(glVertexBuffer* vb);

// Index buffer

void			glIndexBuffer_CreateInPlace(glIndexBuffer* ib, const void* initData);
void			glIndexBuffer_DestroyInPlace(glIndexBuffer* ib);

// Shader constant manager

void					glShaderConstantMgr_Startup();
void					glShaderConstantMgr_Shutdown();
const glConstant* glShaderConstantMgr_GetConstant(const glShaderConstantDesc* desc);

// Vertex shader input

glVertexShaderInput*	glVertexShaderInput_Create(const glVertexShaderInputDesc* desc);
void					glVertexShaderInput_Destroy(glVertexShaderInput* vsi);

// Sampler desc

void					glSamplerDesc_InitializeDefaults();

#endif // GL_LIB_PRIVATE_H

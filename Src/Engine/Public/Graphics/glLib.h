#ifndef GL_LIB_H
#define GL_LIB_H

/**
 *	@defgroup gl Graphics
 *	@brief Low-level graphics library wrapper over APIs such as DirectX or OpenGL.
 */

#include "Base/ueHash.h"
#include "Base/ueShapes.h"
#include "Base/ueNumericalTypes.h"

/**
 *	@addtogroup gl
 *	@{
 */

#if defined(GL_D3D9)
	#include "Graphics/D3D9/glStructs_D3D9.h"
	#include "Graphics/D3D9/glLib_D3D9.h"
#elif defined(GL_D3D11)
	#include "Graphics/D3D11/glStructs_D3D11.h"
	#include "Graphics/D3D11/glLib_D3D11.h"
#elif defined(GL_X360)
	#include "Graphics/X360/glStructs_X360.h"
	#include "Graphics/X360/glLib_X360.h"
#elif defined(GL_OPENGL)
	#include "Graphics/OpenGL/glStructs_OpenGL.h"
	#include "Graphics/OpenGL/glLib_OpenGL.h"
#elif defined(GL_MARMALADE)
	#include "Graphics/Marmalade/glStructs_Marmalade.h"
	#include "Graphics/Marmalade/glLib_Marmalade.h"
#endif

#include "Graphics/glLib_Shared.h"

// Device
// ------------------------------

//! Pre-starts up device
void				glDevice_PreStartup();
//! Starts up device
void				glDevice_Startup(const glDeviceStartupParams* params);
//! Shuts down device; assets on any non-released resources
void				glDevice_Shutdown();
//! Gets closest supported device mode
void				glDevice_GetClosestSupportedMode(glDeviceStartupParams* params);
//! Gets size of used texture memory
u32					glDevice_GetUsedTextureMemory();
//! Changes display mode
ueBool				glDevice_ChangeMode(const glDeviceStartupParams* params);

//! Gets device startup parameters
const glDeviceStartupParams*glDevice_GetStartupParams();
//! Gets physical aspect ratio of the main render target
f32					glDevice_GetAspectRatio();
//! Gets device capabilities
const glCaps*		glDevice_GetCaps();
//! Gets device capabilities
#define GLCAPS		glDevice_GetCaps()
//! Gets default device context
glCtx*				glDevice_GetDefaultContext();

//! Gets device back buffer (may be NULL on some platforms)
glTextureBuffer*	glDevice_GetBackBuffer();
//! Gets device depth stencil buffer (may be NULL on some platforms)
glTextureBuffer*	glDevice_GetDepthStencilBuffer();
//! Gets main render buffer group
glRenderGroup*		glDevice_GetMainRenderGroup();

//! Gets total number of texture buffers created
u32					glDevice_GetNumTextureBuffers();
//! Gets all texture buffers (up to specified capacity);
void				glDevice_GetTextureBuffers(glTextureBuffer** array, u32 capacity, u32 textureBufferFlags, u32& count);

// Context
// ------------------------------

//! Resets selected context's states (see glResetFlags)
void	glCtx_Reset(glCtx* ctx, u32 resetFlags = glResetFlags_All);

//! Begins drawing using given context
ueBool	glCtx_Begin(glCtx* ctx);
//! Ends drawing using given context
void	glCtx_End(glCtx* ctx);
//! Presents given texture buffer using given context
void	glCtx_Present(glCtx* ctx, glTextureBuffer* tb);

//! Gets current render buffer group of a given context
glRenderGroup*		glCtx_GetCurrentRenderGroup(glCtx* ctx);
//! Indicates whether current render target setup is to render to texture
ueBool	glCtx_IsRenderingToTexture(glCtx* ctx);

//! Sets clear color for a given context
void	glCtx_SetClearColor(glCtx* ctx, f32 red, f32 green, f32 blue, f32 alpha);
//! Sets depth and stencil clear values for a given context
void	glCtx_SetClearDepthStencil(glCtx* ctx, f32 depth, u32 stencil);
//! Clears selected buffers for a given context (see glClearFlags)
void	glCtx_Clear(glCtx* ctx, u32 clearFlags);

//! Begins occlusion query
void	glCtx_BeginQuery(glCtx* ctx, glOcclusionQuery* query);
//! Ends occlusion query
void	glCtx_EndQuery(glCtx* ctx, glOcclusionQuery* query);
//! Gets occlusion query result (blocks if forceFinish is set)
ueBool	glCtx_GetQueryResult(glCtx* ctx, glOcclusionQuery* query, u32& numSamplesRendered, ueBool forceFinish = UE_FALSE);

//! Begins conditional rendering
void	glCtx_BeginConditionalRendering(glCtx* ctx, glOcclusionQuery* query);
//! Ends conditional rendering
void	glCtx_EndConditionalRendering(glCtx* ctx);

//! Sets viewport for a context
void	glCtx_SetViewport(glCtx* ctx, const glViewportDesc& viewport);

//! Sets stream data and stream format for a given stream index
void	glCtx_SetStream(glCtx* ctx, u32 streamIndex, glVertexBuffer* vertexBuffer, glStreamFormat* format, u32 offset = 0, u32 divider = 0);
//! Sets indices
void	glCtx_SetIndices(glCtx* ctx, glIndexBuffer* indexBuffer);

//! Sets shader program
void	glCtx_SetProgram(glCtx* ctx, glProgram* program);

//! Draw indexed geometry
void	glCtx_DrawIndexed(glCtx* ctx, glPrimitive primitive, u32 baseVertexIndex, u32 minVertexIndex, u32 numVerts, u32 startIndex, u32 numIndices);
//! Draw non-indexed geometry
void	glCtx_Draw(glCtx* ctx, glPrimitive primitive, u32 startVertex, u32 numVerts);

//! Sets color-write flags
void	glCtx_SetColorWrite(glCtx* ctx, u32 colorTargetIndex, u32 colorMask);

//! Toggles alpha test
void	glCtx_SetAlphaTest(glCtx* ctx, ueBool enable);
//! Sets alpha test function and reference value
void	glCtx_SetAlphaFunc(glCtx* ctx, glCmpFunc alphaFunction, u32 referenceValue);

//! Toggles blending
void	glCtx_SetBlending(glCtx* ctx, ueBool enable);
//! Sets blending operation for color
void	glCtx_SetBlendOp(glCtx* ctx, glBlendOp op);
//! Sets blending operation for color and alpha
void	glCtx_SetBlendOp(glCtx* ctx, glBlendOp colorOp, glBlendOp alphaOp);
//! Sets blending function for color
void	glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc src, glBlendingFunc dst);
//! Sets blending function for color and alpha
void	glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc srcColor, glBlendingFunc dstColor, glBlendingFunc srcAlpha, glBlendingFunc dstAlpha);

//! Sets cull mode
void	glCtx_SetCullMode(glCtx* ctx, glCullMode mode);

//! Toggles depth test
void	glCtx_SetDepthTest(glCtx* ctx, ueBool enable);
//! Toggles depth write
void	glCtx_SetDepthWrite(glCtx* ctx, ueBool enable);
//! Sets depth function
void	glCtx_SetDepthFunc(glCtx* ctx, glCmpFunc depthFunction);

//! Sets fill mode
void	glCtx_SetFillMode(glCtx* ctx, glFillMode mode);

//! Toggles scissor test
void	glCtx_SetScissorTest(glCtx* ctx, ueBool enable);
//! Sets scissor rectangle
void	glCtx_SetScissorRectangle(glCtx* ctx, u32 left, u32 top, u32 width, u32 height);

//! Toggles stencil test
void	glCtx_SetStencilTest(glCtx* ctx, ueBool enable);
//! Sets stencil function
void	glCtx_SetStencilFunc(glCtx* ctx, glCmpFunc func, u32 ref, u32 mask, u32 writeMask);
//! Sets stencil operations for all stencil cases
void	glCtx_SetStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass);

//! Toggles two-sided stencil test
void	glCtx_SetTwoSidedStencilMode(glCtx* ctx, ueBool enable);
//! Sets CCW stencil function
void	glCtx_SetCCWStencilFunc(glCtx* ctx, glCmpFunc stencilFunc);
//! Sets CCW stencil operation
void	glCtx_SetCCWStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass);

#if !defined(GL_FIXED_PIPELINE)

/*
void	glCtx_SetOrthoProjTransform(glCtx* ctx, f32 left, f32 right, f32 top, f32 bottom, f32 nearZ, f32 farZ);
void	glCtx_SetViewTransform(glCtx* ctx, const ueMat44* transform);

void	glCtx_SetTexture(glCtx* ctx, u32 stageIndex, glTextureBuffer* tb, const glSamplerParams* samplerParams = NULL);

void	glCtx_SetPosData(glCtx* ctx, const f32* data, u32 stride);
void	glCtx_SetColorData(glCtx* ctx, const f32* data, u32 stride);
void	glCtx_SetPosData(glCtx* ctx, const f32* data, u32 stride);

#else
*/

//! Sets global shader sampler parameter
void	glCtx_SetSamplerConstant(glCtx* ctx, const glConstantHandle handle, glTextureBuffer* tb, const glSamplerParams* samplerParams = NULL);
//! Sets global shader numerical parameter
void	glCtx_SetNumericConstant(glCtx* ctx, const glConstantHandle handle, glConstantType dataType, const void* srcData, u32 srcStride, u32 count, u32 dstIndexOffset = 0);
//! Prints out current shader constant values for given shader
void	glCtx_PrintShaderConstantValues(glCtx* ctx, glShaderType shaderType);

// Helper functions for setting numerical shader parameters

UE_INLINE void glCtx_SetBoolConstant(glCtx* ctx, const glConstantHandle handle, const ueBool* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Int, value, sizeof(ueBool), count); }
UE_INLINE void glCtx_SetIntConstant(glCtx* ctx, const glConstantHandle handle, const s32* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Int, value, sizeof(s32), count); }
UE_INLINE void glCtx_SetFloatConstant(glCtx* ctx, const glConstantHandle handle, const f32* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float, value, sizeof(f32), count); }
UE_INLINE void glCtx_SetFloat2Constant(glCtx* ctx, const glConstantHandle handle, const f32* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float2, value, sizeof(f32) * 2, count); }
UE_INLINE void glCtx_SetFloat2Constant(glCtx* ctx, const glConstantHandle handle, const ueVec2* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float2, value, sizeof(ueVec2), count); }
UE_INLINE void glCtx_SetFloat3Constant(glCtx* ctx, const glConstantHandle handle, const f32* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float3, value, sizeof(f32) * 3, count); }
UE_INLINE void glCtx_SetFloat3Constant(glCtx* ctx, const glConstantHandle handle, const ueVec3* value, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float3, value, sizeof(ueVec3), count); }
UE_INLINE void glCtx_SetFloat4Constant(glCtx* ctx, const glConstantHandle handle, const f32* values, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float4, values, sizeof(f32) * 4, count); }
UE_INLINE void glCtx_SetFloat4Constant(glCtx* ctx, const glConstantHandle handle, const ueVec4* values, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float4, values, sizeof(ueVec4), count); }
UE_INLINE void glCtx_SetFloat4Constant(glCtx* ctx, const glConstantHandle handle, const ueColor32* value) { ueVec4 colorVec4; value->AsRGBAVec4(colorVec4); glCtx_SetNumericConstant(ctx, handle, glConstantType_Float4, &colorVec4, sizeof(ueVec4), 1); }
UE_INLINE void glCtx_SetFloat4x4Constant(glCtx* ctx, const glConstantHandle handle, const f32* values, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float4x4, values, sizeof(f32) * 16, count); }
UE_INLINE void glCtx_SetFloat4x4Constant(glCtx* ctx, const glConstantHandle handle, const ueMat44* values, u32 count = 1) { glCtx_SetNumericConstant(ctx, handle, glConstantType_Float4x4, values, sizeof(ueMat44), count); }

#endif // !defined(GL_FIXED_PIPELINE)

// Texture buffer
// ------------------------------

//! Creates texture buffer from description
glTextureBuffer*	glTextureBuffer_Create(const glTextureBufferDesc* desc, const glTextureInitData* initData = NULL);
//! Creates texture from description
glTextureBuffer*	glTextureBuffer_Create(const glTextureFileDesc* desc);
//! Destroys the texture
void				glTextureBuffer_Destroy(glTextureBuffer* tb);

//! Uploads data into texture's level
u32			glTextureBuffer_Update(glTextureBuffer* tb, u32 face, u32 level, const glTextureArea* area, const glTextureData* data);
//! Locks texture area
ueBool		glTextureBuffer_Lock(glTextureBuffer* tb, glTextureData* result, u32 face, u32 mipLevel, u32 lockFlags /*= glBufferLockFlags_Write*/, const glTextureArea* area /*= NULL*/);
//! Unlocks texture area
void		glTextureBuffer_Unlock(glTextureBuffer* tb, u32 face, u32 mipLevel);

//! Tells whether given texture buffer is a render target
ueBool		glTextureBuffer_IsRenderTarget(glTextureBuffer* tb);
//! Tells whether given texture buffer is a texture
ueBool		glTextureBuffer_IsTexture(glTextureBuffer* tb);
//! Gets texture description
const glTextureBufferDesc* glTextureBuffer_GetDesc(glTextureBuffer* tb);

//! Generates sub mip-map levels
void		glTextureBuffer_GenerateMips(glTextureBuffer* tb);
//! Copies data from one texture buffer to another
ueBool		glTextureBuffer_Copy(glCtx* ctx, glTextureBuffer* src, glTextureBuffer* dst);

//! Sets debug texture name (used for debugging purposes only)
void		glTextureBuffer_SetDebugName(glTextureBuffer* tb, const char* name);
//! Sets debug texture name (used for debugging purposes only)
const char* glTextureBuffer_GetDebugName(const glTextureBuffer* tb);

// Index buffer
// ------------------------------

//! Creates index buffer
glIndexBuffer*	glIndexBuffer_Create(const glIndexBufferDesc* desc, const void* initData = NULL);
//! Destroys index buffer
void			glIndexBuffer_Destroy(glIndexBuffer* ib);

//! Locks regions of index buffer
void*			glIndexBuffer_Lock(glIndexBuffer* ib, u32 lockFlags = glBufferFlags_WriteOnly, u32 offset = 0, u32 size = 0);
//! Unlocks regions of index buffer
void			glIndexBuffer_Unlock(glIndexBuffer* ib);

//! Gets index buffer description
const glIndexBufferDesc* glIndexBuffer_GetDesc(glIndexBuffer* ib);

// Vertex buffer
// ------------------------------

//! Creates vertex buffer
glVertexBuffer*	glVertexBuffer_Create(const glVertexBufferDesc* desc, const void* initData = NULL);
//! Destroys vertex buffer
void			glVertexBuffer_Destroy(glVertexBuffer* vb);

//! Locks regions of vertex buffer
void*			glVertexBuffer_Lock(glVertexBuffer* vb, u32 lockFlags = glBufferFlags_WriteOnly, u32 offset = 0, u32 size = 0);
//! Unlocks regions of vertex buffer
void			glVertexBuffer_Unlock(glVertexBuffer* vb);

//! Gets vertex buffer description
const glVertexBufferDesc* glVertexBuffer_GetDesc(glVertexBuffer* tb);

// Shader
// ------------------------------

//! Creates shader based on description
glShader*		glShader_Create(glShaderDesc* desc);
//! Destroys shader
void			glShader_Destroy(glShader* shader);

// Program (set of linked shaders)
// ------------------------------

//! Creates shader program from vertex, fragment and geometry shaders
glProgram*		glProgram_Create(glShader* vs, glShader* fs, glShader* gs = NULL);
//! Destroys shader program
void			glProgram_Destroy(glProgram* program);

// Stream format
// ------------------------------

//! Creates stream format for given description
glStreamFormat*	glStreamFormat_Create(const glStreamFormatDesc* desc);
//! Destroys stream format
void			glStreamFormat_Destroy(glStreamFormat* sf);
//! Gets stream format stride in bytes
u32				glStreamFormat_GetStride(glStreamFormat* sf);
//! Gets stream format element
const glVertexElement* glStreamFormat_GetElement(glStreamFormat* sf, glSemantic semantic, u32 usageIndex = 0);

//! Gets vertex element data as floats
void			glVertexElement_GetDataAsF32(const glVertexElement* element, f32* dst, u32 dstCount, const void* srcData);

// Render group
// ------------------------------

//! Creates render group (color0-3, depth) from given descripion (doesn't create any actual texture buffers)
glRenderGroup*			glRenderGroup_Create(const glRenderGroupDesc* desc);
//! Destroys render group
void					glRenderGroup_Destroy(glRenderGroup* group);

//! Begins drawing to this render group; internally gets or creates required render buffers
void					glRenderGroup_Begin(glRenderGroup* group, glCtx* ctx, u32 clearFlags = glClearFlag_All);
//! Begins drawing to this render group (specifies cube render target face)
void					glRenderGroup_BeginCube(glRenderGroup* group, glCtx* ctx, glCubeFace face, u32 clearFlags = glClearFlag_All);

//! Sets depth stencil render target (must be called after glRenderGroup_Begin or glRenderGroup_BeginCube)
void					glRenderGroup_SetDepthStencilRenderTarget(glRenderGroup* group, glTextureBuffer* tb);
//! Sets color render target (must be called after glRenderGroup_Begin or glRenderGroup_BeginCube)
void					glRenderGroup_SetColorRenderTarget(glRenderGroup* group, u32 index, glTextureBuffer* tb);
//! Sets depth stencil render target (must be called after glRenderGroup_Begin or glRenderGroup_BeginCube)
void					glRenderGroup_SetDepthStencilTexture(glRenderGroup* group, glTextureBuffer* tb);
//! Sets color render target (must be called after glRenderGroup_Begin or glRenderGroup_BeginCube)
void					glRenderGroup_SetColorTexture(glRenderGroup* group, u32 index, glTextureBuffer* tb);

//! Ends drawing to this render group and makes sure result of the rendering is stored in designated buffers ready for glRenderGroup_AcquireDepthStencilContent or glRenderGroup_Acquire* calls
void					glRenderGroup_EndDrawing(glRenderGroup* group);

//! Acquires depth stencil output of rendering to group (either render target or a texture)
glTextureBuffer*		glRenderGroup_AcquireDepthStencilOutput(glRenderGroup* group);
//! Acquires color output of rendering to group (either render target or a texture)
glTextureBuffer*		glRenderGroup_AcquireColorOutput(glRenderGroup* group, u32 index);

//! Acquires color render target buffer
glTextureBuffer*		glRenderGroup_AcquireDepthStencilRenderTarget(glRenderGroup* group);
//! Acquires depth stencil render target buffer
glTextureBuffer*		glRenderGroup_AcquireColorRenderTarget(glRenderGroup* group, u32 index);
//! Acquires result of rendering to render group (must be called between glRenderGroup_EndDrawing and glRenderGroup_End)
glTextureBuffer*		glRenderGroup_AcquireDepthStencilTexture(glRenderGroup* group);
//! Acquires result of rendering to render group (must be called between glRenderGroup_EndDrawing and glRenderGroup_End)
glTextureBuffer*		glRenderGroup_AcquireColorTexture(glRenderGroup* group, u32 index);

//! Ends drawing to this render group (must be called after glRenderGroup_EndDrawing); releases any unacquired render buffers
void					glRenderGroup_End(glRenderGroup* group);

//! Gets render group description
const glRenderGroupDesc* glRenderGroup_GetDesc(glRenderGroup* group);

// Render buffer pool
// ------------------------------

//! Gets render target buffer from pool
glTextureBuffer* glRenderBufferPool_GetRenderBuffer(glTextureBufferDesc& desc, ueBool createIfNotFound = UE_TRUE, glRenderGroup* group = NULL);
//! Releases render buffer back to pool
void		glRenderBufferPool_ReleaseTextureBuffer(glTextureBuffer* buffer);

//! Destroys all buffers in the pool
void		glRenderBufferPool_DestroyAllBuffers(ueBool includeAutoManaged = UE_FALSE);

// Queries
// ------------------------------

//! Creates occlusion query; type consists of glQueryType_* flags
glOcclusionQuery*	glQuery_Create(glQueryType type);
//! Destroys occlusion query
void		glQuery_Destroy(glOcclusionQuery* query);

// Utils
// ------------------------------

//! Gets number of display adapters
u32			glUtils_GetNumAdapters();
//! Gets details of adapter at given index
ueBool		glUtils_GetAdapterInfo(u32 adapterIndex, glAdapterInfo* info);
//! Gets output mode details for a given adapter and its mode index
ueBool		glUtils_GetOutputMode(u32 adapterIndex, u32 modeIndex, glOutputMode* mode);

//! Lists supported texture buffer formats
void		glUtils_ListSupportedFormats();

//! Converts data from source into destination format
ueBool		glUtils_ConvertFormat(void* dest, glBufferFormat dstFormat, const void* src, glBufferFormat srcFormat);
//! Converts data from ARGB8 into destination format
ueBool		glUtils_ConvertFromBGRA(void* dest, glBufferFormat dstFormat, const u8* src);
//! Converts data from source into ARGB8 format
ueBool		glUtils_ConvertToBGRA(u8* dest, const void* src, glBufferFormat srcFormat);
//! Gets whether format contains alpha channel
ueBool		glUtils_HasAlpha(glBufferFormat format);

//! Calculates draw primitive count
u32			glUtils_CalcPrimCount(glPrimitive primitive, u32 indexCount);

//! Sets shader constant used for texture map size - @see gxCommonConstants::ColorMapSize
void		glUtils_SetMapSizeConstantValue(glTextureBuffer* tb, ueVec4& value);

//! Calculates dimensions of the texture mip level
void		glUtils_CalcMipLevelDimensions(u32 width, u32 height, u32 depth, u32 level, u32& levelWidth, u32& levelHeight, u32& levelDepth);
//! Calculates texture mip level size
u32			glUtils_CalcMipLevelSize(const glTextureBufferDesc* desc, u32 level);
//! Calculates texture buffer size
u32			glUtils_CalcTextureBufferSize(const glTextureBufferDesc* desc);

//! Texture fill function callback
typedef void (*glTextureFillFunc)(u32 level, u32 width, u32 height, u32 depth, u32 x, u32 y, u32 z, u32& argb);
//! Fills the texture pixels using user supplied callback
ueBool		glUtils_FillTexture(glTextureBuffer* tb, glTextureFillFunc fillFunc);

// @}

#endif // GL_LIB_H

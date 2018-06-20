#ifndef GL_LIB_OPENGL_PRIVATE_H
#define GL_LIB_OPENGL_PRIVATE_H

#include "Graphics/glLib.h"
#include "Graphics/glLib_Private.h"
#include "Graphics/OpenGL/glStructs_Private_OpenGL.h"

// Context

void	glCtx_SetTexture_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* tb, const glSamplerParams* desc = NULL);

// PBO Manager

void glPBO_Startup();
void glPBO_Shutdown();
glPBO* glPBO_Map(GLenum target, GLenum usage, GLenum access, u32 size);
void glPBO_UnmapAndBind(glPBO* pbo);
void glPBO_Release(glPBO* pbo);

// Utils

void glUtils_BuildAdapterList();

void glUtils_InitCgSemanticsMap();
void glUtils_DeinitCgSemanticsMap();
void glUtils_GetUsageAndSemanticForCgAttrib(char* attrName, u32 attrNamelength, u8& semantic, u8& usageIndex, u8& isGeneric);

ueBool glUtils_ToOpenGLFormat(glBufferFormat src, u32 textureFlags, glOpenGLBufferFormat& dst);
ueBool glUtils_FromOpenGLFormat(glBufferFormat& dst, GLenum src);
GLenum glUtils_ToOpenGLPrimitive(glPrimitive primitiveType);
void glUtils_ToOpenGLType(ueNumType src, GLenum& dst);
GLenum glUtils_ToOpenGLCmpFunc(glCmpFunc ueFunc);
GLenum glUtils_ToOpenGLBlendOp(glBlendOp blendOp);
GLenum glUtils_ToOpenGLBlendFunc(glBlendingFunc blendFunc);
GLenum glUtils_ToOpenGLStencilOp(glStencilOperation stencilOp);
GLenum glUtils_ToOpenGLCubeFace(glCubeFace face);
GLenum glUtils_ToOpenGLTexAddr(glTexAddr addressMode);
void glUtils_ToOpenGLTexFilter(glTexFilter srcMinFilter, glTexFilter srcMagFilter, glTexFilter srcMipFilter, u32 numLevels, GLenum& dstMinFilter, GLenum& dstMagFilter);

ueBool glUtils_CheckFramebufferStatus(ueBool silentMode);
void glUtils_ReportUnsupportedFBO(glRenderGroup* group, const char* usageDesc);
const char* glUtils_GetErrorString(GLenum error);
void glUtils_CheckGLErrors(const char* operation, const char* file, u32 line);

#if defined(GL_CHECK_OPENGL_ERRORS)

	template <typename TYPE>
	TYPE glUtils_GLCall(TYPE result, const char* operation, const char* file, u32 line)
	{
		glUtils_CheckGLErrors(operation, file, line);
		return result;
	}

	#define GL(x) do { x; glUtils_CheckGLErrors(#x, __FILE__, __LINE__); } while (0)
	#define GLC(x) glUtils_GLCall(x, #x, __FILE__, __LINE__);

#else // defined(GL_CHECK_OPENGL_ERRORS)

	#define GL(x) x
	#define GLC(x) x

#endif // defined(GL_CHECK_OPENGL_ERRORS)

#endif // GL_LIB_OPENGL_PRIVATE_H

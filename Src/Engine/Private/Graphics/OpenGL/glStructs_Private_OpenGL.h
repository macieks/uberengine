#ifndef GL_STRUCTS_PRIVATE_OPENGL_H
#define GL_STRUCTS_PRIVATE_OPENGL_H

#if defined(UE_WIN32)
	#ifdef GL_OPENGL_ES
		extern bool GLEW_EXT_framebuffer_multisample;
		extern bool GLEW_EXT_texture_filter_anisotropic;
		extern bool GLEW_EXT_framebuffer_blit;
		extern bool GLEW_EXT_occlusion_query_boolean;
		#define GLEW_EXT_framebuffer_object 1
		#define GLEW_ARB_half_float_vertex 0
		#define GLEW_EXT_multisample 0
		#pragma comment(lib, "libEGL.lib")
		#pragma comment(lib, "libGLESv2.lib")
	#else
		#pragma comment(lib, "opengl32.lib")
		#pragma comment(lib, "glewlib.lib")
		#define GLEW_STATIC
		#include <GL/glew.h>
		#include <GL/wglew.h>
	#endif
#elif defined(UE_MAC)
	#define GLEW_STATIC
	#include <GL/glew.h>
	#include <OpenGL/OpenGL.h>
#elif defined(UE_LINUX)
	#define GLEW_STATIC
	#include <GL/glew.h>
	#include <GL/glxew.h>
#elif defined(UE_MARMALADE)
	#include "IwGL.h"
	#include "s3e.h"
	#define GL_NONE 0

	#define GLEW_EXT_framebuffer_multisample IwGLExtAvailable(IW_GL_APPLE_framebuffer_multisample)
	#define GLEW_ARB_half_float_vertex IwGLExtAvailable(IW_GL_OES_vertex_half_float)
	#define GLEW_EXT_texture_filter_anisotropic IwGLExtAvailable(IW_GL_EXT_texture_filter_anisotropic)
	#define GLEW_EXT_framebuffer_object IwGLExtAvailable(IW_GL_OES_framebuffer_object)
	#define GLEW_EXT_multisample 0
	#define GLEW_EXT_occlusion_query_boolean 0
#endif

#ifdef GL_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#define GL_GLEXT_PROTOTYPES
	#include <EGL/eglext.h>
	#include <GLES2/gl2ext.h>

	#define GLhandleARB GLuint
	#define glGenQueriesARB glGenQueriesEXT
	#define glDeleteQueriesARB glDeleteQueriesEXT
	#define glIsQueryARB glIsQueryEXT
	#define glBeginQueryARB glBeginQueryEXT
	#define glEndQueryARB glEndQueryEXT
	#define glGetQueryivARB glGetQueryivEXT
	#define glGetQueryObjectuivARB glGetQueryObjectuivEXT
	#define GL_ANY_SAMPLES_PASSED_ARB GL_ANY_SAMPLES_PASSED_EXT
	#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_ARB GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT
	#define GL_CURRENT_QUERY_ARB GL_CURRENT_QUERY_EXT
	#define GL_QUERY_RESULT_ARB GL_QUERY_RESULT_EXT
	#define GL_QUERY_RESULT_AVAILABLE_ARB GL_QUERY_RESULT_AVAILABLE_EXT
	#define GL_TEXTURE_3D GL_TEXTURE_3D_OES
	#define GL_TEXTURE_BINDING_3D GL_TEXTURE_BINDING_3D_OES
	#define GL_RENDERBUFFER_BINDING_EXT GL_RENDERBUFFER_BINDING
	#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER
	#define glCompressedTexImage3D glCompressedTexImage3DOES
	#define glTexImage3D glTexImage3DOES
	#define glTexSubImage3D glTexSubImage3D
	#define glGenerateMipmapEXT glGenerateMipmap
	#define glGenRenderbuffersEXT glGenRenderbuffers
	#define glRenderbufferStorageEXT glRenderbufferStorage
	#define glBindRenderbufferEXT glBindRenderbuffer
	#define glDeleteRenderbuffersEXT glDeleteRenderbuffers
	#define glRenderbufferStorageMultisampleEXT glRenderbufferStorageMultisampleANGLE
	#define GL_TEXTURE_MAX_LEVEL GL_TEXTURE_MAX_LEVEL_APPLE
	#define GL_STREAM_DRAW_ARB GL_STREAM_DRAW
	#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
	#define GL_DYNAMIC_DRAW_ARB GL_DYNAMIC_DRAW
	#define GL_WRITE_ONLY_ARB GL_WRITE_ONLY_OES
	#define glActiveTextureARB glActiveTexture
	#define glBindBufferARB glBindBuffer
	#define glEnableVertexAttribArrayARB glEnableVertexAttribArray
	#define glVertexAttribPointerARB glVertexAttribPointer
	#define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
	#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
	#define GL_TEXTURE_WRAP_R GL_TEXTURE_WRAP_R_OES
	#define glDepthRange glDepthRangef
	#define glClearDepth glClearDepthf
	#define GL_MULTISAMPLE_EXT GL_MULTISAMPLE
	#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
	#define glBindFramebufferEXT glBindFramebuffer
	#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT
	#define GL_STENCIL_ATTACHMENT_EXT GL_STENCIL_ATTACHMENT
	#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0
	#define glFramebufferTexture2DEXT glFramebufferTexture2D
	#define glFramebufferRenderbufferEXT glFramebufferRenderbuffer
	#define GL_READ_FRAMEBUFFER_EXT GL_READ_FRAMEBUFFER_ANGLE
	#define GL_READ_FRAMEBUFFER_BINDING_EXT GL_READ_FRAMEBUFFER_BINDING_ANGLE
	#define GL_DRAW_FRAMEBUFFER_EXT GL_DRAW_FRAMEBUFFER_ANGLE
	#define GL_DRAW_FRAMEBUFFER_BINDING_EXT GL_DRAW_FRAMEBUFFER_BINDING_ANGLE
	#define glBlitFramebufferEXT glBlitFramebufferANGLE
	#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB GL_MAX_TEXTURE_IMAGE_UNITS
	#define glGenFramebuffersEXT glGenFramebuffers
	#define glDeleteFramebuffersEXT glDeleteFramebuffers
	#define GL_VERTEX_SHADER_ARB GL_VERTEX_SHADER
	#define GL_FRAGMENT_SHADER_ARB GL_FRAGMENT_SHADER
	#define glShaderSourceARB glShaderSource
	#define glCompileShaderARB glCompileShader
	#define glCreateShaderObjectARB glCreateShader
	#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
	#define glCreateProgramObjectARB glCreateProgram
	#define glAttachObjectARB glAttachShader
	#define glLinkProgramARB glLinkProgram
	#define glUseProgramObjectARB glUseProgram
	#define glGetObjectParameterivARB glGetShaderiv
	#define GL_OBJECT_LINK_STATUS_ARB GL_LINK_STATUS
	#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
	#define glGetInfoLogARB glGetString
	#define GLcharARB GLchar
	#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB GL_ACTIVE_ATTRIBUTES
	#define GL_OBJECT_ACTIVE_UNIFORMS_ARB GL_ACTIVE_UNIFORMS
	#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB GL_ACTIVE_UNIFORM_MAX_LENGTH
	#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
	#define glGetActiveAttribARB glGetActiveAttrib
	#define glGetAttribLocationARB glGetAttribLocation
	#define glGetActiveUniformARB glGetActiveUniform
	#define glGetUniformLocationARB glGetUniformLocation
	#define glCheckFramebufferStatusEXT glCheckFramebufferStatus
	#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE
	#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
	#define GL_FRAMEBUFFER_UNSUPPORTED_EXT GL_FRAMEBUFFER_UNSUPPORTED
	#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
	#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
	#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT GL_FRAMEBUFFER_INCOMPLETE_FORMATS
	#define GLEW_framebuffer_blit GL_ANGLE_framebuffer_blit
	#define glBlitFramebufferEXT glBlitFramebufferANGLE
	#define GL_RGB5 GL_RGB5_A1
	#define GL_BGRA GL_BGRA_EXT
	#define GL_RGBA8 GL_RGBA8_OES
	#define GL_RGB8 GL_RGB8_OES
	#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
	#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
	#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
	#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
	#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
	#define GL_HALF_FLOAT GL_HALF_FLOAT_OES
	#define GLEW_VERSION_3_0 0
#endif

#include "Base/Containers/ueHashSet.h"
#include "Base/Containers/ueHashMap.h"
#include "Base/Containers/ueList.h"
#include "Graphics/glStructs_Private.h"

// Engine to OpenGL buffer format mapping

struct glOpenGLBufferFormat
{
	GLenum m_internalFormat;
	GLenum m_format;
	GLenum m_type;

	glBufferFormat m_fallback;

	glOpenGLBufferFormat() :
		m_internalFormat(GL_NONE),
		m_format(GL_NONE),
		m_type(GL_NONE),
		m_fallback(glBufferFormat_Unknown)
	{}
};

struct glOpenGLFormatMapping
{
	glOpenGLBufferFormat m_texFormat;
	glOpenGLBufferFormat m_rtFormat;
	glOpenGLBufferFormat m_rtTexFormat;
};

// Context

#if !defined(GL_FIXED_PIPELINE)

struct glProgram : glProgram_Base
{
	struct Uniform
	{
		GLint m_location;
		const glConstant* m_constant;
	};
	u32 m_numUniforms;
	Uniform* m_uniforms;

	struct Attr
	{
		GLint m_location;
		u8 m_semantic;
		u8 m_usageIndex;
		u8 m_isGeneric;
	};
	u32 m_numAttrs;
	Attr* m_attrs;

	u32 m_refCount;

	GLhandleARB m_handle;

	struct HashPred
	{
		UE_INLINE u32 operator () (const glProgram& value) const
		{
			return ueCalcHash(value.m_shaders, sizeof(value.m_shaders));
		}
	};

	struct CmpPred
	{
		UE_INLINE s32 operator () (const glProgram& a, const glProgram& b) const
		{
			return ueMemCmp(a.m_shaders, b.m_shaders, sizeof(a.m_shaders));
		}
	};
};

#endif // !defined(GL_FIXED_PIPELINE)

struct glCtx : public glCtx_Base
{
	u32 m_colorMask; // See glColorMask flags
	ueBool m_depthMask;
	GLuint m_stencilMask;

	u32 m_numSamplers;
	glTextureBuffer** m_samplers;

	glCullMode m_cullMode;

	GLuint m_fbo;
	GLuint m_fbo2;

	ueBool m_enableFBOTest;
	ueBool m_FBOTestResult;
};

// Device

struct glUsageAndSemantic
{
	u8 m_semantic;
	u8 m_minUsage;
	u8 m_maxUsage;
	u8 m_isGeneric;

	UE_INLINE glUsageAndSemantic() {}
	UE_INLINE glUsageAndSemantic(glSemantic semantic, u32 minUsage, u32 maxUsage, ueBool isGeneric) :
		m_semantic(semantic),
		m_minUsage(minUsage),
		m_maxUsage(maxUsage),
		m_isGeneric(isGeneric ? 1 : 0)
	{}
};

struct glDevice : public glDevice_Base
{
	glDeviceStartupParams m_params;
	glCaps m_caps;
	f32 m_monitorAspectRatio;

#ifdef GL_OPENGL_ES
	EGLNativeDisplayType m_displayType;
	EGLSurface m_EGLSurface;
	EGLDisplay m_EGLDisplay;
	EGLContext m_EGLContext;
	EGLConfig m_EGLConfig;
#elif defined(UE_WIN32)
	HDC m_hdc;
	HGLRC m_glContext;
#endif

#if !defined(GL_FIXED_PIPELINE)
	ueHashSet<glProgram, glProgram::HashPred, glProgram::CmpPred> m_programs;
	ueHashMap<const char*, glUsageAndSemantic> m_CgSemanticMap;
#endif

	ueHashMap<glRenderGroupDesc, ueBool> m_fboSupportCache;

	glOpenGLFormatMapping m_bufferFormatMappings[glBufferFormat_MAX];
};

// Pixel buffer object (PBO)

struct glPBO : ueList<glPBO>::Node
{
	GLuint m_handle;
	GLenum m_target;
	GLenum m_usage;
	GLuint m_size;

	glTextureArea m_lockedArea;
	u32 m_lockedSize;
	void* m_lockedData; // Pointer to data to write to

	void* m_texOpData; // Pointer to pass to glTexImage etc. functions (NULL for HW PBO, same as m_lockedData for emulated PBO)
};

// Texture buffer

struct glTextureBuffer : public glTextureBuffer_Base
{
   	glTextureBufferDesc m_desc;
	glSamplerParams m_samplerParams; // In OpenGL sampler description is bound to texture, so its best place to cache it
	GLuint m_textureHandle;
	GLuint m_renderBufferHandle;

	glPBO* m_pbos[16]; // PBO per (locked) mip level
};

// Shaders

#if !defined(GL_FIXED_PIPELINE)

struct glShader : public glShader_Base
{
	GLhandleARB m_handle;
#if defined(GL_ENABLE_SHADER_PREVIEW)
	char* m_debugSrc;
#endif
};

#endif // !defined(GL_FIXED_PIPELINE)

// Index & vertex buffer

struct glBuffer
{
	GLuint m_handle;
	u32 m_size;
	GLenum m_usage;
	GLenum m_target;
	GLenum m_binding;

	ueBool m_isLocked;
	u32 m_lockFlags;
	void* m_scratchPad;
	u32 m_scratchPadOffset;
	u32 m_scratchPadSize;
};

struct glIndexBuffer : glBuffer
{
	glIndexBufferDesc m_desc;
};

struct glVertexBuffer : glBuffer
{
	glVertexBufferDesc m_desc;
};

// Occlusion query

struct glOcclusionQuery
{
	glQueryType m_type;
	GLuint m_handle;
};

struct glRenderGroup : glRenderGroup_Base
{
};

#endif // GL_STRUCTS_PRIVATE_OPENGL_H

#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Base/ueSorting.h"
#include "Utils/ueWindow.h"
#include "Graphics/glLib_Private.h"

extern void glDevice_StartupPlatform(const glDeviceStartupParams* params);

static glDevice s_dev;

glDeviceStartupParams::glDeviceStartupParams() :
	m_fullscreen(UE_FALSE),
	m_adapterIndex(0),
	m_colorBits(32),
	m_redBits(8),
	m_greenBits(8),
	m_blueBits(8),
	m_alphaBits(8),
	m_depthBits(24),
	m_stencilBits(8),
	m_window(NULL),
	m_useHardwarePBOs(UE_TRUE),
	m_maxPBOs(256),
	m_maxTotalPBOSize(4096 * 4096 * 4 * 2 /* >= full mip chain for 4096 x 4096 ARGB texture */)
{}

ueBool glDevice_ChangeMode(const glDeviceStartupParams* params)
{
	if (s_dev.m_params.m_width != params->m_width ||
		s_dev.m_params.m_height != params->m_height ||
		s_dev.m_params.m_fullscreen != params->m_fullscreen ||
		s_dev.m_params.m_frequency != params->m_frequency)
	{
		if (!ueWindow_ChangeMode(s_dev.m_params.m_window, params->m_fullscreen, params->m_width, params->m_height, params->m_frequency))
			return UE_FALSE;

		s_dev.m_params.m_width = params->m_width;
		s_dev.m_params.m_height = params->m_height;
		s_dev.m_params.m_fullscreen = params->m_fullscreen;
		s_dev.m_params.m_frequency = params->m_frequency;
	}

	// Update back buffer etc.

	glRenderBufferPool_DestroyAllBuffers();

	s_dev.m_mainRenderGroup->m_desc.m_width = params->m_width;
	s_dev.m_mainRenderGroup->m_desc.m_height = params->m_height;

	s_dev.m_backBuffer->m_desc.m_width = params->m_width;
	s_dev.m_backBuffer->m_desc.m_height = params->m_height;

	s_dev.m_depthStencilBuffer->m_desc.m_width = params->m_width;
	s_dev.m_depthStencilBuffer->m_desc.m_height = params->m_height;

	GL(glViewport(0, 0, params->m_width, params->m_height));

	return UE_TRUE;
}

f32 glDevice_GetAspectRatio()
{
	return s_dev.m_params.m_fullscreen ? glDevice_GetMonitorAspectRatio() : glDevice_GetBufferAspectRatio();
}

f32 glDevice_GetMonitorAspectRatio()
{
	return s_dev.m_monitorAspectRatio;
}

void glDevice_CalcMonitorAspectRatio(const glDeviceStartupParams* params)
{
	s_dev.m_monitorAspectRatio = 16.0f / 9.0f;

	u32 largestSize = 0;

	glAdapterInfo info;
	u32 numModes = glUtils_GetAdapterInfo(params->m_adapterIndex, &info);
	for (u32 i = 0; i < info.m_numModes; i++)
	{
		glOutputMode mode;
		glUtils_GetOutputMode(params->m_adapterIndex, i, &mode);

		const u32 size = mode.m_width * mode.m_height;
		if (size > largestSize)
		{
			largestSize = size;
			s_dev.m_monitorAspectRatio = (f32) mode.m_width / (f32) mode.m_height;
		}
	}
}

void glDevice_PrintStats()
{
	ueLogI("Display info (OpenGL):");

	const char* renderer = (const char*) glGetString(GL_RENDERER);

	ueLogI("\tOpenGL version: %s", glGetString(GL_VERSION));
	ueLogI("\tOpenGL renderer: %s", glGetString(GL_RENDERER));
	ueLogI("\tOpenGL vendor: %s", glGetString(GL_VENDOR));
	ueLogI("\tOpenGL extensions: %s", glGetString(GL_EXTENSIONS));
#if defined(GL_OPENGL_ES)
	// Nothing
#elif defined(UE_WIN32)
	if (WGLEW_ARB_extensions_string)
		ueLogI("\tOpenGL Windows extensions: %s", wglGetExtensionsStringARB(s_dev.m_hdc));
#endif
	ueLogI("\tGLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glAdapterInfo adapter;
	glUtils_GetAdapterInfo(s_dev.m_params.m_adapterIndex, &adapter);
	ueLogI("\tAdapter: %s", adapter.m_description);
	ueLogI("\tDevice name: %s", adapter.m_deviceName);
	ueLogI("\tNative resolution: %u x %u", adapter.m_monitorWidth, adapter.m_monitorHeight);
}

void glDevice_CheckFormat(glBufferFormat engineFormat, GLenum internalFormat, GLenum format, GLenum type, glCaps::FormatInfo* srcInfo = NULL)
{
	glOpenGLFormatMapping& mapping = s_dev.m_bufferFormatMappings[engineFormat];
	glCaps::FormatInfo& info = s_dev.m_caps.m_formatInfos[engineFormat];

	glOpenGLBufferFormat fmt;
	fmt.m_internalFormat = internalFormat;
	fmt.m_format = format;
	fmt.m_type = type;

	if (srcInfo)
	{
		if (srcInfo->m_supportedAsTextureOnly)
			mapping.m_texFormat = fmt;
		if (srcInfo->m_supportedAsRTOnly)
			mapping.m_rtFormat = fmt;
		if (srcInfo->m_supportedAsRTAndTexture)
			mapping.m_rtTexFormat = fmt;

		info = *srcInfo;
		return;
	}

	// Check render target support

	if (!info.m_supportedAsRTOnly) // Only if not checked before
	{
		if (GLEW_EXT_framebuffer_object)
		{
			GLuint fbo;
			GL(glGenFramebuffersEXT(1, &fbo));
			GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo));

			GLuint buf;
			GL(glGenRenderbuffersEXT(1, &buf));
			GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buf));

			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, 16, 16);
			if (glGetError() == GL_NO_ERROR)
			{
				if (glUtils_IsDepthFormat(engineFormat) && glUtils_IsStencilFormat(engineFormat))
				{
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buf);
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buf);
	#ifndef GL_OPENGL_ES
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);
	#endif
				}
				else if (glUtils_IsDepthFormat(engineFormat))
				{
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buf);
	#ifndef GL_OPENGL_ES
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);
	#endif
				}
				else if (glUtils_IsStencilFormat(engineFormat))
				{
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buf);
	#ifndef GL_OPENGL_ES
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);
	#endif
				}
				else
				{
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buf);
	#ifndef GL_OPENGL_ES
					glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
					glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	#endif
				}

				if (glGetError() == GL_NO_ERROR &&
					glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
				{
					info.m_supportedAsRTOnly = UE_TRUE;
					mapping.m_rtFormat = fmt;
				}
			}

			GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0));
			GL(glDeleteRenderbuffersEXT(1, &buf));

			GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
			GL(glDeleteFramebuffersEXT(1, &fbo));
		}

		// No support for FBO? - try PBuffer

		else
		{
#ifdef GL_OPENGL_ES
		/*const EGLint attribListPbuffer[] = {
			EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
			EGL_RED_SIZE, 5,
			EGL_GREEN_SIZE, 6,
			EGL_BLUE_SIZE, 5,
			EGL_ALPHA_SIZE, 0,
			EGL_DEPTH_SIZE, 16,
			EGL_STENCIL_SIZE, 0,
			EGL_NONE
		};
		const EGLint srfPbufferAttr[] = {
			EGL_WIDTH, 256,
			EGL_HEIGHT, 256,
			// if enabled, the following lines result in a EGL_BAD_ATTRIBUTE error on the eglCreatePbufferSurface
			// EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
			// EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
			EGL_NONE
		};

			EGLSurface surface = eglCreatePbufferSurface(s_dev.m_EGLDisplay, s_dev.m_EGLConfig, surfaceAttribs);
			if (surface)
			{
				if (eglMakeCurrent(s_dev.m_EGLDisplay, surface, surface, surfaceAttribs))
				{
					info.m_supportedAsRTOnly = UE_TRUE;
					mapping.m_rtFormat = fmt;
				}

				eglDestroySurface(s_dev.m_EGLDisplay, surface);
			}*/
#endif
		}
	}

	// Check texture support

	ueBool isTextureSupported = UE_FALSE;

	GLuint tex;
	GL(glGenTextures(1, &tex));
	GL(glBindTexture(GL_TEXTURE_2D, tex));
#ifndef GL_OPENGL_ES
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1));
#endif
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	const u32 dummyTexSize = 16;
	if (glUtils_Is4x4CompressedFormat(engineFormat))
	{
		char dummyData[dummyTexSize * dummyTexSize / 2];
		const u32 dummyDataSize = dummyTexSize * dummyTexSize * 8 / glUtils_GetFormatBits(engineFormat);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dummyTexSize, dummyTexSize, 0, dummyDataSize, dummyData);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dummyTexSize, dummyTexSize, 0, format, type, NULL);
	if (glGetError() == GL_NO_ERROR)
	{
		isTextureSupported = UE_TRUE;
		if (!info.m_supportedAsTextureOnly)
		{
			info.m_supportedAsTextureOnly = UE_TRUE;
			mapping.m_texFormat = fmt;

			// Also check if this format supports auto mip-map generation

#ifdef GL_OPENGL_ES
			info.m_supportsAutoMips = UE_FALSE;
#else
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
			info.m_supportsAutoMips = glGetError() == GL_NO_ERROR;
#endif
		}
	}

	// Check texture and render target support

	if (isTextureSupported && !info.m_supportedAsRTAndTexture)
	{
		if (GLEW_EXT_framebuffer_object)
		{
			GLuint fbo;
			GL(glGenFramebuffersEXT(1, &fbo));
			GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo));

			if (glUtils_IsDepthFormat(engineFormat) && glUtils_IsStencilFormat(engineFormat))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex, 0);
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, tex, 0);
	#ifndef GL_OPENGL_ES
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
	#endif
			}
			else if (glUtils_IsDepthFormat(engineFormat))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex, 0);
	#ifndef GL_OPENGL_ES
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
	#endif
			}
			else if (glUtils_IsStencilFormat(engineFormat))
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, tex, 0);
	#ifndef GL_OPENGL_ES
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
	#endif
			}
			else
			{
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0);
	#ifndef GL_OPENGL_ES
				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
				glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	#endif
			}

			if (glGetError() == GL_NO_ERROR &&
				glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				info.m_supportedAsRTAndTexture = UE_TRUE;
				mapping.m_rtTexFormat = fmt;
			}

			GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
			GL(glDeleteFramebuffersEXT(1, &fbo));
		}

		// No support for FBO

		else
		{
			// TODO
		}
	}

	// Destroy texture

	GL(glFinish());

	GL(glBindTexture(GL_TEXTURE_2D, 0));

#ifndef GL_OPENGL_ES
	// FIXME: Workaround (causing resource leak) for crash on Intel(R) Graphics Media Accelerator HD, Build 8.15.10.2021
	if (!isTextureSupported || info.m_supportedAsRTAndTexture)
#endif
		GL(glDeleteTextures(1, &tex));
}

void glDevice_AddFallback(glBufferFormat src, glBufferFormat dst)
{
	glOpenGLFormatMapping& srcMapping = s_dev.m_bufferFormatMappings[src];
	const glCaps::FormatInfo& srcInfo = s_dev.m_caps.m_formatInfos[src];

	const glOpenGLFormatMapping& dstMapping = s_dev.m_bufferFormatMappings[dst];
	const glCaps::FormatInfo& dstInfo = s_dev.m_caps.m_formatInfos[dst];

	if (!srcInfo.m_supportedAsTextureOnly && dstInfo.m_supportedAsTextureOnly)
		srcMapping.m_texFormat.m_fallback = dst;
	if (!srcInfo.m_supportedAsRTOnly && dstInfo.m_supportedAsRTOnly)
		srcMapping.m_rtFormat.m_fallback = dst;
	if (!srcInfo.m_supportedAsRTAndTexture && dstInfo.m_supportedAsRTAndTexture)
		srcMapping.m_rtTexFormat.m_fallback = dst;
}

void glDevice_CheckFormats()
{
	glCaps::FormatInfo texOnlyInfo;
	texOnlyInfo.m_supportedAsTextureOnly = UE_TRUE;
	texOnlyInfo.m_supportsAutoMips = UE_TRUE;

	// Check support for buffer formats

#ifdef GL_OPENGL_ES
	glDevice_CheckFormat(glBufferFormat_PVRTC2_RGB, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, GL_NONE, GL_NONE);
	glDevice_CheckFormat(glBufferFormat_PVRTC2_RGBA, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GL_NONE, GL_NONE);
	glDevice_CheckFormat(glBufferFormat_PVRTC4_RGB, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, GL_NONE, GL_NONE);
	glDevice_CheckFormat(glBufferFormat_PVRTC4_RGBA, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_NONE, GL_NONE);
	glDevice_CheckFormat(glBufferFormat_A8, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_R8G8B8, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_R8G8B8A8, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_R4G4B4A4, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
	glDevice_CheckFormat(glBufferFormat_R16G16B16A16F, GL_RGBA, GL_RGBA, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32G32B32A32F, GL_RGBA, GL_RGBA, GL_FLOAT);
#else
	glDevice_CheckFormat(glBufferFormat_A8, GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_A16, GL_ALPHA16, GL_ALPHA, GL_UNSIGNED_SHORT);
	glDevice_CheckFormat(glBufferFormat_B5G6R5, GL_RGB5, GL_BGR, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_B8G8R8, GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_B4G4R4A4, GL_RGBA4, GL_BGRA, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_B5G5R5A1, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_B8G8R8A8, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE);
	glDevice_CheckFormat(glBufferFormat_R16F, GL_R16F, GL_RED, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R16F, GL_LUMINANCE16F_ARB, GL_LUMINANCE, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R16G16F, GL_RG16F, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R16G16F, GL_LUMINANCE_ALPHA16F_ARB, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R16G16B16A16F, GL_RGBA16F_ARB, GL_RGBA, GL_HALF_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32F, GL_R32F, GL_RED, GL_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32F, GL_LUMINANCE32F_ARB, GL_RED, GL_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32G32F, GL_RG32F, GL_LUMINANCE_ALPHA, GL_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32G32F, GL_LUMINANCE_ALPHA32F_ARB, GL_LUMINANCE_ALPHA, GL_FLOAT);
	glDevice_CheckFormat(glBufferFormat_R32G32B32A32F, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT);
#endif
	glDevice_CheckFormat(glBufferFormat_DXT1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_NONE, GL_NONE, &texOnlyInfo);
	glDevice_CheckFormat(glBufferFormat_DXT1a, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_NONE, GL_NONE, &texOnlyInfo);
	glDevice_CheckFormat(glBufferFormat_DXT3, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_NONE, GL_NONE, &texOnlyInfo);
	glDevice_CheckFormat(glBufferFormat_DXT5, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_NONE, GL_NONE, &texOnlyInfo);
	glDevice_CheckFormat(glBufferFormat_D16, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT);
	glDevice_CheckFormat(glBufferFormat_D24, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8);
	glDevice_CheckFormat(glBufferFormat_D24S8, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
	glDevice_CheckFormat(glBufferFormat_D32, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
	glDevice_CheckFormat(glBufferFormat_Depth, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

	// Set up default format fallbacks

	glDevice_AddFallback(glBufferFormat_R32F, glBufferFormat_R32G32F);
	glDevice_AddFallback(glBufferFormat_R32F, glBufferFormat_R32G32B32A32F);
	glDevice_AddFallback(glBufferFormat_R16F, glBufferFormat_R16G16F);
	glDevice_AddFallback(glBufferFormat_R16F, glBufferFormat_R16G16B16A16F);
	glDevice_AddFallback(glBufferFormat_R16G16F, glBufferFormat_R16G16B16A16F);
	glDevice_AddFallback(glBufferFormat_Depth, glBufferFormat_D24S8);
	glDevice_AddFallback(glBufferFormat_Depth, glBufferFormat_D32);
}

void glDevice_PreStartup()
{
	UE_ASSERT(!glDevice_IsSet());
	glDevice_Set(&s_dev);
	glUtils_BuildAdapterList();
}

void glDevice_Startup(const glDeviceStartupParams* params)
{
	UE_ASSERT_MSG(glDevice_IsSet(), "PreStartup was not called before.");

	s_dev.m_params = *params;

	glDevice_CalcMonitorAspectRatio(params);

	glSamplerDesc_InitializeDefaults();
	glDevice_Base_Startup(params);

#ifndef GL_OPENGL_FIXED_PIPELINE
	glUtils_InitCgSemanticsMap();

	UE_ASSERT_FUNC(s_dev.m_programs.Init(GL_STACK_ALLOC, params->m_maxPrograms));
	UE_ASSERT_FUNC(s_dev.m_fboSupportCache.Init(GL_STACK_ALLOC, 256));
#endif

#if defined(UE_MARMALADE)

	if (!IwGLInit())
		ueFatalError("Failed to initialize OpenGL ES via IwGL");

#else

	glDevice_StartupPlatform(params);

#endif

	// Output some stats

	glDevice_PrintStats();

	// Set up caps

	for (u32 i = 0; i < glShaderType_MAX; i++)
		s_dev.m_caps.m_maxShaderRegisters[i] = 0; // No access to registers in GLSL

	s_dev.m_caps.m_supportsTwoSidedStencil = UE_FALSE;

	s_dev.m_caps.m_maxColorRenderTargets = GL_MAX_COLOR_RENDER_TARGETS;

	GLint maxSamplers;

	GL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &maxSamplers));
	s_dev.m_caps.m_maxSamplers[glShaderType_Fragment] = maxSamplers;

	GL(glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxSamplers));
	s_dev.m_caps.m_maxSamplers[glShaderType_Vertex] = 0;//maxSamplers; FIXME: needs fragment/vertex shader shared samplers set
	s_dev.m_caps.m_maxSamplers[glShaderType_Geometry] = 0;

	s_dev.m_caps.m_maxVertexStreams = GL_MAX_BOUND_VERTEX_STREAMS;

	s_dev.m_caps.m_maxAnisotropicFilteringlevel = 1;
	if (GLEW_EXT_texture_filter_anisotropic)
	{
		GLint aniso = 1;
		GL(glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
		s_dev.m_caps.m_maxAnisotropicFilteringlevel = aniso;
	}

#ifdef GL_OPENGL_ES
	s_dev.m_caps.m_maxUserClippingPlanes = 0;
#else
	GLint clippingPlanes = 6;
	GL(glGetIntegerv(GL_MAX_CLIP_PLANES, &clippingPlanes));
	UE_ASSERT(clippingPlanes >= 6);
	s_dev.m_caps.m_maxUserClippingPlanes = clippingPlanes - 6;
#endif

#ifdef GL_OPENGL_ES
	s_dev.m_caps.m_supportsOcclusionQuery = UE_FALSE;
	s_dev.m_caps.m_supportsOcclusionQueryAnySample = GLEW_EXT_occlusion_query_boolean;
#else
	s_dev.m_caps.m_supportsOcclusionQuery = GLEW_ARB_occlusion_query ? UE_TRUE : UE_FALSE;
	s_dev.m_caps.m_supportsOcclusionQueryAnySample = s_dev.m_caps.m_supportsOcclusionQuery;
#endif
	s_dev.m_caps.m_supportsOcclusionPredicateQuery = GLEW_VERSION_3_0 ? UE_TRUE : UE_FALSE;
	s_dev.m_caps.m_supportsIndependentColorWriteMasks = UE_FALSE; // FIXME: How to?

	glDevice_CheckFormats();
	glUtils_ListSupportedFormats();

	s_dev.m_caps.m_maxPrimitives = 1 << 20;

	// Create default render context

	s_dev.m_defCtx = new(s_dev.m_contextsPool) glCtx;
	glCtx_Init(s_dev.m_defCtx);

	// Create FBOs used internally

	if (GLEW_EXT_framebuffer_object)
	{
		GL(glGenFramebuffersEXT(1, &s_dev.m_defCtx->m_fbo));
		GL(glGenFramebuffersEXT(1, &s_dev.m_defCtx->m_fbo2));
	}

	// Startup necessary subsystems

	glShaderConstantMgr_Startup();
	glRenderBufferPool_Startup();
	glPBO_Startup();

	// Insert back buffer into pool

	glTextureBufferDesc backBufferDesc;
	backBufferDesc.m_width = params->m_width;
	backBufferDesc.m_height = params->m_height;
	backBufferDesc.m_format = params->m_colorFormat;
	backBufferDesc.m_numLevels = 1;
	backBufferDesc.m_flags = glTextureBufferFlags_IsRenderTarget;
	backBufferDesc.m_rt.m_MSAALevel = s_dev.m_params.m_MSAALevel;

	s_dev.m_backBuffer = new(s_dev.m_textureBuffersPool) glTextureBuffer;
	s_dev.m_backBuffer->m_desc = backBufferDesc;
	s_dev.m_backBuffer->m_textureHandle = 0;
	s_dev.m_backBuffer->m_renderBufferHandle = 0;
	s_dev.m_backBuffer->m_isMainFrameBuffer = UE_TRUE;

	glRenderBufferPool_ReleaseTextureBuffer(s_dev.m_backBuffer);

	// Insert depth stencil buffer into pool

	glTextureBufferDesc depthStencilBufferDesc;
	depthStencilBufferDesc.m_width = params->m_width;
	depthStencilBufferDesc.m_height = params->m_height;
	depthStencilBufferDesc.m_format = params->m_depthStencilFormat;
	depthStencilBufferDesc.m_numLevels = 1;
	depthStencilBufferDesc.m_flags = glTextureBufferFlags_IsRenderTarget;
	depthStencilBufferDesc.m_rt.m_MSAALevel = s_dev.m_params.m_MSAALevel;

	s_dev.m_depthStencilBuffer = new(s_dev.m_textureBuffersPool) glTextureBuffer;
	s_dev.m_depthStencilBuffer->m_desc = depthStencilBufferDesc;
	s_dev.m_depthStencilBuffer->m_textureHandle = 0;
	s_dev.m_depthStencilBuffer->m_renderBufferHandle = 0;
	s_dev.m_depthStencilBuffer->m_isMainFrameBuffer = UE_TRUE;

	glRenderBufferPool_ReleaseTextureBuffer(s_dev.m_depthStencilBuffer);

	// Create main render group

	glRenderGroupDesc rgDesc;
	rgDesc.m_width = params->m_width;
	rgDesc.m_height = params->m_height;
	rgDesc.m_MSAALevel = s_dev.m_params.m_MSAALevel;
	rgDesc.m_numColorBuffers = 1;
	rgDesc.m_colorDesc[0].m_format = params->m_colorFormat;
	rgDesc.m_depthStencilDesc.m_format = params->m_depthStencilFormat;
	rgDesc.m_hasDepthStencil = UE_TRUE;
	rgDesc.m_needColorContent = UE_FALSE;
	rgDesc.m_needDepthStencilContent = UE_FALSE;
	rgDesc.m_isMainFrameBuffer = UE_TRUE;
	s_dev.m_mainRenderGroup = s_dev.m_defCtx->m_renderGroup = glRenderGroup_Create(&rgDesc);

	// Reset default context

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_All);

	// Set some other OpenGL defaults

	GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
	GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL(glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST));
}

void glDevice_Shutdown()
{
	UE_ASSERT(glDevice_IsSet());

	// Release all D3D9 resources

	GL(glFlush());
	GL(glFinish());

	glCtx_Reset(s_dev.m_defCtx, glResetFlags_Resources);

	if (GLEW_EXT_framebuffer_object)
	{
		GL(glDeleteFramebuffersEXT(1, &s_dev.m_defCtx->m_fbo));
		GL(glDeleteFramebuffersEXT(1, &s_dev.m_defCtx->m_fbo2));
	}

	glRenderGroup_Destroy(s_dev.m_mainRenderGroup);
	s_dev.m_mainRenderGroup = NULL;

	// Shut down all subsystems

	glPBO_Shutdown();
	glRenderBufferPool_Shutdown();
	glShaderConstantMgr_Shutdown();

	// Destroy default context

	glCtx_Deinit(s_dev.m_defCtx);
	s_dev.m_contextsPool.Free(s_dev.m_defCtx);

	// Shut down device

	s_dev.m_fboSupportCache.Deinit();
	s_dev.m_programs.Deinit();
	glUtils_DeinitCgSemanticsMap();
	glDevice_Base_Shutdown();

	glDevice_Set(NULL);
}

u32 glDevice_GetUsedTextureMemory()
{
	return 0;
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
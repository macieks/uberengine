#include "Base/ueBase.h"

#if defined(GL_OPENGL) && defined(GL_OPENGL_ES)

bool GLEW_EXT_framebuffer_multisample = false;
bool GLEW_EXT_texture_filter_anisotropic = false;
bool GLEW_EXT_framebuffer_blit = false;
bool GLEW_EXT_occlusion_query_boolean = false;

#include "Base/ueSorting.h"
#include "Utils/ueWindow.h"
#include "Graphics/glLib_Private.h"

void glDevice_DetectMSAAModes(const glDeviceStartupParams* params)
{
	GLDEV->m_caps.m_numMSAALevels = 1;
	GLDEV->m_caps.m_MSAALevels[0] = 1;

	EGLint configAttribList[] =
	{
		EGL_RED_SIZE,       params->m_redBits,
		EGL_GREEN_SIZE,     params->m_greenBits,
		EGL_BLUE_SIZE,      params->m_blueBits,
		EGL_ALPHA_SIZE,     params->m_alphaBits,
		EGL_DEPTH_SIZE,     params->m_depthBits,
		EGL_STENCIL_SIZE,   params->m_stencilBits,
		EGL_SAMPLE_BUFFERS, 1,
		EGL_SAMPLES, 2,
		EGL_NONE
	};

	EGLint surfaceAttribList[] =
	{
		EGL_NONE, EGL_NONE
	};

	EGLConfig configs[256];
	EGLint numConfigs;
	if (!eglChooseConfig(GLDEV->m_EGLDisplay, configAttribList, configs, UE_ARRAY_SIZE(configs), &numConfigs))
		return;

	for (EGLint i = 0; i < numConfigs; i++)
	{
		EGLint samples;
		if (!eglGetConfigAttrib(GLDEV->m_EGLDisplay, configs[i], EGL_SAMPLES, &samples) || samples < 2)
			continue;

		ueArray_PushBackUnique<u32>(GLDEV->m_caps.m_MSAALevels, UE_ARRAY_SIZE(GLDEV->m_caps.m_MSAALevels), GLDEV->m_caps.m_numMSAALevels, samples);
	}
}

void glDevice_StartupPlatform(const glDeviceStartupParams* params)
{
#ifdef UE_WIN32
	HWND window = ueWindow_GetHWND(params->m_window);
	GLDEV->m_displayType = GetDC(window);
#else
	#error
#endif
	if (!GLDEV->m_displayType)
		ueFatalError("Failed to get valid native OpenGLES display type");

	// Get the default display.

	GLDEV->m_EGLDisplay = eglGetDisplay(GLDEV->m_displayType);

    if (GLDEV->m_EGLDisplay == EGL_NO_DISPLAY)
        GLDEV->m_EGLDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);

    // Initialize EGL

    EGLint iMajorVersion, iMinorVersion;

	if (!eglInitialize(GLDEV->m_EGLDisplay, &iMajorVersion, &iMinorVersion))
        ueFatalError("eglInitialize failed");

	// Determine supported MSAA modes

	glDevice_DetectMSAAModes(params);

    // Specify the required configuration attributes.

	u32 MSAALevel = 1;

	EGLint configAttribList[] =
	{
		EGL_RED_SIZE,       params->m_redBits,
		EGL_GREEN_SIZE,     params->m_greenBits,
		EGL_BLUE_SIZE,      params->m_blueBits,
		EGL_ALPHA_SIZE,     params->m_alphaBits,
		EGL_DEPTH_SIZE,     params->m_depthBits,
		EGL_STENCIL_SIZE,   params->m_stencilBits,
		EGL_SAMPLE_BUFFERS, 0,
		EGL_NONE
	};

    // Choose matching config

	EGLint numConfigs;
	if (!eglChooseConfig(GLDEV->m_EGLDisplay, configAttribList, &GLDEV->m_EGLConfig, 1, &numConfigs) || (numConfigs != 1))
		ueFatalError("eglChooseConfig failed");

    // Create a surface to draw to

	EGLint surfaceAttribList[] =
	{
		EGL_NONE, EGL_NONE
	};

	GLDEV->m_EGLSurface = eglCreateWindowSurface(GLDEV->m_EGLDisplay, GLDEV->m_EGLConfig, window, surfaceAttribList);
    if (GLDEV->m_EGLSurface == EGL_NO_SURFACE)
    {
        eglGetError(); // Clear error and try again.
        GLDEV->m_EGLSurface = eglCreateWindowSurface(GLDEV->m_EGLDisplay, GLDEV->m_EGLConfig, NULL, surfaceAttribList);
		if (!GLDEV->m_EGLSurface)
			ueFatalError("eglCreateWindowSurface failed");
    }

    // Bind the API ( It could be OpenGLES or OpenVG )

	eglBindAPI(EGL_OPENGL_ES_API);

    // Create a context

	EGLint contextAttribList[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE
    };

	GLDEV->m_EGLContext = eglCreateContext(GLDEV->m_EGLDisplay, GLDEV->m_EGLConfig, EGL_NO_CONTEXT, contextAttribList);
	if (!GLDEV->m_EGLContext)
		ueFatalError("eglCreateContext failed");

    // Bind the context to the current thread and use our window surface for 
    // drawing and reading

    if (!eglMakeCurrent(GLDEV->m_EGLDisplay, GLDEV->m_EGLSurface, GLDEV->m_EGLSurface, GLDEV->m_EGLContext))
		ueFatalError("eglMakeCurrent failed");

	// Initialize required extensions

	const char* extensions = (const char*) glGetString(GL_EXTENSIONS);

	GLEW_EXT_framebuffer_multisample = !!ueStrStr(extensions, "GL_ANGLE_framebuffer_multisample");
	GLEW_EXT_texture_filter_anisotropic  = !!ueStrStr(extensions, "GL_EXT_texture_filter_anisotropic");
	GLEW_EXT_framebuffer_blit = !!ueStrStr(extensions, "GL_ANGLE_framebuffer_blit");
	GLEW_EXT_occlusion_query_boolean = !!ueStrStr(extensions, "GL_EXT_occlusion_query_boolean");
}

#else
	UE_NO_EMPTY_FILE
#endif
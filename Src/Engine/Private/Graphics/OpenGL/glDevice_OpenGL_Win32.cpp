#include "Base/ueBase.h"

#if defined(GL_OPENGL) && !defined(GL_OPENGL_ES) && defined(UE_WIN32)

#include "Base/ueSorting.h"
#include "Utils/ueWindow.h"
#include "Graphics/glLib_Private.h"

LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(hwnd, umsg, wp, lp);
}

void glDevice_DetectMSAAModes()
{
	const char* dummyClassWindowName = "UberEngineGL";

	WNDCLASS dummyClass;
	memset(&dummyClass, 0, sizeof(WNDCLASS));
	dummyClass.style = CS_OWNDC;
	dummyClass.lpfnWndProc = dummyWndProc;
	dummyClass.lpszClassName = dummyClassWindowName;
	RegisterClass(&dummyClass);

	HWND hwnd = CreateWindow(dummyClassWindowName, dummyClassWindowName,
		WS_POPUP | WS_CLIPCHILDREN,
		0, 0, 32, 32, 0, 0, NULL, 0);
	if (!hwnd)
		ueFatalError("CreateWindow failed");

	HDC hdc = GetDC(hwnd); 

	// Set a simple OpenGL pixel format that everyone supports

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 15;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	
	// If these fail, wglCreateContext will also quietly fail

	INT format;
	if ((format = ChoosePixelFormat(hdc, &pfd)) != 0)
		SetPixelFormat(hdc, format, &pfd);

	GLDEV->m_caps.m_numMSAALevels = 1;
	GLDEV->m_caps.m_MSAALevels[0] = 1;

	HGLRC hrc = wglCreateContext(hdc);
	if (hrc)
	{
		HGLRC oldrc = wglGetCurrentContext();
		HDC oldhdc = wglGetCurrentDC();
		wglMakeCurrent(hdc, hrc);
		
		ueBool hasPixelFormatARB = UE_FALSE;
		ueBool hasMSAA = UE_FALSE;

		// Check for pixel format and MSAA support

		PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");
		if (_wglGetExtensionsStringARB)
		{
			const char* extensions = _wglGetExtensionsStringARB(hdc);

			if (ueStrStr(extensions, "WGL_ARB_pixel_format"))
				hasPixelFormatARB = UE_TRUE;
			if (ueStrStr(extensions, "WGL_ARB_multisample"))
				hasMSAA = UE_TRUE;
		}

		if (hasPixelFormatARB && hasMSAA)
		{
			// Enumerate all formats with MSAA

			static const int iattr[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_SAMPLES_ARB, 2,
				0
			};

			int formats[256];
			unsigned int count;

			// Cheating here.  wglChoosePixelFormatARB procc address needed later on
            // when a valid GL context does not exist and glew is not initialized yet.

			WGLEW_GET_FUN(__wglewChoosePixelFormatARB) = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
            if (WGLEW_GET_FUN(__wglewChoosePixelFormatARB)(hdc, iattr, 0, 256, formats, &count))
            {
				PFNWGLGETPIXELFORMATATTRIBIVARBPROC _wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC) wglGetProcAddress("wglGetPixelFormatAttribivARB");

                int query = WGL_SAMPLES_ARB, samples;
                for (unsigned int i = 0; i < count; ++i)
                    if (_wglGetPixelFormatAttribivARB(hdc, formats[i], 0, 1, &query, &samples))
						ueArray_PushBackUnique<u32>(GLDEV->m_caps.m_MSAALevels, UE_ARRAY_SIZE(GLDEV->m_caps.m_MSAALevels), GLDEV->m_caps.m_numMSAALevels, samples);
            }
		}
		
		wglMakeCurrent(oldhdc, oldrc);
		wglDeleteContext(hrc);
	}

	// Clean up

	DestroyWindow(hwnd);
	UnregisterClass(dummyClassWindowName, NULL);
}

void glDevice_StartupPlatform(const glDeviceStartupParams* params)
{
	glDevice_DetectMSAAModes();

	// Select MSAA level

	u32 MSAALevel = 1;
	for (u32 i = 0; i < GLDEV->m_caps.m_numMSAALevels; i++)
		if (GLDEV->m_caps.m_MSAALevels[i] == params->m_MSAALevel)
		{
			MSAALevel = params->m_MSAALevel;
			break;
		}
	GLDEV->m_params.m_MSAALevel = MSAALevel;

	// Set up pixel format descriptor

	PIXELFORMATDESCRIPTOR pfd =
	{ 
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
		1,                     // version number  
		PFD_DRAW_TO_WINDOW |   // support window  
		PFD_SUPPORT_OPENGL |   // support OpenGL  
		PFD_DOUBLEBUFFER,      // double buffered  
		PFD_TYPE_RGBA,         // RGBA type  
		params->m_colorBits,
		0, 0, 0, 0, 0, 0,      // color bits ignored  
		0,                     // no alpha buffer  
		0,                     // shift bit ignored  
		0,                     // no accumulation buffer  
		0, 0, 0, 0,            // accum bits ignored  
		params->m_depthBits,
		params->m_stencilBits,
		0,                     // no auxiliary buffer  
		PFD_MAIN_PLANE,        // main layer  
		0,                     // reserved  
		0, 0, 0                // layer masks ignored  
	}; 

	// Set up WGL attributes

	INT wglAttribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB,  params->m_backBufferCount == 1 ? GL_FALSE : GL_TRUE,
		WGL_RED_BITS_ARB,       params->m_redBits,
		WGL_GREEN_BITS_ARB,     params->m_greenBits,
		WGL_BLUE_BITS_ARB,      params->m_blueBits,
		WGL_ALPHA_BITS_ARB,     params->m_alphaBits,
		WGL_DEPTH_BITS_ARB,     params->m_depthBits,
		WGL_STENCIL_BITS_ARB,   params->m_stencilBits,
		WGL_SAMPLE_BUFFERS_ARB,	MSAALevel > 1 ? GL_TRUE : GL_FALSE,
		WGL_SAMPLES_ARB,		MSAALevel > 1 ? MSAALevel : 0,
		0,
	};

	// Get device context for main window

	GLDEV->m_hdc = GetDC(ueWindow_GetHWND(params->m_window));
	UE_ASSERT(GLDEV->m_hdc);

	// Set pixel format for desired MSAA level

	INT bestPixelFormat = 0;
	if (MSAALevel == 1)
		bestPixelFormat = ChoosePixelFormat(GLDEV->m_hdc, &pfd);
	else
	{
		UINT numPixelFormats = 0;
		UE_ASSERT_FUNC(WGLEW_GET_FUN(__wglewChoosePixelFormatARB)(GLDEV->m_hdc, wglAttribs, NULL, 1, &bestPixelFormat, &numPixelFormats));
		if (!numPixelFormats)
			ueFatalErrorP("wglewChoosePixelFormatARB returned no valid formats (MSAA x %u)", MSAALevel);
	}
	if (!SetPixelFormat(GLDEV->m_hdc, bestPixelFormat, &pfd))
		ueFatalErrorP("Failed to select pixel format, pf = %u, (MSAA x %u)", bestPixelFormat, MSAALevel);

	// Create OpenGL context

	GLDEV->m_glContext = wglCreateContext(GLDEV->m_hdc);
	wglMakeCurrent(GLDEV->m_hdc, GLDEV->m_glContext);

	// Init GLEW

	const GLenum glewInitRet = glewInit();
	if (glewInitRet != GLEW_OK)
		ueFatalErrorP("GLEW initialization failed, ret = %u", (u32) glewInitRet);

	// Check required features

	if (!GLEW_VERSION_2_0)
		ueFatalErrorP("Graphics card doesn't support OpenGL 2.0 (your version is: %s)", glGetString(GL_VERSION));
	if (!GLEW_EXT_framebuffer_object)
		ueFatalError("Graphics card doesn't support OpenGL EXT_framebuffer_object extension");
	if (!GLEW_ARB_fragment_shader || !GLEW_ARB_vertex_shader)
		ueFatalError("Graphics card doesn't support OpenGL ARB_fragment_shader and ARB_vertex_shader extensions");
	if (!GLEW_ARB_draw_buffers)
		ueFatalError("Graphics card doesn't support OpenGL ARB_draw_buffers extension");
}

#else
	UE_NO_EMPTY_FILE
#endif
#include "Base/ueBase.h"
#include "Base/Allocators/ueMemoryDebugger.h"
#include "Base/Allocators/ueMallocAllocator.h"
#include "Base/Allocators/ueDLMallocAllocator.h"
#include "Base/Allocators/ueStackAllocator.h"
#include "Base/ueWindow.h"
#include "Base/ueRand.h"
#include "Base/ueJobSys.h"
#include "Input/inSys.h"
#include "Utils/utCompression.h"
#include "Utils/utAssetCompilerHelper.h"
#include "IO/ioFile.h"
#include "Audio/auLib.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShader.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxEffect.h"
#include "GraphicsExt/gxFont.h"
#include "ueDefaultApp.h"

#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC)
	extern void ueRegisterCompressionLib_ZLIB();
#endif

struct ueDefaultApp
{
	ueBool m_quit;

	u32 m_argc;
	const char** m_argv;

	ueMallocAllocator m_systemAllocator;
	ueStackAllocator m_stackAllocator;
	ueDLMallocAllocator m_frequentAllocator;
	
	ueAppTimer m_timer;

#if defined(UE_HAS_WINDOWS)
	ueWindow* m_window;
#endif

#if !defined(UE_FINAL)
	ueLogFile m_logFile;
#endif

	ueDefaultApp() :
		m_quit(UE_FALSE),
		m_argc(0),
		m_argv(NULL),
		m_window(NULL)
	{}
};

static ueDefaultApp g_app;

void ueDefaultApp_FatalErrorHandler(const char* file, u32 line, u32 errorCode, const char* msg)
{
	char msgBuffer[1 << 12];
	ueStrFormatS(msgBuffer, "Fatal error occured.\n\nDescription:\n%s:%d (code: %u)\n%s\n\nFor more details check log file.\nApplication will now be closed.", file, line, errorCode, msg);

#if defined(UE_HAS_WINDOWS)
	if (g_app.m_window)
		ueWindow_Minimize(g_app.m_window);
#endif

	ueWindowUtils_ShowMessageBox(NULL, msgBuffer, "Application Error", ueWindowMsgBoxType_Error);
	ueExit(1);
}

s32 ueDefaultApp_Start(u32 numArgs, const char** args);
{
	// Initialize only basic subsystems

	ueSetFatalErrorHandler(ueDefaultApp_FatalErrorHandler);
	ueCallstackHelper_Startup();
	ueClock_Startup();
	ueLogger_Startup();
	ueAssets_Startup(UE_FALSE);
	ueAssets_SetLanguageSymbol(params->m_languageSymbol);

	// User initialization

	ueDefaultApp_Init();

	// Run the game

	ueDefaultApp_MainLoop();

	// User deinitialization

	ueDefaultApp_Deinit();

	// Deinitialize basic subsystems

	ueAssets_Shutdown();
	ueLogger_Shutdown();
	ueClock_Shutdown();
	ueCallstackHelper_Shutdown();

	return result;
}

void ueDefaultApp_InitBase(ueDefaultAppInitParams* params)
{
	// Memory allocators

	g_app.m_systemAllocator.SetName("system");

	void* freqHeapMemory = g_app.m_systemAllocator.Alloc(params->m_frequentAllocatorSize, 4);
	if (!freqHeapMemory)
	{
		ueLogE("Failed to allocate %u bytes for frequent allocator", params->m_frequentAllocatorSize);
		ueExit(1);
	}
	g_app.m_freqAllocator.InitMem(freqHeapMemory, freqHeapSize);
	g_app.m_freqAllocator.SetName("frequent-global");

	void* stackHeapMemory = g_app.m_systemAllocator.Alloc(params->m_stackAllocatorSize, 4);
	if (!stackHeapMemory)
	{
		ueLogE("Failed to allocate %u bytes for stack allocator", params->m_stackAllocatorSize);
		ueExit(1);
	}
	u32 stackHeapMaxAllocs = 0;
	ueSize stackHeapDebugMemorySize = 0;
	void* stackHeapDebugMemory = NULL;
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	stackHeapMaxAllocs = 1 << 12;
	stackHeapDebugMemorySize = ueStackAllocator::CalcDebugMemReq(stackHeapMaxAllocs);
	stackHeapDebugMemory = g_app.m_sysAllocator.Alloc(stackHeapDebugMemorySize, 4);
#endif
	g_app.m_stackAllocator.InitMem(stackHeapMemory, stackHeapSize, stackHeapMaxAllocs, stackHeapDebugMemory, stackHeapDebugMemorySize);
	g_app.m_stackAllocator.SetName("stack-global");

	// Memory debugger

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	g_app.m_memoryDebugger = ueMemoryDebugger_Create(&g_app.m_stackAllocator, 1 << 12, 16);
	ueMemoryDebugger_Begin(g_app.m_memoryDebugger);
	ueMemoryDebugger_AddAllocatorToTrack(g_app.m_memoryDebugger, &g_app.m_freqAllocator);
	ueMemoryDebugger_AddAllocatorToTrack(g_app.m_memoryDebugger, &g_app.m_stackAllocator);
#endif

	// Random numbers generator

	ueRandMgr_Startup(&g_app.m_stackAllocator);

	// Global job system

	ueJobSysThreadDesc jobsysThreadDesc;
	jobsysThreadDesc.m_name = "global-job-sys";

	ueJobSysParams jobsysParams;
	jobsysParams.m_stackAllocator = &g_app.m_stackAllocator;
	jobsysParams.m_numThreads = 1;
	jobsysParams.m_threadDescs = &jobsysThreadDesc;

	thGlobalJobSys_Startup(&jobsysParams);

	// Profiler

#if defined(UE_ENABLE_PROFILER)
	ueProfiler_Startup(&g_app.m_stackAllocator);
#endif

	utCompression_Startup(&g_app.m_stackAllocator);
#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC)
	ueRegisterCompressionLib_ZLIB();
#endif

	ioFileSys_Startup(&g_app.m_stackAllocator);

	// Native file system pointing to bin directory

	ioNativeFileSysParams fsParams;
	fsParams.m_allocator = &g_app.m_freqAllocator;
	fsParams.m_maxFilesOpen = 16;
	fsParams.m_root = "";
	ioFileSys* binFileSys = ioFileSys_CreateNative(&fsParams);
	if (!binFileSys)
	{
		ueLogE("Failed to create 'binary' file system");
		ueExit(1);
	}

	ioFileSys_Mount(binFileSys, "", 0);
	ioFileSys_SetDefault(binFileSys);

	// Data file systems

	if (params->m_afsDataFile)
	{
		ioArchiveFileSysParams afsParams;
		afsParams.m_allocator = &g_app.m_freqAllocator;
		afsParams.m_maxFilesOpen = 16;
		afsParams.m_path = params->m_afsDataFile;
		ioFileSys* afsFileSys = ioFileSys_CreateAFS(&afsParams);
		if (!afsFileSys)
		{
			ueLogE("Failed to create 'afs' file system from file '%s'", params->m_afsDataFile);
			ueExit(1);
		}

		ioFileSys_Mount(afsFileSys, "", 0);
		ioFileSys_SetDataFileSys(afsFileSys);
	}
	else
	{
		// Native file system with data

		fsParams.m_maxFilesOpen = 16;
		fsParams.m_root = "../../Data/Exp/";
		ioFileSys* expFileSys = ioFileSys_CreateNative(&fsParams);
		if (!expFileSys)
		{
			ueLogE("Failed to create 'data' file system at '%s'", fsParams.m_root);
			ueExit(1);
		}

		ioFileSys_Mount(expFileSys, "", 0);
		ioFileSys_SetDataFileSys(expFileSys);

		// Native file system with source data

		fsParams.m_maxFilesOpen = 4;
		fsParams.m_root = "../../Data/Src/";
		ioFileSys* srcFileSys = ioFileSys_CreateNative(&fsParams);
		if (!srcFileSys)
		{
			ueLogE("Failed to create 'source data' file system at '%s'", fsParams.m_root);
			ueExit(1);
		}

		ioFileSys_Mount(srcFileSys, "", 1);
		ioFileSys_SetSourceFileSys(srcFileSys);
	}

#ifndef UE_FINAL
	g_app.logFile.Init("game.log");
#endif
	ueLogSystemInfo();

	ueEnv_Startup(&g_app.m_stackAllocator, 512, 512);
	ueAppVars::Startup();

	ueLocaleMgr_Startup(&g_app.m_freqAllocator, 1);
	ueLocaleMgr_SetRootDir("game/Texts");
	ueLocaleMgr_LoadSet("Game");

	uePath configFilePath;
	ueAssets_GetAssetPath(configFilePath, "game_config", "txt");
	UE_ASSERT_FUNC( ueEnv_DoFile(configFilePath) );

	if (params->m_buildAssets && !ueApp_GetArgByName("no_build_assets"))
	{
		s32 errorCode;
		utAssetCompilerHelperConfig config;
		//config.m_enableAssetWatcher = UE_TRUE;
		if (!utAssetCompilerHelper_Run(&config, &errorCode))
		{
			ueLogE("Failed to build assets");
			logFile.Deinit();
			ueExit(1);
		}
	}

	u32 initWidth, initHeight;
	ueAppVars::GetResolution(initWidth, initHeight);

#if defined(UE_HAS_WINDOWS)
	ueWindowParams windowParams;
	windowParams.m_allocator = &g_app.m_stackAllocator;
	windowParams.m_rect = ueRectI(50, 50, initWidth, initHeight);
	windowParams.m_minWidth = 64;
	windowParams.m_minHeight = 64;
	windowParams.m_maxWidth = 4096;
	windowParams.m_maxHeight = 4096;
	windowParams.m_fullscreen = ueVar_GetBool(ueAppVars::m_fullscreen);
	ueStrCpyS(windowParams.m_name, params->m_appName);
#if defined(UE_WIN32)
	ueStrCpyS(windowParams.m_className, "UberEngine");
	windowParams.m_hInstance = NULL;
#endif
	windowParams.m_hideCursorWhenActive = UE_TRUE;
	windowParams.m_resizeable = UE_TRUE;
	windowParams.m_maximizable = UE_TRUE;
	windowParams.m_minimizable = UE_TRUE;

	g_app.m_window = ueWindow_Create(&windowParams);
	if (!g_app.m_window)
	{
		logFile.Deinit();
		ueExit(1);
	}

	ueWindow_SetMain(g_app.m_window);
#endif // defined(UE_HAS_WINDOWS)

	glDevice_PreStartup();

	glDeviceStartupParams glParams;
	glParams.m_stackAllocator = &g_app.m_stackAllocator;
	glParams.m_freqAllocator = &g_app.m_freqAllocator;
	glParams.m_width = initWidth;
	glParams.m_height = initHeight;
#if defined(UE_HAS_WINDOWS)
	glParams.m_fullscreen = ueVar_GetBool(ueAppVars::m_fullscreen);
	glParams.m_window = m_window;
#endif // defined(UE_HAS_WINDOWS)
	glParams.m_colorFormat = glBufferFormat_Native_R8G8B8A8;
	glParams.m_depthStencilFormat = glBufferFormat_D24S8;
#if defined(UE_FINAL)
	glParams.m_displaySync = glDisplaySync_Vertical;
#else
	glParams.m_displaySync = glDisplaySync_None;
#endif
	glParams.m_backBufferCount = 2;
	glParams.m_MSAALevel = ueVar_GetS32(ueAppVars::m_msaaLevel);
#if defined(GL_D3D9)
	glParams.m_adapterIndex = 0;
	glParams.m_frequency = 60;
	glParams.m_hardwareVertexProcessing = UE_TRUE;
	glParams.m_multithreaded = UE_FALSE;
	glParams.m_deviceType = D3DDEVTYPE_HAL;
#elif defined(GL_D3D11)
	glParams.m_adapterIndex = 0;
	glParams.m_frequency = 60;
#endif
#if defined(UE_HAS_WINDOWS)
	if (glParams.m_fullscreen)
		glDevice_GetClosestSupportedMode(&glParams);
#elif defined(UE_MARMALADE)
	glParams.m_maxTotalPBOSize = 1024 * 1024 * 4 / 3;
#endif
	glDevice_Startup(&glParams);
#if defined(GL_MARMALADE)
	{
		u32 screenWidth = IwGxGetDisplayWidth();
		u32 screenHeight = IwGxGetDisplayHeight();
		if (screenWidth < screenHeight)
		{
			const u32 tmp = screenWidth;
			screenWidth = screenHeight;
			screenHeight = tmp;
		}
		if (screenWidth <= 480 || screenHeight <= 360) // Equivalent of 1024x768 / 2 (4:3 aspect ratio)
			glDevice_EnableFakeTextureDownsize2x();
	}
#endif

#if !defined(GL_FIXED_PIPELINE)
	gxCommonConstants::Startup();
#endif

	// FIXME: This should be done each time actual viewport / render target changes
	glViewportDesc viewport;
	viewport.m_left = 0;
	viewport.m_top = 0;
	viewport.m_width = glParams.m_width;
	viewport.m_height = glParams.m_height;
	viewport.m_minZ = 0.0f;
	viewport.m_maxZ = 1.0f;
	glCtx_SetViewport(glDevice_GetDefaultContext(), viewport);

	// Timer

	g_app.m_timer.Init();

	// Input

	inKeyboard_Startup();
#if defined(IN_ENABLE_MOUSE)
	inMouse_Startup();
#endif
#if defined(IN_ENABLE_GAMEPAD)
	inGamepad_Startup();
#endif
#if defined(IN_ENABLE_TOUCHPAD)
	inTouchpad_Startup(UE_FALSE);
#endif

	inSysStartupParams insysParams;
	insysParams.m_stackAllocator = &g_app.m_stackAllocator;
	inSys_Startup(&insysParams);

	// Resource manager

	ueResourceMgr_Startup(&g_app.m_stackAllocator, 16, 1024);

	// Determine configs for all resource types

	const char* shaderConfig = NULL;
	const char* textureConfig = NULL;
	const char* fontConfig = NULL;
	const char* effectConfig = NULL;

#if defined(UE_WIN32)
	#if defined(GL_D3D9)
		shaderConfig = "win32-d3d9_sm30";
		textureConfig = "win32-d3d9";
		fontConfig = "win32-d3d9";
	#elif defined(GL_D3D11)
		shaderConfig = "win32-d3d11_sm40";
		textureConfig = "win32-d3d11";
		fontConfig = "win32-d3d11";
	#elif defined(GL_OPENGL_ES)
		shaderConfig = "win32-gles";
		textureConfig = "win32-gles";
		fontConfig = "win32-gles";
	#elif defined(GL_OPENGL)
		shaderConfig = "win32-glsl";
		textureConfig = "win32-gl";
		fontConfig = "win32-gl";
	#endif
	effectConfig = "win32";
#elif defined(UE_LINUX)
	shaderConfig = "linux-glsl";
	textureConfig = "linux-gl";
	fontConfig = "linux-gl";
	effectConfig = "linux";
#elif defined(UE_X360)
	shaderConfig = "x360";
	textureConfig = "x360";
	fontConfig = "x360";
	effectConfig = "x360";
#endif

	// Textures

	gxTextureMgr_Startup(&g_app.m_freqAllocator, textureConfig, UE_FALSE);

	// Shaders

#if !defined(GL_FIXED_PIPELINE)
	gxShaderMgr_Startup(&g_app.m_freqAllocator, shaderConfig);
#endif

	// Particle effects

	gxEffectTypeMgr_Startup(&g_app.m_freqAllocator, effectConfig, UE_FALSE);

	// Fonts and text rendering

	gxFontMgr_Startup(&g_app.m_freqAllocator, fontConfig);

	gxTextRendererStartupParams textRendererParams;
	textRendererParams.m_stackAllocator = &g_app.m_stackAllocator;
	textRendererParams.m_defaultFontName = NULL;
	gxTextRenderer_Startup(&textRendererParams);

	// Debug shape drawing

#if !defined(UE_MARMALADE)
	gxShapeDrawStartupParams shapeDrawParams;
	shapeDrawParams.m_maxBoxBatch = 0;
	shapeDrawParams.m_maxSphereBatch = 0;
	shapeDrawParams.m_maxLineBatch = 500;
	shapeDrawParams.m_maxTriangleBatch = 500;
	shapeDrawParams.m_stackAllocator = &g_app.m_stackAllocator;
	gxShapeDraw_Startup(&shapeDrawParams);
#endif

#if defined(IN_ENABLE_ACCELEROMETER)
	inAccelerometer_Startup();
	inAccelerometer_SetFixedPosition(inDevicePosition_LandscapeLeft);
#endif

	// Audio

	auLibStartupParams auLibParams;
	auLibParams.m_freqAllocator = &g_app.m_freqAllocator;
	auLibParams.m_stackAllocator = &g_app.m_stackAllocator;
	auLibParams.m_settingsFileName = "game/sound/game";
	auLib_Startup(&auLibParams);

	// Postprocessing

	gxMultiPostprocessStartupParams ppParams;
	ppParams.m_allocator = g_app.m_stackAllocator;
	gxMultiPostprocess_Startup(&ppParams);

	gxMultiPostprocess_SetEnabled(
		gxMultiPostprocessEffect_Bloom |
		gxMultiPostprocessEffect_CameraMotionBlur);

	// Camera

	g_app.m_camera.SetLookAt(ueVec3(3, 3, 3), ueVec3::Zero);
}

void ueDefaultApp_MainLoop()
{
	// Main loop

	while (!g_app.m_quit)
	{
		UE_PROF_SCOPE("Main Loop");

		// Update timer

		g_app.m_timer.Tick();
		const f32 dt = g_app.m_timer.DT();

		// Get whether to update input devices

		ueBool hasFocus = UE_TRUE;
#if defined(UE_HAS_WINDOWS)
		ueWindowUtils_HandleOSMessages();
		hasFocus = ueWindow_HasFocus(g_app.m_window);
#endif
		// Yield if app has no focus

#if defined(UE_HAS_WINDOWS)
		if (!hasFocus)
			ueThread_Sleep(50);
#endif

		// Check quit request

#if defined(UE_MARMALADE)
		if (s3eDeviceCheckQuitRequest())
			s3eDefaultApp_Quit();
#endif
		if (g_app.m_quit)
			break;

		// Update resources

		ueResourceMgr_DoWork(1.0f / 240.0f);

		// Update input system

		inSys_Update(updateInputDevices);

		// Update audio

		auLib_Update(dt);

		// Update post-processes

		if (m_updatePostprocesses)
		{
			gxMultiPostprocessUpdateParams ppParams;
			ppParams.m_dt = dt;
			ppParams.m_view = &m_camera.GetView();
			ppParams.m_proj = &m_camera.GetProj();
			gxMultiPostprocess_Update(&ppParams);
		}

		// User update

		ueDefaultApp_Update(dt);

		// User rendering

		glCtx* ctx = ueDefaultApp_BeginDrawing();
		if (!ctx)
			return;

		// ...

		ueDefaultApp_EndDrawing();
	}
}

void ueDefaultApp_DeinitBase()
{
	ueLogI("Shutting down...");

	gxMultiPostprocess_Shutdown();

	auLib_Shutdown();

#if defined(IN_ENABLE_ACCELEROMETER)
	inAccelerometer_Shutdown();
#endif

#if !defined(UE_MARMALADE)
	gxShapeDraw_Shutdown();
#endif

	gxShapeDraw_Shutdown();
	gxTextRenderer_Shutdown();
	gxFontMgr_Shutdown();
	gxEffectTypeMgr_Shutdown();
	gxTextureMgr_Shutdown();
#if !defined(GL_FIXED_PIPELINE)
	gxShaderMgr_Shutdown();
#endif
	ueResourceMgr_Shutdown();

	inSys_Shutdown();
	inKeyboard_Shutdown();
#if defined(IN_ENABLE_MOUSE)
	inMouse_Shutdown();
#endif
#if defined(IN_ENABLE_GAMEPAD)
	inGamepad_Shutdown();
#endif
#if defined(IN_ENABLE_TOUCHPAD)
	inTouchpad_Shutdown();
#endif

	glDevice_Shutdown();

#if defined(UE_HAS_WINDOWS)
	ueWindow_Destroy(g_app.m_window);
#endif

	ueLocaleMgr_Shutdown();
	ueAppVars::Shutdown();
	ueEnv_Shutdown();

#ifndef UE_FINAL
	g_app.m_logFile.Deinit();
#endif

	ioFileSys_Shutdown();

	utCompression_Shutdown();

#if defined(UE_ENABLE_PROFILER)
	ueProfiler_Shutdown();
#endif

	thGlobalJobSys_Shutdown();

	ueRandMgr_Shutdown();

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	ueMemoryDebugger_DumpUnfreedAllocations(g_app.m_memoryDebugger);
	UE_ASSERT(ueMemoryDebugger_GetNumUnfreedAllocations(g_app.m_memoryDebugger) == 0);

	ueMemoryDebugger_RemoveAllocatorToTrack(g_app.m_memoryDebugger, &g_app.m_stackAllocator);
	ueMemoryDebugger_RemoveAllocatorToTrack(g_app.m_memoryDebugger, &g_app.m_freqAllocator);
	ueMemoryDebugger_End(g_app.m_memoryDebugger);
	ueMemoryDebugger_Destroy(g_app.m_memoryDebugger);
#endif

#ifndef UE_MARMALADE
	g_app.m_freqAllocator.Deinit();
#endif

}

glCtx* ueDefaultApp_BeginDrawing()
{
	UE_PROF_SCOPE("Begin Drawing");

	// Begin rendering to context

	glCtx* ctx = glDevice_GetDefaultContext();
	if (!glCtx_Begin(ctx))
		return NULL;

	// Set up clear values

	glCtx_SetClearColor(ctx, 0, 0.0f, 0, 0);
	glCtx_SetClearDepthStencil(ctx, 1.0f, 0);

	// Begin drawing to main render group

	glRenderGroup_Begin(glDevice_GetMainRenderGroup(), ctx);

	return ctx;
}

void ueDefaultApp_EndDrawing()
{
	UE_PROF_SCOPE("End Drawing");

	glCtx* ctx = glDevice_GetDefaultContext();

	// End drawing to main render group

	glRenderGroup_EndDrawing(glDevice_GetMainRenderGroup());
	glTextureBuffer* sceneColor = glRenderGroup_AcquireColorOutput(glDevice_GetMainRenderGroup(), 0);
	glRenderGroup_End(glDevice_GetMainRenderGroup());

	// End drawing to context

	glCtx_End(ctx);

	// Present result of drawing to screen

	glCtx_Present(ctx, sceneColor);

	// Release buffer

	glRenderBufferPool_ReleaseTextureBuffer(sceneColor);
}

// Implementation
// --------------

s32 ueDefaultApp_Init()
{
	ueDefaultAppInitParams params;
	params.m_appName = "Parallel Realities - (C) 2013 Pixel Elephant";

	if (!ueDefaultApp_InitBase(&params))
		return 1;

	return 0;
}

void ueDefaultApp_Deinit()
{
	ueDefaultApp_DeinitBase();
}

void ueDefaultApp_Update(f32 dt)
{
}

void ueDefaultApp_Draw(glCtx* ctx)
{
}

// Main

#if defined(UE_WIN32) && defined(UE_FINAL)

	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	{
		// Convert command line into arguments

		char argBuffer[512];
		u32 argc;
		char* argv[64];
		ueProcess_CommandLineToArgs(lpCmdLine, argBuffer, UE_ARRAY_SIZE(argBuffer), argv, UE_ARRAY_SIZE(argv), &argc);

		// Main app function

		return ueDefaultApp_Start(argc, argv);
	}

#else

	int main(u32 argc, char** argv)
	{
		return ueDefaultApp_Start(argc - 1, (const char**) (argv + 1));
	}

#endif
#include "Base/ueProfilerCapture.h"
#include "Base/Allocators/ueMemoryDebugger.h"
#include "Base/ueJobSys.h"
#include "Base/ueWindow.h"
#include "Base/ueRand.h"
#include "Input/inSys.h"
#include "Utils/utProcess.h"
#include "Utils/utCompression.h"
#include "Utils/utWarningSys.h"
#include "Utils/utAssetCompilerHelper.h"
#include "IO/ioFile.h"
#include "Audio/auLib.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShader.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxAnimation.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxDebugModelRenderer.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "GraphicsExt/gxDebugWindow.h"
#include "GraphicsExt/gxTextureDebugger.h"
#include "GraphicsExt/gxConsole.h"
#include "GraphicsExt/gxProfiler.h"
#include "SampleVars.h"
#include "SampleApp.h"
#include "Sample.h"

#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC)
	extern void ueRegisterCompressionLib_ZLIB();
#endif

#if !defined(UE_FINAL)
	#define UE_SUPPORT_BUILD_ASSETS
#endif

ueSampleApp g_app;

ueSampleApp::ueSampleApp() :
	m_quit(UE_FALSE),
	m_sampleCreateFunc(NULL),
	m_sample(NULL)
{}

void ueSampleApp::Run(u32 argc, const char** argv)
{
	// Startup

	ueSetFatalErrorHandler(FatalErrorHandlerFunc);
	ueCallstackHelper_Startup();
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	m_sysAllocator.SetName("system");
#endif

	const ueSize freqHeapSize = (1 << 20) * 128;
	void* freqHeapMemory = m_sysAllocator.Alloc(freqHeapSize, 4);
	UE_ASSERT(freqHeapMemory);
	m_freqAllocator.InitMem(freqHeapMemory, freqHeapSize);
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	m_freqAllocator.SetName("frequent-global");
#endif

	const ueSize stackHeapSize = (1 << 20) * 16;
	void* stackHeapMemory = m_sysAllocator.Alloc(stackHeapSize, 4);
	UE_ASSERT(stackHeapMemory);
	u32 stackHeapMaxAllocs = 0;
	ueSize stackHeapDebugMemorySize = 0;
	void* stackHeapDebugMemory = NULL;
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	stackHeapMaxAllocs = 1 << 12;
	stackHeapDebugMemorySize = ueStackAllocator::CalcDebugMemReq(stackHeapMaxAllocs);
	stackHeapDebugMemory = m_sysAllocator.Alloc(stackHeapDebugMemorySize, 4);
#endif
	m_stackAllocator.InitMem(stackHeapMemory, stackHeapSize, stackHeapMaxAllocs, stackHeapDebugMemory, stackHeapDebugMemorySize);
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	m_stackAllocator.SetName("stack-global");
#endif

	ueRandMgr_Startup(&m_stackAllocator);
	ueClock_Startup();
	ueLogger_Startup();

	ueAssets_Startup(argc, argv);
	ueAssets_SetLanguageSymbol("EN");

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	m_memoryDebugger = ueMemoryDebugger_Create(&m_stackAllocator, 65536, 16);
	ueMemoryDebugger_Begin(m_memoryDebugger);
	ueMemoryDebugger_AddAllocatorToTrack(m_memoryDebugger, &m_freqAllocator);
	ueMemoryDebugger_AddAllocatorToTrack(m_memoryDebugger, &m_stackAllocator);
#endif

	ueJobSysThreadDesc jobsysThreadDesc;
	jobsysThreadDesc.m_name = "global-job-sys";
	ueJobSysParams jobsysParams;
	jobsysParams.m_stackAllocator = &m_stackAllocator;
	jobsysParams.m_numThreads = 1;
	jobsysParams.m_threadDescs = &jobsysThreadDesc;
	thGlobalJobSys_Startup(&jobsysParams);

#if defined(UE_ENABLE_WARNINGS)
	utWarningSys_Startup(&m_stackAllocator);
#endif

#if defined(UE_ENABLE_PROFILER)
	ueProfiler_Startup(&m_stackAllocator);
#endif
	m_profCapture = NULL;

	utCompression_Startup(&m_stackAllocator);
#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC)
	ueRegisterCompressionLib_ZLIB();
#endif

	ioFileSys_Startup(&m_stackAllocator);

	ioNativeFileSysParams fsParams;
	fsParams.m_allocator = &m_stackAllocator;
	fsParams.m_maxFilesOpen = 16;
	fsParams.m_root = "../../Data/Exp/";
	ioFileSys* fs = ioFileSys_CreateNative(&fsParams);
	UE_ASSERT(fs);
	ioFileSys_Mount(fs, "", 0);
	ioFileSys_SetDefault(fs);

#if !defined(UE_FINAL)
	fsParams.m_maxFilesOpen = 4;
	fsParams.m_root = "../../Data/Src/";
	ioFileSys* sourceFS = ioFileSys_CreateNative(&fsParams);
	UE_ASSERT(sourceFS);
	ioFileSys_Mount(sourceFS, "", 1);
	ioFileSys_SetDefault(sourceFS); // Override default file system (non final builds only!)
#endif // !defined(UE_FINAL)

	ueLogFile logFile;
	logFile.Init("UberSample.log");
	ueLogSystemInfo();

	ueEnv_Startup(&m_stackAllocator, 512, 512);
	ueSampleVars::Startup();

	gxDebugMenu_StartupEnv(&m_freqAllocator);
	gxProfiler_StartupEnv(&m_freqAllocator); // FIXME: Use stack allocator here

	ueLocaleMgr_Startup(&m_freqAllocator);

	uePath configFilePath;
	ueAssets_GetAssetPath(configFilePath, "uber_sample_config", "txt");
	UE_ASSERT_FUNC( ueEnv_DoFile(configFilePath) );

#if defined(UE_SUPPORT_BUILD_ASSETS)
	if (!ueApp_GetArgByName("no_build_assets"))
	{
		s32 errorCode;
		utAssetCompilerHelperConfig config;
		config.m_shaderModel = "30";
		config.m_targetsFile = "../../Data/Src/uber_sample_targets.xml";
		config.m_assetsFile = "../../Data/Src/uber_sample_assets.xml";
		if (!utAssetCompilerHelper_Run(&config, &errorCode))
		{
			logFile.Deinit();
			ueExit(1);
		}
	}
#endif // defined(UE_SUPPORT_BUILD_ASSETS)

	u32 initWidth, initHeight;
	ueSampleVars::GetResolution(initWidth, initHeight);

#if defined(UE_HAS_WINDOWS)
	ueWindowParams windowParams;
	windowParams.m_allocator = &m_stackAllocator;
	windowParams.m_rect = ueRectI(50, 50, initWidth, initHeight);
	windowParams.m_minWidth = 64;
	windowParams.m_minHeight = 64;
	windowParams.m_maxWidth = 4096;
	windowParams.m_maxHeight = 4096;
	windowParams.m_fullscreen = ueVar_GetBool(ueSampleVars::m_fullscreen);
	ueStrCpyS(windowParams.m_name, "UberSample");
#if defined(UE_WIN32)
	ueStrCpyS(windowParams.m_className, "UberSample");
	windowParams.m_hInstance = NULL;
#endif
	windowParams.m_hideCursorWhenActive = UE_TRUE;
	windowParams.m_resizeable = UE_TRUE;
	windowParams.m_maximizable = UE_TRUE;
	windowParams.m_minimizable = UE_TRUE;

	m_window = ueWindow_Create(&windowParams);
	UE_ASSERT(m_window);
	ueWindow_SetMain(m_window);
#endif // defined(UE_HAS_WINDOWS)

	glDevice_PreStartup();

	glDeviceStartupParams glParams;
	glParams.m_stackAllocator = &m_stackAllocator;
	glParams.m_freqAllocator = &m_freqAllocator;
#if defined(UE_HAS_WINDOWS)
	glParams.m_width = initWidth;
	glParams.m_height = initHeight;
	glParams.m_fullscreen = ueVar_GetBool(ueSampleVars::m_fullscreen);
#else
	glParams.m_width = ueVar_GetS32(ueSampleVars::m_width);
	glParams.m_height = ueVar_GetS32(ueSampleVars::m_height);
#endif // defined(UE_HAS_WINDOWS)
	glParams.m_colorFormat = glBufferFormat_Native_R8G8B8A8;
	glParams.m_depthStencilFormat = glBufferFormat_D24S8;
#if defined(UE_FINAL)
	glParams.m_displaySync = glDisplaySync_Vertical;
#else
	glParams.m_displaySync = glDisplaySync_None;
#endif
	glParams.m_backBufferCount = 2;
	glParams.m_MSAALevel = ueVar_GetS32(ueSampleVars::m_msaaLevel);
	glParams.m_window = m_window;
	glParams.m_adapterIndex = 0;
#if defined(GL_D3D9)
	glParams.m_frequency = 60;
	glParams.m_hardwareVertexProcessing = UE_TRUE;
	glParams.m_multithreaded = UE_FALSE;
	glParams.m_deviceType = D3DDEVTYPE_HAL;
#elif defined(GL_D3D11)
	glParams.m_frequency = 60;
#endif
	if (glParams.m_fullscreen)
		glDevice_GetClosestSupportedMode(&glParams);
	glDevice_Startup(&glParams);

	gxCommonConstants::Startup();

	m_timer.Init();

	gxDebugWindowSys_Startup(&m_freqAllocator);
#if !(defined(UE_FINAL) || defined(UE_RELEASE))
	gxDebugWindowSys_SetActive(UE_TRUE);
#endif

	inKeyboard_Startup();
	inMouse_Startup();
	inGamepad_Startup();

	inSysStartupParams insysParams;
	insysParams.m_stackAllocator = &m_stackAllocator;
	inSys_Startup(&insysParams);

#if !defined(UE_FINAL)
	const inConsumerId sampleAppInput = inSys_RegisterConsumer("Sample App");
	const inEventId quitInputEvent = inSys_RegisterEvent(sampleAppInput, "quit app", &inBinding(inDev_Keyboard, inKey_Escape), &inBinding(inDev_Gamepad, inGamepadAnalog_Shoulder_L2, inGamepadAnalog_Shoulder_R2, inGamepadButton_Back));
	const inEventId toggleDebugMenuInputEvent = inSys_RegisterEvent(sampleAppInput, "toggle debug menu", &inBinding(inDev_Keyboard, inKey_F1), &inBinding(inDev_Gamepad, inGamepadButton_Back));
	const inEventId toggleProfilerInputEvent = inSys_RegisterEvent(sampleAppInput, "toggle profiler", &inBinding(inDev_Keyboard, inKey_F2));
	const inEventId toggleProfilerInputInputEvent = inSys_RegisterEvent(sampleAppInput, "toggle profiler input", &inBinding(inDev_Keyboard, inKey_F3));
	const inEventId restartSampleInputEvent = inSys_RegisterEvent(sampleAppInput, "restart sample", &inBinding(inDev_Keyboard, inKey_F6), &inBinding(inDev_Gamepad, inGamepadButton_Start));
	const inEventId nextSampleInputEvent = inSys_RegisterEvent(sampleAppInput, "next sample", &inBinding(inDev_Keyboard, inKey_F7), &inBinding(inDev_Gamepad, inGamepadButton_Shoulder_L1));
	const inEventId prevSampleInputEvent = inSys_RegisterEvent(sampleAppInput, "prev sample", &inBinding(inDev_Keyboard, inKey_F8), &inBinding(inDev_Gamepad, inGamepadButton_Shoulder_R1));
#endif
	inSys_SetConsumerCallback(inActiveConsumerChangedCallback);
	m_inputConsumerName[0] = 0;
	m_inputConsumerChangeTime = U32_MAX;

	ueResourceMgr_Startup(&m_stackAllocator);

	const char* animationConfig = NULL;
	const char* shaderConfig = NULL;
	const char* modelConfig = NULL;
	const char* textureConfig = NULL;
	const char* fontConfig = NULL;

#if defined(UE_WIN32)
	animationConfig = "win32";
	#if defined(GL_D3D9)
		shaderConfig = "win32-d3d9_sm30";
		#if defined(PH_BULLET)
			modelConfig = "win32-bullet-d3d9";
		#elif defined(PH_PHYSX)
			modelConfig = "win32-physx-d3d9";
		#else
			#error "Unsupported physics engine"
		#endif
		textureConfig = "win32-d3d9";
		fontConfig = "win32-d3d9";
	#elif defined(GL_D3D11)
		shaderConfig = "win32-d3d11_sm40";
		#if defined(PH_BULLET)
			modelConfig = "win32-bullet-d3d11";
		#elif defined(PH_PHYSX)
			modelConfig = "win32-physx-d3d11";
		#else
			#error "Unsupported physics engine"
		#endif
		textureConfig = "win32-d3d11";
		fontConfig = "win32-d3d11";
	#elif defined(GL_OPENGL_ES)
		shaderConfig = "win32-gles";
		#if defined(PH_BULLET)
			modelConfig = "win32-bullet-gl";
		#elif defined(PH_PHYSX)
			modelConfig = "win32-physx-gl";
		#else
			#error "Unsupported physics engine"
		#endif
		textureConfig = "win32-gles";
		fontConfig = "win32-gles";
	#elif defined(GL_OPENGL)
		shaderConfig = "win32-glsl";
		#if defined(PH_BULLET)
			modelConfig = "win32-bullet-gl";
		#elif defined(PH_PHYSX)
			modelConfig = "win32-physx-gl";
		#else
			#error "Unsupported physics engine"
		#endif
		textureConfig = "win32-gl";
		fontConfig = "win32-gl";
	#endif
#elif defined(UE_LINUX)
	animationConfig = "linux";
	shaderConfig = "linux-glsl";
	#if defined(PH_BULLET)
		modelConfig = "linux-bullet-gl";
	#else
		#error "Unsupported physics engine"
	#endif
	textureConfig = "linux-gl";
	fontConfig = "linux-gl";
#elif defined(UE_X360)
	animationConfig = "x360";
	shaderConfig = "x360";
	#if defined(PH_BULLET)
		modelConfig = "x360-bullet";
	#elif defined(PH_PHYSX)
		modelConfig = "x360-physx";
	#else
		#error "Unsupported physics engine"
	#endif
	textureConfig = "x360";
	fontConfig = "x360";
#endif
	gxTextureMgr_Startup(&m_freqAllocator, textureConfig);
	gxShaderMgr_Startup(&m_freqAllocator, shaderConfig);
	gxModelMgr_Startup(&m_freqAllocator, modelConfig);
	gxAnimationMgr_Startup(&m_freqAllocator, animationConfig);
	gxFontMgr_Startup(&m_freqAllocator, fontConfig);

	gxTextureDebugger_Startup(&m_freqAllocator);

	gxShapeDrawStartupParams shapeDrawParams;
	shapeDrawParams.m_stackAllocator = &m_stackAllocator;
	gxShapeDraw_Startup(&shapeDrawParams);

	gxDebugWindowSys_StartupResources();

	gxTextRendererStartupParams textRendererParams;
	textRendererParams.m_stackAllocator = &m_stackAllocator;
	gxTextRenderer_Startup(&textRendererParams);

	gxDebugMenuStartupParams debugMenuParams;
	gxDebugMenu_Startup(&debugMenuParams);

	gxDebugMenu_CreateMenu(NULL, "Statistics");
	ueSampleVars::AddToDebugMenu(configFilePath);
	gxDebugMenuItem* demoMenu = gxDebugMenu_CreateMenu(NULL, "Demo");
	{
		gxDebugMenu_CreateUserAction(demoMenu, "next sample", gxDebugMenuCallback_NextSample);
		gxDebugMenu_CreateUserAction(demoMenu, "previous sample", gxDebugMenuCallback_PrevSample);
		gxDebugMenu_CreateUserAction(demoMenu, "restart sample", gxDebugMenuCallback_RestartSample);
		gxDebugMenu_CreateUserAction(demoMenu, "quit", gxDebugMenuCallback_Quit);
	}

	gxProfilerStartupParams profilerParams;
	gxProfiler_Startup(&profilerParams);
	gxProfiler_EnableInput(UE_FALSE);
	gxProfiler_AddAllocator(&m_freqAllocator);
	gxProfiler_AddAllocator(&m_stackAllocator);

	gxConsoleStartupParams consoleParams;
	consoleParams.m_stackAllocator = &m_stackAllocator;
	gxConsole_Startup(&consoleParams);

	gxDebugModelRendererStartupParams debugModelRendererParams;
	gxDebugModelRenderer_Startup(&debugModelRendererParams);

	gxDebugWindowSys_LoadLayout();

	auLibStartupParams auLibParams;
	auLibParams.m_freqAllocator = &m_freqAllocator;
	auLibParams.m_stackAllocator = &m_stackAllocator;
	auLibParams.m_settingsFileName = "test_sound_config";
	auLib_Startup(&auLibParams);

	// Initialize first sample

	SetSample( ueSampleMgr_GetByName("Model") );

	// Main loop

	while (!m_quit)
	{
		{
			// Finish profiling last frame

#if defined(UE_ENABLE_PROFILER)
			ueProfiler_NextFrame();
#endif

			UE_PROF_SCOPE("Main Loop");

#if defined(UE_ENABLE_PROFILER)
			if (gxProfiler_IsWaitingForProfilerCapture())
			{
				UE_PROF_SCOPE("Build Profiler Capture");
				const ueProfilerCapture* profCapture = ueProfiler_BuildCapture();
				if (profCapture)
					gxProfiler_SetProfilerCapture(profCapture);
			}
#endif // defined(UE_ENABLE_PROFILER)

			m_timer.Tick();
			const f32 dt = m_timer.DT();

			ueBool updateInputDevices = UE_TRUE;
#if defined(UE_HAS_WINDOWS)
			ueWindowUtils_HandleOSMessages();
			updateInputDevices = ueWindow_HasFocus(m_window);
#endif // defined(UE_HAS_WINDOWS)

			ueResourceMgr_DoWork(1.0f / 120.0f);

			inSys_Update(updateInputDevices);
			auLib_Update(dt);

			gxProfiler_Update(dt);
			gxConsole_Update(dt);
			gxDebugMenu_Update(dt);
			gxDebugWindowSys_Update(dt);
			gxTextureDebugger_Update(dt);

#if defined(UE_ENABLE_WARNINGS)
			utWarningSys_Reset();
#endif

#if !defined(UE_FINAL)
			if (inSys_WasPressed(quitInputEvent))
			{
				m_quit = UE_TRUE;
				break;
			}
			else if (inSys_WasPressed(toggleDebugMenuInputEvent))
				gxDebugMenu_Enable(!gxDebugMenu_IsEnabled());
			else if (inSys_WasPressed(toggleProfilerInputEvent))
				gxProfiler_Enable(!gxProfiler_IsEnabled());
			else if (inSys_WasPressed(toggleProfilerInputInputEvent))
				gxProfiler_EnableInput(!gxProfiler_IsInputEnabled());
			else if (inSys_WasPressed(restartSampleInputEvent))
				RestartSample();
			else if (inSys_WasPressed(prevSampleInputEvent))
				PrevSample();
			else if (inSys_WasPressed(nextSampleInputEvent))
				NextSample();
#endif // !defined(UE_FINAL)

			if (m_quit)
				break;

			// Do sample specific frame
			{
				UE_PROF_SCOPE("DoFrame");
				m_sample->DoFrame(dt);
			}
		}
	}

	// Shut down

	SetSample(NULL);

	auLib_Shutdown();

	ueLocaleMgr_Shutdown();

	gxDebugModelRenderer_Shutdown();
	gxConsole_Shutdown();
	gxProfiler_Shutdown();
	gxDebugMenu_Shutdown();
	gxTextRenderer_Shutdown();
	gxShapeDraw_Shutdown();
	gxDebugWindowSys_ShutdownResources();
	gxTextureDebugger_Shutdown();

	gxFontMgr_Shutdown();
	gxAnimationMgr_Shutdown();
	gxModelMgr_Shutdown();
	gxTextureMgr_Shutdown();
	gxShaderMgr_Shutdown();
	ueResourceMgr_Shutdown();

	inSys_Shutdown();
	inKeyboard_Shutdown();
	inMouse_Shutdown();
	inGamepad_Shutdown();

	gxDebugWindowSys_Shutdown();

	glDevice_Shutdown();

	ueWindow_Destroy(m_window);

	ueSampleVars::Shutdown();
	ueEnv_Shutdown();

	logFile.Deinit();

#if !defined(UE_FINAL)
	ioFileSys_Unmount(sourceFS);
	ioFileSys_Destroy(sourceFS);
#endif
	ioFileSys_Unmount(fs);
	ioFileSys_Destroy(fs);
	ioFileSys_Shutdown();

	utCompression_Shutdown();

#if defined(UE_ENABLE_PROFILER)
	ueProfiler_Shutdown();
#endif

#if defined(UE_ENABLE_WARNINGS)
	utWarningSys_Shutdown();
#endif
	thGlobalJobSys_Shutdown();

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	ueMemoryDebugger_DumpUnfreedAllocations(m_memoryDebugger);
	UE_ASSERT(ueMemoryDebugger_GetNumUnfreedAllocations(m_memoryDebugger) == 0);

	ueMemoryDebugger_RemoveAllocatorToTrack(m_memoryDebugger, &m_stackAllocator);
	ueMemoryDebugger_RemoveAllocatorToTrack(m_memoryDebugger, &m_freqAllocator);
	ueMemoryDebugger_End(m_memoryDebugger);
	ueMemoryDebugger_Destroy(m_memoryDebugger);
#endif

	m_freqAllocator.Deinit();

	ueAssets_Shutdown();

	ueLogger_Shutdown();
	ueClock_Shutdown();
	ueRandMgr_Shutdown();
}

glCtx* ueSampleApp::BeginDrawing()
{
	UE_PROF_SCOPE("Begin Drawing");

	glCtx* ctx = glDevice_GetDefaultContext();
	if (!glCtx_Begin(ctx))
		return NULL;

	glCtx_SetClearColor(ctx, 0, 0.0f, 0, 0);
	glCtx_SetClearDepthStencil(ctx, 1.0f, 0);

	glRenderGroup_Begin(glDevice_GetMainRenderGroup(), ctx);

	return ctx;
}

void ueSampleApp::EndDrawing()
{
	UE_PROF_SCOPE("End Drawing");

	glCtx* ctx = glDevice_GetDefaultContext();

	glRenderGroup_EndDrawing(glDevice_GetMainRenderGroup());
	glTextureBuffer* sceneColor = glRenderGroup_AcquireColorOutput(glDevice_GetMainRenderGroup(), 0);
	glRenderGroup_End(glDevice_GetMainRenderGroup());

	glCtx_End(ctx);

	glCtx_Present(ctx, sceneColor);
	glRenderBufferPool_ReleaseTextureBuffer(sceneColor);
}

void ueSampleApp::DrawAppOverlay()
{
	UE_PROF_SCOPE("Draw App Overlay");

#if !defined(UE_FINAL)

	glCtx* ctx = glDevice_GetDefaultContext();

	// Draw debug windows

	gxTextureDebugger_Draw();
	gxDebugWindowSys_Draw(ctx);

	// Draw fading out "Input switched to ..." text

	const f32 inputConsumerChangeFadeOutTime = 3.0f;
	const f32 inputConsumerChangeTimePassed = ueClock_GetSecsSince(m_inputConsumerChangeTime);
	if (inputConsumerChangeTimePassed < inputConsumerChangeFadeOutTime)
	{
		char buffer[128];
		ueStrFormatS(buffer, "Input switched to: %s", m_inputConsumerName);

		gxText text;
		text.m_x = 100.0f;
		text.m_y = 500.0f;
		text.m_color = ueColor32::Red;
		text.m_color.SetAlphaF((inputConsumerChangeFadeOutTime - inputConsumerChangeTimePassed) / inputConsumerChangeFadeOutTime);
		text.m_utf8Buffer = buffer;

		gxTextRenderer_Draw(ctx, &text);
	}

#endif // !defined(UE_FINAL)
}

void ueSampleApp::NextSample()
{
	SetSample(m_sampleCreateFunc ? ueSampleMgr_GetNext(m_sampleCreateFunc) : ueSampleMgr_GetFirst());
}

void ueSampleApp::PrevSample()
{
	if (!m_sampleCreateFunc)
		SetSample(ueSampleMgr_GetFirst());
	else
		SetSample(ueSampleMgr_GetPrev(m_sampleCreateFunc));
}

void ueSampleApp::RestartSample()
{
	ueSampleCreateFunc curr = m_sampleCreateFunc;
	SetSample(NULL);
	SetSample(curr);
}

void ueSampleApp::gxDebugMenuCallback_NextSample(const char* actionName, void* userData)
{
	g_app.NextSample();
}

void ueSampleApp::gxDebugMenuCallback_PrevSample(const char* actionName, void* userData)
{
	g_app.PrevSample();
}

void ueSampleApp::gxDebugMenuCallback_RestartSample(const char* actionName, void* userData)
{
	g_app.RestartSample();
}

void ueSampleApp::gxDebugMenuCallback_Quit(const char* actionName, void* userData)
{
	g_app.m_quit = UE_TRUE;
}

void ueSampleApp::SetSample(ueSampleCreateFunc sampleCreateFunc)
{
	if (sampleCreateFunc == m_sampleCreateFunc)
		return;

	// Destroy old sample

	if (m_sample)
	{
		m_sample->Deinit();
		ueDelete(m_sample, &m_freqAllocator);
	}

	// Create new sample

	m_sampleCreateFunc = sampleCreateFunc;
	if (!m_sampleCreateFunc)
	{
		m_sample = NULL;
		ueWindow_SetName(m_window, "UberSample");
		return;
	}
	m_sample = m_sampleCreateFunc(&m_freqAllocator);
	UE_ASSERT(m_sample);

	// Initialize new sample

	if (m_sample->Init())
	{
#if defined(GL_OPENGL_ES)
		const char* graphicsAPIName = "OpenGL ES";
#elif defined(GL_OPENGL)
		const char* graphicsAPIName = "OpenGL";
#elif defined(GL_D3D9)
		const char* graphicsAPIName = "Direct3D 9";
#elif defined(GL_D3D11)
		const char* graphicsAPIName = "Direct3D 11";
#endif

		char windowName[256];
		ueStrFormatS(windowName, "UberSample (%s) - %s", graphicsAPIName, ueSampleMgr_GetName(m_sampleCreateFunc));
		ueWindow_SetName(m_window, windowName);
	}
	else
	{
		ueLogW("Sample '%s' failed to initialize - setting next available sample...", ueSampleMgr_GetName(m_sampleCreateFunc));

		ueDelete(m_sample, &m_freqAllocator);
		m_sample = NULL;
		m_sampleCreateFunc = NULL;

		// Try to set next sample - this causes recursive chain of calls

		SetSample(ueSampleMgr_GetNext(sampleCreateFunc));
	}
}

void ueSampleApp::inActiveConsumerChangedCallback(const char* consumerName)
{
	ueStrCpyS(g_app.m_inputConsumerName, consumerName);
	g_app.m_inputConsumerChangeTime = ueClock_GetCurrent();
}

void ueSampleApp::FatalErrorHandlerFunc(const char* file, u32 line, u32 errorCode, const char* msg)
{
	char msgBuffer[1 << 12];
	ueStrFormatS(msgBuffer, "Fatal error occured.\n\nDescription:\n%s:%d (code: %u)\n%s\n\nFor more details check log file.\nApplication will now be closed.", file, line, errorCode, msg);

	ueWindow* window = g_app.GetWindow();
	if (window)
		ueWindow_Minimize(window);
	ueWindowUtils_ShowMessageBox(NULL, msgBuffer, "Application Error", ueWindowMsgBoxType_Error);
	ueExit(2);
}

#if defined(UE_WIN32) && defined(UE_FINAL)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Convert command line into arguments

	char argBuffer[512];
	u32 argc;
	char* argv[64];
	utProcess_CommandLineToArgs(lpCmdLine, argBuffer, UE_ARRAY_SIZE(argBuffer), argv, UE_ARRAY_SIZE(argv), &argc);

	// Run app

	g_app.Run(argc, (const char**) argv);
}

#else

void main(int argc, char** argv)
{
	g_app.Run((u32) argc - 1, (const char**) (argv + 1));
}

#endif
#ifndef UE_DEFAULT_APP
#define UE_DEFAULT_APP

struct ueDefaultAppInitParams
{
	u32 m_frequentAllocatorSize;
	u32 m_stackAllocatorSize;
	const char* m_afsDataFile;
	ueBool m_buildAssets;
	const char* m_appName;
	const char* m_languageSymbol;

	ueDefaultAppInitParams() :
		m_frequentAllocatorSize((1 << 20) * 7),
		m_stackAllocatorSize((1 << 20) * 4),
		m_afsDataFile("game.afs"),
#if defined(UE_FINAL)
		m_buildAssets(UE_FALSE),
		m_isDevelopmentMode(UE_FALSE)
#else
		m_buildAssets(UE_TRUE)
		m_isDevelopmentMode(UE_TRUE)
#endif
		m_appName("Uber Engine App"),
		m_languageSymbol("EN")
	{}
};

// To be defined by the user

s32					ueDefaultApp_Init();
s32					ueDefaultApp_Deinit();
void				ueDefaultApp_Update(f32 dt);
void				ueDefaultApp_Draw(glCtx* ctx);

// Default application

void				ueDefaultApp_InitBase(ueDefaultAppInitParams* params);
void				ueDefaultApp_DeinitBase();
void				ueDefaultApp_MainLoop();
glCtx*				ueDefaultApp_BeginDrawing();
void				ueDefaultApp_EndDrawing();
void				ueDefaultApp_Quit();
const ueAppTimer&	ueDefaultApp_GetGameTimer();
ueMemoryDebugger*	ueDefaultApp_GetMemoryDebugger();
ueAllocator*		ueDefaultApp_GetSystemAllocator();
ueAllocator*		ueDefaultApp_GetStackAllocator();
ueAllocator*		ueDefaultApp_GetFrequentAllocator();

#if defined(UE_HAS_WINDOWS)
ueWindow*			ueDefaultApp_GetWindow();
#endif

#endif // UE_DEFAULT_APP
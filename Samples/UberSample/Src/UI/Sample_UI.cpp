#include "SampleApp.h"
#include "UI/uiLib.h"
#include "Input/inSys.h"

/**
 *	Demonstrates UI library usage.
 */
class ueSample_UI : public ueSample
{
public:
	ueBool Init()
	{
		// Init UI library

		uiLibStartupParams params;
		params.m_allocator = g_app.GetFreqAllocator();
		params.m_rootAssetPath = "ui_sample/";
		uiLib_Startup(&params);
		uiLib_SetEventListener(UIEventListener, this);

		// Load cursor

		UE_ASSERT_FUNC(uiLib_LoadCursor("cursor"));

		// Load fonts
			
		UE_ASSERT_FUNC(uiLib_LoadFont("Delicious-Roman.otf"));
		UE_ASSERT_FUNC(uiLib_LoadFont("Delicious-Italic.otf"));
		UE_ASSERT_FUNC(uiLib_LoadFont("Delicious-Bold.otf"));
		UE_ASSERT_FUNC(uiLib_LoadFont("Delicious-BoldItalic.otf"));

		// Create main menu UI scene

		m_mainMenu = uiScene_Create("main_menu");
		UE_ASSERT(m_mainMenu);
		uiScene_SetVisible(m_mainMenu, UE_TRUE);

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("UI sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
		uiScene_Destroy(m_mainMenu);
		uiLib_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		// Update

		if (inSys_IsConsumerActive(m_inputConsumerId))
			uiLib_Update(dt);

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		
		uiLib_Draw(ctx);

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	static void UIEventListener(uiEventType eventType, const char* value, void* userData)
	{
		if (!ueStrCmp(value, "toggle_debugger"))
			uiLib_ShowDebugger( !uiLib_IsDebuggerVisible() );
		else if (!ueStrCmp(value, "exit"))
		{
		}
	}

	inConsumerId m_inputConsumerId;

	uiScene* m_mainMenu;
};

UE_DECLARE_SAMPLE(ueSample_UI, "UI")

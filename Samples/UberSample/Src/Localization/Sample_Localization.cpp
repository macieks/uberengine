#include "SampleApp.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxFont.h"

/**
 *	Demonstrates localization support for texts, fonts and texture.
 */
class ueSample_Localization : public ueSample
{
public:
	ueBool Init()
	{
		ueLocaleMgr_SetRootDir("localization_sample");

		// Load text localization set

		UE_ASSERT_FUNC( ueLocaleMgr_LoadSet("TestTexts") );

		// TODO: Load (localized) font
	#if 0
		m_font.SetByName("localization/font");
	#endif

		return UE_TRUE;
	}

	void Deinit()
	{
		ueLocaleMgr_UnloadSet("TestTexts");
	}

	void DoFrame(f32 dt)
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		// Localize text

		const ueLocaleParam locParams[] =
		{
			ueLocaleParam("name", "James"),
			ueLocaleParam("surname", "Bond")
		};
		const char* localizedText = UE_LOC_P("TestTexts.Misc.Intro", locParams);

		// Draw text

		gxText text;
		text.m_x = 150.0f;
		text.m_y = 100.0f;
		text.m_utf8Buffer = localizedText;
		text.m_font = *m_font;
		gxTextRenderer_Draw(ctx, &text);

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	ueResourceHandle<gxFont> m_font;
};

UE_DECLARE_SAMPLE(ueSample_Localization, "Localization")
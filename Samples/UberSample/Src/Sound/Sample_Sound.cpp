#include "SampleApp.h"
#include "Audio/auLib.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates playing 2D and 3D sound.
 */
class ueSample_Sound : public ueSample
{
public:
	ueBool Init()
	{
		// Check if we have valid audio device

		if (!auLib_IsValid())
			return UE_FALSE;

		// Load sound bank

		m_soundBank = auSoundBank_Load("test_bank");
		UE_ASSERT(m_soundBank);

		// Create background music

		auSoundParams params;
		params.m_handlePtr = &m_music;
		params.m_id = auSoundBank_GetSoundId("test_music", m_soundBank);
		UE_ASSERT(params.m_id);

		auSound_Create(&params);
		UE_ASSERT(*m_music);

		// Start (looping) 3D sound

		params.m_position = &ueVec3::Zero;
		params.m_handlePtr = &m_sound3D;
		params.m_id = auSoundBank_GetSoundId("test_looping_sound", m_soundBank);
		UE_ASSERT(params.m_id);

		auSound_Create(&params);
		UE_ASSERT(*m_sound3D);

		m_sound3DRotation = 0.0f;

		// Place listener at (0, 0, 0)

		auListenerDesc listener;
		auLib_SetListenerDesc(listener);

		return UE_TRUE;
	}

	void Deinit()
	{
		m_sound3D.Destroy();
		m_music.Destroy();
		auSoundBank_Unload(m_soundBank);
	}

	void DoFrame(f32 dt)
	{
		// Update 3D sound transformation (spinning around the listener)

		m_sound3DRotation += dt * 0.9f;
		if (m_sound3DRotation > UE_PI * 2)
			m_sound3DRotation -= UE_PI * 2;

		ueMat44 rotTransform;
		rotTransform.SetTranslation(0, 0, 1000);
		rotTransform.Rotate(ueVec3(0, 1, 0), m_sound3DRotation);

		const ueVec3& translation = rotTransform.GetTranslation();
		auSound_SetPosition(*m_sound3D, translation);

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		
		gxText text;
		text.m_x = 150;
		text.m_y = 100;
		text.m_utf8Buffer = "Just listen to the sound...";
		gxTextRenderer_Draw(ctx, &text);

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	auSoundBank* m_soundBank;
	auSoundHandle m_music;
	auSoundHandle m_sound3D;

	f32 m_sound3DRotation;
};

UE_DECLARE_SAMPLE(ueSample_Sound, "Sound")

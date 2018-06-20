#include "SampleApp.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxMovie.h"

/**
 *	Demonstrates movie playback (video and audio).
 */
class ueSample_Movie : public ueSample
{
public:
	ueBool Init()
	{
		// Startup movie manager

		gxMovieMgr_Startup(g_app.GetFreqAllocator());

		// Play movie

		uePath moviePath;
		ueAssets_GetAssetPath(moviePath, "movie_sample/test_movie", "wmv");

		// FIXME: TEMP, must FIX gxMovie_ffmpeg's reading from stream (not file) !!!
		ueStrCpyS(moviePath, "../../Data/Src/movie_sample/test_movie.wmv");

		UE_ASSERT_FUNC( m_player.Create(moviePath) );
		m_player.EnableLooping(UE_TRUE);
		m_player.Play();

		m_rotation = 0.0f;

		return UE_TRUE;
	}

	void Deinit()
	{
		m_player.Destroy();
		gxMovieMgr_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		// Update polygon rotation

		m_rotation = ueMod(m_rotation + dt * 0.5f, UE_2PI);

		// Update movie

		m_player.Update(dt);

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		gxShapeDrawParams shapeDrawParams;
		shapeDrawParams.m_2DCanvas.Set(-1.0f, -1.0f, 1.0f, 1.0f);
		gxShapeDraw_SetDrawParams(&shapeDrawParams);
		gxShapeDraw_Begin(ctx);
		{
			DrawVideo();
		}
		gxShapeDraw_End();

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

private:

	void DrawVideo()
	{
		UE_PROF_SCOPE("Draw Video");

		ueMat44 rotTransform;
		rotTransform.SetAxisRotation(ueVec3(0, 0, 1), m_rotation);

		gxShape_TexturedRect rect;
		rect.m_transform = &rotTransform;
		rect.m_colorMap = m_player.GetTextureBuffer();
		rect.m_pos.Set(-0.5f, -0.5f, 0.5f, 0.5f);
		rect.m_tex.Set(0.0f, 0.0f, 1.0f, 1.0f);

		gxShapeDraw_DrawTexturedRect(rect);
	}

	f32 m_rotation;
	gxMoviePlayer m_player;
};

UE_DECLARE_SAMPLE(ueSample_Movie, "Movie")
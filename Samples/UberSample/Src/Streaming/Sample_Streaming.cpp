#include "SampleApp.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates asynchronous resource streaming (loading from disc).
 */
class ueSample_Streaming : public ueSample
{
public:
	ueBool Init()
	{
		m_missingTexture.SetByName("common/default_texture");

		m_enableAsyncLoading = UE_TRUE;

		// Initialize objects' boxes

		const ueBox box(-1, -1, -1, 1, 1, 1);
		const f32 distanceOffCenter = 10.0f;

		m_objects[0].m_box = box;
		m_objects[0].m_box.Translate(distanceOffCenter, 0, 0);

		m_objects[1].m_box = box;
		m_objects[1].m_box.Translate(-distanceOffCenter, 0, 0);

		m_objects[2].m_box = box;
		m_objects[2].m_box.Translate(0, 0, distanceOffCenter);

		m_objects[3].m_box = box;
		m_objects[3].m_box.Translate(0, 0, -distanceOffCenter);

		// Create camera

		m_camera.SetLookAt(ueVec3::Zero, ueVec3(0, 0, 10));

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Streaming sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
	}

	void DoFrame(f32 dt)
	{
		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
			m_camera.UpdateFreeCamera(dt);

		// Update streaming

		UpdateStreaming();

		// Draw

		Draw();
	}

	void Draw()
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		// Draw objects

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = &m_camera.GetViewProj();
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		for (u32 i = 0; i < UE_ARRAY_SIZE(m_objects); i++)
		{
			Object& obj = m_objects[i];

			static const ueVec2 tex[] =
			{
				ueVec2(0, 0),
				ueVec2(1, 0),
				ueVec2(1, 1),
				ueVec2(0, 1)
			};
			ueVec3 pos[4];
			obj.m_box.GetCorner(pos[0], UE_TRUE,	UE_TRUE,  UE_TRUE);
			obj.m_box.GetCorner(pos[1], UE_FALSE,	UE_TRUE,  UE_TRUE);
			obj.m_box.GetCorner(pos[2], UE_FALSE,	UE_FALSE, UE_TRUE);
			obj.m_box.GetCorner(pos[3], UE_TRUE,	UE_FALSE, UE_TRUE);

			gxShape_TexturedPoly drawPoly;
			drawPoly.m_enableDepthTest = UE_TRUE;
			drawPoly.m_enableDepthWrite = UE_TRUE;
			drawPoly.m_numVerts = 4;
			drawPoly.m_pos = pos;
			drawPoly.m_tex = tex;
			drawPoly.m_colorMap =
				obj.m_texture.IsReady() ?
				gxTexture_GetBuffer(*obj.m_texture) :
				gxTexture_GetBuffer(*m_missingTexture);

			gxShapeDraw_DrawTexturedPoly(drawPoly);
		}

		gxShapeDraw_End();

		// Draw debug info indicating current texture streaming state

		for (u32 i = 0; i < UE_ARRAY_SIZE(m_objects); i++)
		{
			const Object& obj = m_objects[i];

			const char* stateName = "invalid";
			if (obj.m_texture.IsValid())
			{
				const ueResourceState state = obj.m_texture.GetState();
				switch (state)
				{
					case ueResourceState_Initial			: stateName = "initial"; break;
					case ueResourceState_WaitingForSyncInit	: stateName = "waiting for sync init"; break;
					case ueResourceState_WaitingForAsyncInit: stateName = "waiting for async init"; break;
					case ueResourceState_Initializing		: stateName = "initializing"; break;
					case ueResourceState_Ready				: stateName = "ready"; break;
					case ueResourceState_FailedToInitialize	: stateName = "failed to initialize"; break;
					UE_INVALID_CASE(state)
				}
			}

			char buffer[256];
			ueStrFormatS(buffer, "Texture [%d] state: %s", i, stateName);

			gxText text;
			text.m_x = 150.0f;
			text.m_y = 100.0f + i * 20;
			text.m_utf8Buffer = buffer;
			gxTextRenderer_Draw(ctx, &text);
		}

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void UpdateStreaming()
	{
		const char* textureNames[] =
		{
			"streaming_sample/texture_0",
			"streaming_sample/texture_1",
			"streaming_sample/texture_2",
			"streaming_sample/texture_3"
		};

		UE_ASSERT(UE_ARRAY_SIZE(m_objects) == UE_ARRAY_SIZE(textureNames));

		for (u32 i = 0; i < UE_ARRAY_SIZE(m_objects); i++)
		{
			Object& obj = m_objects[i];
			const ueBool isVisible = m_camera.GetFrustum().Intersect(obj.m_box);

			// Unload the texture if not visible

			if (!isVisible)
				obj.m_texture.Release();

			// Start loading the texture if visible (and if not already loaded or loading is in progress)

			else if (!obj.m_texture.IsValid())
				obj.m_texture.SetByName(textureNames[i], !m_enableAsyncLoading);
		}
	}

	ueBool m_enableAsyncLoading;

	gxCamera m_camera;
	inConsumerId m_inputConsumerId;

	ueResourceHandle<gxTexture> m_missingTexture;

	struct Object
	{
		ueResourceHandle<gxTexture> m_texture;
		ueBox m_box;
	};
	Object m_objects[4];
};

UE_DECLARE_SAMPLE(ueSample_Streaming, "Streaming")
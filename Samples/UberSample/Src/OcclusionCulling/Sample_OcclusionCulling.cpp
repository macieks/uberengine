#include "SampleApp.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates occlusion culling and conditional rendering (if supported).
 */
class ueSample_OcclusionCulling : public ueSample
{
public:
	ueBool Init()
	{
		// Determine query type based on what is supported

		const glCaps* caps = glDevice_GetCaps();

		glQueryType queryType;
		if (caps->m_supportsOcclusionPredicateQuery)
			queryType = glQueryType_OcclusionPredicate;
		else if (caps->m_supportsOcclusionQuery)
			queryType = glQueryType_OcclusionNumSamples;
		else if (caps->m_supportsOcclusionQueryAnySample)
			queryType = glQueryType_OcclusionAnySample;
		else
			return UE_FALSE;

		// Create occlusion queries

		m_box0Query = glQuery_Create(queryType);
		m_box1Query = glQuery_Create(queryType);
		UE_ASSERT(m_box0Query && m_box1Query);

		// Initialize model rotation

		m_rotation = -UE_PI * 0.5f;

		// Create camera

		m_camera.SetLookAt(ueVec3(0, 1.0f, -6.0f), ueVec3(0, 0.5f, 0));

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Occlusion Culling sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		glQuery_Destroy(m_box0Query);
		glQuery_Destroy(m_box1Query);
		inSys_UnregisterConsumer(m_inputConsumerId);
	}

	void DoFrame(f32 dt)
	{
		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
		{
			m_camera.UpdateFreeCamera(dt);
			m_rotation = ueMod(m_rotation + dt * 0.3f, UE_PI * 2);
		}

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = &m_camera.GetViewProj();
		drawParams.m_2DCanvas.Set(0, 0, 1, 1);
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		ueMat44 world;
		world.SetAxisRotation(ueVec3(0, 1, 0), m_rotation);

		gxShape_Box box0;
		box0.m_wireFrame = UE_FALSE;
		box0.m_transform = &world;
		box0.m_color = ueColor32::Red;
		box0.m_box.m_min.Set(0, 0, -3);
		box0.m_box.m_max.Set(2, 2, -1);

		gxShape_Box box1;
		box1.m_wireFrame = UE_FALSE;
		box1.m_transform = &world;
		box1.m_color = ueColor32::Yellow;
		box1.m_box.m_min.Set(0, 0, 1);
		box1.m_box.m_max.Set(2, 2, 3);

		{
			UE_PROF_SCOPE("Draw Depth");

			glCtx_SetDepthFunc(ctx, glCmpFunc_Less);
			glCtx_SetColorWrite(ctx, 0, 0);

			gxShapeDraw_DrawBox(box0);
			gxShapeDraw_DrawBox(box1);
			gxShapeDraw_Flush();

			glCtx_SetColorWrite(ctx, 0, glColorMask_All);
		}

		{
			UE_PROF_SCOPE("Draw Objects");

			glCtx_SetDepthFunc(ctx, glCmpFunc_Equal);

			glCtx_BeginQuery(ctx, m_box0Query);
			{
				gxShapeDraw_DrawBox(box0);
				gxShapeDraw_Flush();
			}
			glCtx_EndQuery(ctx, m_box0Query);

			glCtx_BeginQuery(ctx, m_box1Query);
			{
				gxShapeDraw_DrawBox(box1);
				gxShapeDraw_Flush();
			}
			glCtx_EndQuery(ctx, m_box1Query);

			glCtx_SetDepthFunc(ctx, glCmpFunc_Less);
		}

		{
			UE_PROF_SCOPE("Conditional Draw");

			// Conditional query may be unsupported in which case it does nothing

			glCtx_BeginConditionalRendering(ctx, m_box0Query);
			{
				gxShape_Rect rect0;
				rect0.m_wireFrame = UE_FALSE;
				rect0.m_color = ueColor32::Red;
				rect0.m_rect.Set(0.1f, 0.5f, 0.2f, 0.6f);
				gxShapeDraw_DrawRect(rect0);
				gxShapeDraw_Flush();
			}
			glCtx_EndConditionalRendering(ctx);

			glCtx_BeginConditionalRendering(ctx, m_box1Query);
			{
				gxShape_Rect rect1;
				rect1.m_wireFrame = UE_FALSE;
				rect1.m_color = ueColor32::Yellow;
				rect1.m_rect.Set(0.1f, 0.65f, 0.2f, 0.75f);
				gxShapeDraw_DrawRect(rect1);
				gxShapeDraw_Flush();
			}
			glCtx_EndConditionalRendering(ctx);
		}

		gxShapeDraw_End();

		u32 numBox0Samples;
		u32 numBox1Samples;
		{
			UE_PROF_SCOPE("Get Occlusion Query Results");

			glCtx_GetQueryResult(ctx, m_box0Query, numBox0Samples, UE_TRUE);
			glCtx_GetQueryResult(ctx, m_box1Query, numBox1Samples, UE_TRUE);
		}

		{
			UE_PROF_SCOPE("Draw Occlusion Query Results");

			char buffer[128];
			ueStrFormatS(buffer, "Red Box Samples: %u\nYellow Box Samples: %u", numBox0Samples, numBox1Samples);

			gxText text;
			text.m_x = 150.0f;
			text.m_y = 100.0f;
			text.m_utf8Buffer = buffer;
			gxTextRenderer_Draw(ctx, &text);
		}

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	gxCamera m_camera;

	glOcclusionQuery* m_box0Query;
	glOcclusionQuery* m_box1Query;

	f32 m_rotation;

	inConsumerId m_inputConsumerId;
};

UE_DECLARE_SAMPLE(ueSample_OcclusionCulling, "OcclusionCulling")

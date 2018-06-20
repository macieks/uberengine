#include "SampleApp.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxDebugModelRenderer.h"

/**
 *	Demonstrates rendering to multiple render targets at once.
 */
class ueSample_MultipleRenderTargets : public ueSample
{
public:
	ueBool Init()
	{
		// Check support

		const glCaps* caps = glDevice_GetCaps();
		if (caps->m_maxColorRenderTargets < 4)
			return UE_FALSE;

		// Create render target group (with 4 color and 1 depth render targets)

		glRenderGroup* mainGroup = glDevice_GetMainRenderGroup();
		const glRenderGroupDesc* mainGroupDesc = glRenderGroup_GetDesc(mainGroup);

		glRenderGroupDesc groupDesc;
		groupDesc.m_width = mainGroupDesc->m_width / 2;
		groupDesc.m_height = mainGroupDesc->m_height / 2;
		groupDesc.m_numColorBuffers = 4;
		groupDesc.m_needColorContent = UE_TRUE;
		groupDesc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;	// Color
		groupDesc.m_colorDesc[1].m_format = glBufferFormat_Native_R8G8B8A8;	// Lighting
		groupDesc.m_colorDesc[2].m_format = glBufferFormat_R32F;		// Depth
		groupDesc.m_colorDesc[3].m_format = glBufferFormat_Native_R8G8B8A8;	// Normals
		groupDesc.m_hasDepthStencil = UE_TRUE;
		groupDesc.m_needDepthStencilContent = UE_FALSE;
		groupDesc.m_depthStencilDesc.m_format = glBufferFormat_Depth;	// Best supported depth format

		m_group = glRenderGroup_Create(&groupDesc);
		if (!m_group)
			return UE_FALSE; // Might still be unsupported even though all buffer formats are

		// Load model & shaders

		m_model.SetByName("postprocessing_sample/chair_model");
		m_texture.SetByName("postprocessing_sample/chair_tex");

		m_program.Create("mrt_sample/mrt_model_vs", "mrt_sample/mrt_model_fs");

		// Create camera

		m_camera.SetLookAt(ueVec3(2, 1.5f, 0), ueVec3(0, 1.5f, 0));
		m_camera.SetDrawingToTexture(UE_TRUE);

		// Initialize input

		m_inputConsumerId = inSys_RegisterConsumer("Multiple Render Targets sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		glRenderGroup_Destroy(m_group);
		inSys_UnregisterConsumer(m_inputConsumerId);
	}

	void DoFrame(f32 dt)
	{
		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
			m_camera.UpdateFreeCamera(dt);

		// Draw

		glCtx* ctx = glDevice_GetDefaultContext();
		if (!glCtx_Begin(ctx))
			return;

		glCtx_SetClearColor(ctx, 0, 0, 0, 0);
		glCtx_SetClearDepthStencil(ctx, 1.0f, 0);

		// Draw model to render targets

		glRenderGroup_Begin(m_group, ctx, glClearFlag_All);
		{
			gxDebugModelRendererDrawParams params;
			params.m_ctx = ctx;
			params.m_viewProj = &m_camera.GetViewProj();
			params.m_model = *m_model;
			params.m_texture = gxTexture_GetBuffer(*m_texture);
			params.m_rigidProgram = m_program.GetProgram();

			gxDebugModelRenderer_Draw(&params);
		}
		glRenderGroup_EndDrawing(m_group);
		glTextureBuffer* color = glRenderGroup_AcquireColorOutput(m_group, 0);
		glTextureBuffer* lighting = glRenderGroup_AcquireColorOutput(m_group, 1);
		glTextureBuffer* depth = glRenderGroup_AcquireColorOutput(m_group, 2);
		glTextureBuffer* normals = glRenderGroup_AcquireColorOutput(m_group, 3);
		glRenderGroup_End(m_group);

		// Draw contents of the render targets

		glRenderGroup_Begin(glDevice_GetMainRenderGroup(), ctx, glClearFlag_Color);

		gxShapeDrawParams shapeDrawParams;
		shapeDrawParams.m_2DCanvas.Set(0, 0, 1, 1);
		gxShapeDraw_SetDrawParams(&shapeDrawParams);
		gxShapeDraw_Begin(ctx);
		{
			gxShape_TexturedRect rect;
			rect.m_colorMapSampler = &glSamplerParams::DefaultClamp;

			rect.m_pos.Set(0, 0, 0.5f, 0.5f);
			rect.m_colorMap = color;
			gxShapeDraw_DrawTexturedRect(rect);

			rect.m_pos.Set(0.5f, 0, 1, 0.5f);
			rect.m_colorMap = lighting;
			gxShapeDraw_DrawTexturedRect(rect);

			rect.m_pos.Set(0, 0.5f, 0.5f, 1);
			rect.m_colorMap = depth;
			gxShapeDraw_DrawTexturedRect(rect);

			rect.m_pos.Set(0.5f, 0.5f, 1, 1);
			rect.m_colorMap = normals;
			gxShapeDraw_DrawTexturedRect(rect);
		}
		gxShapeDraw_End();

		// Draw captions

		gxText text;
		text.m_scaleX = text.m_scaleY = 0.8f;
		text.m_color = ueColor32::Yellow;

		text.m_x = 50.0f;
		text.m_y = 50.0f;
		text.m_utf8Buffer = "Color";
		gxTextRenderer_Draw(ctx, &text);

		text.m_x = 550.0f;
		text.m_y = 50.0f;
		text.m_utf8Buffer = "Lighting";
		gxTextRenderer_Draw(ctx, &text);

		text.m_x = 50.0f;
		text.m_y = 350.0f;
		text.m_utf8Buffer = "Depth";
		gxTextRenderer_Draw(ctx, &text);

		text.m_x = 550.0f;
		text.m_y = 350.0f;
		text.m_utf8Buffer = "Normals";
		gxTextRenderer_Draw(ctx, &text);

		// Release render targets back to pool

		glRenderBufferPool_ReleaseTextureBuffer(color);
		glRenderBufferPool_ReleaseTextureBuffer(lighting);
		glRenderBufferPool_ReleaseTextureBuffer(depth);
		glRenderBufferPool_ReleaseTextureBuffer(normals);

		// Finalize rendering

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	gxCamera m_camera;

	gxProgram m_program;
	ueResourceHandle<gxTexture> m_texture;
	ueResourceHandle<gxModel> m_model;

	glRenderGroup* m_group;

	inConsumerId m_inputConsumerId;
};

UE_DECLARE_SAMPLE(ueSample_MultipleRenderTargets, "MultipleRenderTargets")
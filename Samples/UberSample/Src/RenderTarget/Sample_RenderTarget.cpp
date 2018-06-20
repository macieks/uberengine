#include "SampleApp.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxFont.h"

/**
 *	Demonstrates rendering to (mip-map'ed) render target.
 */
class ueSample_RenderTarget : public ueSample
{
public:
	ueBool Init()
	{
		m_rotation = 0.0f;
		m_scale = 0.0f;

		// Check whether auto mip-generation is supported

		const glCaps* caps = glDevice_GetCaps();
		const ueBool supportedAutoMips = caps->m_formatInfos[glBufferFormat_Native_R8G8B8A8].m_supportsAutoMips;

		// Create render target group

		glRenderGroupDesc groupDesc;
		groupDesc.m_width = 512;
		groupDesc.m_height = 512;
		groupDesc.m_numColorBuffers = 1;
		groupDesc.m_needColorContent = UE_TRUE;
		groupDesc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
		if (supportedAutoMips)
		{
			groupDesc.m_colorDesc[0].m_enableAutoMips = UE_TRUE;
			groupDesc.m_colorDesc[0].m_numLevels = 0; // All levels
		}
		groupDesc.m_hasDepthStencil = UE_FALSE;
		m_group = glRenderGroup_Create(&groupDesc);
		UE_ASSERT(m_group);

		return UE_TRUE;
	}

	void Deinit()
	{
		glRenderGroup_Destroy(m_group);
	}

	void DoFrame(f32 dt)
	{
		// Update rectangle rotation

		m_rotation = ueMod(m_rotation + dt * 0.5f, UE_2PI);
		m_scale = ueSin(m_rotation) * 0.9f + 0.9f;

		// Begin drawing

		glCtx* ctx = glDevice_GetDefaultContext();
		if (!glCtx_Begin(ctx))
			return;

		// Draw to render target

		glTextureBuffer* scene = DrawToRenderTarget(ctx);

		// Draw using render target

		DrawUsingRenderTarget(ctx, scene);

		// Return render buffer back to pool

		glRenderBufferPool_ReleaseTextureBuffer(scene);

		// End drawing

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

private:

	glTextureBuffer* DrawToRenderTarget(glCtx* ctx)
	{
		UE_PROF_SCOPE("Draw Scene To Render Target");

		glCtx_SetClearColor(ctx, 0, 0.3f, 0, 0);

		// Inside glRenderGroup_Begin() render target is taken from glRenderBufferPool
		// We must remember to return it back to pool when it's not needed
		//
		// Alternative approach is to manually create render buffer (see glTextureBuffer_Create)
		// and then manually set it (see glRenderGroup_Set*)

		glRenderGroup_Begin(m_group, ctx, glClearFlag_Color);

		// Draw

		gxText text;
		text.m_x = 20.0f;
		text.m_y = 100.0f + ueSin(m_rotation) * 100.0f;
		text.m_color = ueColor32::Yellow;
		text.m_utf8Buffer =
			"Some text\n"
			"rendered to\n"
			"512x512 render target\n"
			"with automatic mip-maps.";
		gxTextRenderer_Draw(ctx, &text);

		// End

		glRenderGroup_EndDrawing(m_group);
		glTextureBuffer* result = glRenderGroup_AcquireColorOutput(m_group, 0);
		glRenderGroup_End(m_group);

		return result;
	}

	void DrawUsingRenderTarget(glCtx* ctx, glTextureBuffer* texture)
	{
		UE_PROF_SCOPE("Draw Using Render Target");

		// Begin

		glCtx_SetClearColor(ctx, 0, 0, 0, 0);
		glCtx_SetClearDepthStencil(ctx, 1.0f, 0);

		glRenderGroup_Begin(glDevice_GetMainRenderGroup(), ctx);

		gxShapeDrawParams shapeDrawParams;
		shapeDrawParams.m_2DCanvas.Set(-1.0f, -1.0f, 1.0f, 1.0f);
		gxShapeDraw_SetDrawParams(&shapeDrawParams);
		gxShapeDraw_Begin(ctx);

		// Draw

		ueMat44 transform;
		transform.SetAxisRotation(ueVec3(0, 0, 1), m_rotation);
		transform.Scale(m_scale);

		gxShape_TexturedRect rect;
		rect.m_transform = &transform;
		rect.m_colorMap = texture;
		rect.m_pos.Set(-0.5f, -0.5f, 0.5f, 0.5f);
		rect.m_tex.Set(0.0f, 0.0f, 1.0f, 1.0f);

		gxShapeDraw_DrawTexturedRect(rect);

		// End

		gxShapeDraw_End();
	}

	f32 m_rotation;
	f32 m_scale;
	glRenderGroup* m_group;
};

UE_DECLARE_SAMPLE(ueSample_RenderTarget, "RenderTarget")
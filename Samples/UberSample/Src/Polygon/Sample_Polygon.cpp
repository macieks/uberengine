#include "SampleApp.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "Utils/utWarningSys.h"

/**
 *	Demonstrates how to draw textured polygon using low-level engine functionality:
 *	- stream format
 *	- index & vertex buffers
 *	- shader
 *	- texture
 *	- shader constants
 *	- render buffer group
 */
class ueSample_Polygon : public ueSample
{
public:
	ueBool Init()
	{
		// Load resources (sync load)

		m_program.Create("common/pos_col_tex_vs", "common/pos_col_tex_fs");

		m_texture.SetByName("common/default_texture");

		// Create vertex buffer

		const f32 z = 1.0f;
		const MyVertex verts[] =
		{
			{-0.4f,  -0.6f, z, 0.0f,  0.0f, 1, 0, 0, 1},
			{0.4f,   -0.6f, z, 0.8f,  0.0f, 1, 1, 0, 1},
			{0.7f,	  0.4f, z, 1.1f,  1.0f, 0, 1, 0, 1},
			{0,       0.7f, z, 0.4f,  1.2f, 0, 1, 1, 1},
			{-0.7f,   0.4f, z, -0.3f, 1.0f, 0, 0, 1, 1}
		};

		glVertexBufferDesc vbDesc;
		vbDesc.m_size = sizeof(verts);
		m_VB = glVertexBuffer_Create(&vbDesc, verts);
		UE_ASSERT(m_VB);

		// Create stream format

		const glVertexElement vfElems[] = 
		{
			{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(MyVertex, x)},
			{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(MyVertex, u)},
			{glSemantic_Color, 0, ueNumType_F32, 4, UE_FALSE, UE_OFFSET_OF(MyVertex, r)}
		};

		glStreamFormatDesc vfDesc;
		vfDesc.m_stride = sizeof(MyVertex);
		vfDesc.m_numElements = UE_ARRAY_SIZE(vfElems);
		vfDesc.m_elements = vfElems;
		m_SF = glStreamFormat_Create(&vfDesc);
		UE_ASSERT(m_SF);

		// Create index buffer

		const u16 indices[] = {0, 1, 4, 2, 3};

		glIndexBufferDesc ibDesc;
		ibDesc.m_numIndices = UE_ARRAY_SIZE(indices);
		ibDesc.m_indexSize = sizeof(u16);
		m_IB = glIndexBuffer_Create(&ibDesc, indices);
		UE_ASSERT(m_IB);

		// Misc.

		m_rotation = 0.0f;

		return UE_TRUE;
	}

	void Deinit()
	{
		glVertexBuffer_Destroy(m_VB);
		glIndexBuffer_Destroy(m_IB);
		glStreamFormat_Destroy(m_SF);
	}

	void DoFrame(f32 dt)
	{
		// Update polygon rotation

		m_rotation = ueMod(m_rotation + dt * 0.5f, UE_2PI);

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		DrawPolygon(ctx);
		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void DrawPolygon(glCtx* ctx)
	{
		UT_WARNING_CHECK(2 < 1, "Hello! This is run-time warning !!!");
		UT_WARNING("Hi! This is another run-time warning !!!");

		// Set render states

		glCtx_SetBlending(ctx, UE_FALSE);
		glCtx_SetCullMode(ctx, glCullMode_None);
		glCtx_SetDepthWrite(ctx, UE_FALSE);
		glCtx_SetDepthTest(ctx, UE_FALSE);

		// Set shader constants

		ueMat44 W;
		W.SetAxisRotation(ueVec3(0, 0, 1), m_rotation);

		ueMat44 VP;
		VP.SetOrthoOffCenter(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f, glCtx_IsRenderingToTexture(ctx));

		ueMat44 WVP;
		ueMat44::Mul(WVP, W, VP);
		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldViewProj, &WVP);

		glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, gxTexture_GetBuffer(*m_texture));

		// Set index & vertex data

		glCtx_SetStream(ctx, 0, m_VB, m_SF);
		glCtx_SetIndices(ctx, m_IB);

		// Set program

		glCtx_SetProgram(ctx, m_program.GetProgram());

		// Draw

		glCtx_DrawIndexed(ctx, glPrimitive_TriangleStrip, 0, 0, 5, 0, 5);
	}

	struct MyVertex
	{
		f32 x, y, z;
		f32 u, v;
		f32 r, g, b, a;
	};

	f32 m_rotation;

	gxProgram m_program;

	ueResourceHandle<gxTexture> m_texture;

	glVertexBuffer* m_VB;
	glIndexBuffer* m_IB;
	glStreamFormat* m_SF;
};

UE_DECLARE_SAMPLE(ueSample_Polygon, "Polygon")
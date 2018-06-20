#include "GX/gxEffect_Private.h"

#define MAX_PARTICLES	512
#define MAX_VERTS		(MAX_PARTICLES * 4)

struct Data
{
	CIwSVec3 m_pos[MAX_VERTS];
	CIwColour m_color[MAX_VERTS];
	CIwSVec2 m_uv[MAX_VERTS];

	ueMat44 m_view;
};

static Data* s_data = NULL;

const f32 s_quadPos[8] =
{
	-0.5f, -0.5f,
	0.5f, -0.5f,
	0.5f, 0.5f,
	-0.5f, 0.5f
};

const f32 s_quadTex[8] =
{
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
};

void gxEffectTypeMgr_Startup_SKU()
{
	s_data = new(s_fxSysData->m_allocator) Data();
	ueAssert(s_data);

	// Init UV channel (never changes)

	CIwSVec2* uv = s_data->m_uv;

	u32 v = 0;
	for (u32 i = 0; i < MAX_PARTICLES; i++)
		for (u32 j = 0; j < 4; j++, v++)
		{
			uv[v].x = GL_TO_MARMALADE_UV(s_quadTex[j * 2]);
			uv[v].y = GL_TO_MARMALADE_UV(s_quadTex[j * 2 + 1]);
		}
}

void gxEffectTypeMgr_Shutdown_SKU()
{
	ueDelete(s_data, s_fxSysData->m_allocator);
	s_data = NULL;
}

void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueMat44& proj)
{
	ueNotYetImplemented();
}

void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueRect& projRect)
{
	IwGxSetOrtho(
		(iwfixed) GL_TO_MARMALADE_COORD(projRect.m_left),
		(iwfixed) GL_TO_MARMALADE_COORD(projRect.m_right),
		(iwfixed) GL_TO_MARMALADE_COORD(projRect.m_top),
		(iwfixed) GL_TO_MARMALADE_COORD(projRect.m_bottom),
		-1, -1);

	ueMat44_Copy(s_data->m_view, view);
}

void gxEffectTypeMgr_EndDraw()
{
	//IwGxFlush();
}

void gxEmitter::Draw(glCtx* ctx)
{
	if (m_state != State_Running && m_state != State_BeingDestroyed)
		return;

	if (m_set.m_count == 0)
		return;

	// Set world transform

	CIwMat worldViewM;
	if (m_type->m_localSpaceSimulation)
	{
		ueMat44 world;
		ueMat44_SetTranslationV3(world, m_pos);
		ueMat44_Rotate(world, m_rot);
		ueMat44_Scale(world, m_scale);

		ueMat44 worldView;
		ueMat44_Mul(worldView, world, s_data->m_view);

		glUtils_ToMarmaladeMatrix(&worldViewM, &worldView);
	}
	else
		glUtils_ToMarmaladeMatrix(&worldViewM, &s_data->m_view);

	IwGxSetModelMatrix(&worldViewM);

	// Set up vertex data

	const u32 numVerts = m_set.m_count * 4;
	CIwSVec3* pos = s_data->m_pos;
	CIwColour* color = s_data->m_color;

	const int16 zCoord = GL_TO_MARMALADE_COORD(2.0f);

	u32 v = 0;
	const gxParticle* p = m_set.m_particles;
	for (u32 i = 0; i < m_set.m_count; i++, p++)
	{
		f32 center[3];
		ueVec3_GetPtr(center, p->m_pos);

		f32 size[3];
		ueVec3_GetPtr(size, p->m_size);

		ueColor32 finalColor = ueColor32(p->m_color) * m_color;

		CIwColour colorAsShaderAttr;
		colorAsShaderAttr = finalColor.AsShaderAttr();

		const f32 rotSin = ueSin(p->m_rot);
		const f32 rotCos = ueCos(p->m_rot);

		for (u32 j = 0; j < 4; j++, v++)
		{
			f32 tmp[2];
			tmp[0] = s_quadPos[j * 2] * size[0];
			tmp[1] = s_quadPos[j * 2 + 1] * size[1];

			f32 quadPos[2];
			quadPos[0] = rotCos * tmp[0] - rotSin * tmp[1];
			quadPos[1] = rotSin * tmp[0] + rotCos * tmp[1];

			pos[v].x = GL_TO_MARMALADE_COORD(quadPos[0] + center[0]);
			pos[v].y = GL_TO_MARMALADE_COORD(quadPos[1] + center[1]);
			pos[v].z = zCoord;

			color[v] = colorAsShaderAttr;
		}
	}

	// Set data

	IwGxSetVertStream(pos, numVerts);
	IwGxSetUVStream(s_data->m_uv);
	IwGxSetColStream(color, numVerts);

	// Set material

	gxTexture* texture = *m_type->m_textures[gxEmitterType::Texture_ColorMap];
	glTextureBuffer* tb = texture->GetBuffer();
	CIwMaterial* material = glTextureBuffer_GetIwMaterial(tb);
	IwGxSetMaterial(material);

	// Draw

	IwGxDrawPrims(IW_GX_QUAD_LIST, NULL, numVerts);
	IwGxFlush();
}
#include "GraphicsExt/gxEffect_Private.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "Graphics/glVertexBufferFactory.h"

struct fxSysData_Rendering
{
	glVertexBufferFactory m_VBF;
	glIndexBuffer* m_quadIB;
	glStreamFormat* m_SF;

	gxProgram m_programs[gxEmitterType::DrawMethod_MAX];

	ueMat44 m_view;
	ueMat44 m_proj;
	ueMat44 m_viewProj;
	ueVec3 m_rightVec;
	ueVec3 m_upVec;

	struct ShaderConstants
	{
		glConstantHandle ParticleScale;
	} m_shaderConstants;
};

static fxSysData_Rendering s_fxSysData_Rendering;

struct fxParticleVertex_Rendering
{
	f32 pos[3];
	u32 color;
	f32 size2_tex2[4];
	f32 vel3_rot[4];
};

static f32 s_quadTex[] =
{
	0, 0,
	0, 1,
	1, 1,
	1, 0
};

void gxEffectTypeMgr_Startup_SKU()
{
	// Initialize vertex buffer factory

	const u32 maxParticlesDrawCount = 16384;
	s_fxSysData_Rendering.m_VBF.Init(maxParticlesDrawCount * sizeof(fxParticleVertex_Rendering) * 4);

	// Initialize quad index buffer

	UE_ASSERT(maxParticlesDrawCount * 4 <= UE_POW2(16));

	glIndexBufferDesc ibDesc;
	ibDesc.m_indexSize = sizeof(u16);
	ibDesc.m_numIndices = maxParticlesDrawCount * 6;
	ibDesc.m_flags = glBufferFlags_WriteOnly;
	s_fxSysData_Rendering.m_quadIB = glIndexBuffer_Create(&ibDesc);
	UE_ASSERT(s_fxSysData_Rendering.m_quadIB);

	u16* quadIndices = (u16*) glIndexBuffer_Lock(s_fxSysData_Rendering.m_quadIB, glBufferLockFlags_Write);
	for (u32 i = 0; i < maxParticlesDrawCount; i++)
	{
		const u16 baseVertexIndex = (u16) (i * 4);
		const u32 baseIndex = i * 6;

		quadIndices[baseIndex + 0] = baseVertexIndex;
		quadIndices[baseIndex + 1] = baseVertexIndex + 1;
		quadIndices[baseIndex + 2] = baseVertexIndex + 2;

		quadIndices[baseIndex + 3] = baseVertexIndex;
		quadIndices[baseIndex + 4] = baseVertexIndex + 2;
		quadIndices[baseIndex + 5] = baseVertexIndex + 3;
	}
	glIndexBuffer_Unlock(s_fxSysData_Rendering.m_quadIB);

	// Initialize stream format

	const glVertexElement vfElems[] =
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(fxParticleVertex_Rendering, pos)},
		{glSemantic_Color,	  0, ueNumType_U8,  4, UE_TRUE,  UE_OFFSET_OF(fxParticleVertex_Rendering, color)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 4, UE_FALSE, UE_OFFSET_OF(fxParticleVertex_Rendering, size2_tex2)},
		{glSemantic_TexCoord, 1, ueNumType_F32, 4, UE_FALSE, UE_OFFSET_OF(fxParticleVertex_Rendering, vel3_rot)}
	};

	glStreamFormatDesc vfDesc;
	vfDesc.m_elements = vfElems;
	vfDesc.m_numElements = UE_ARRAY_SIZE(vfElems);
	vfDesc.m_stride = sizeof(fxParticleVertex_Rendering);
	s_fxSysData_Rendering.m_SF = glStreamFormat_Create(&vfDesc);
	UE_ASSERT(s_fxSysData_Rendering.m_SF);

	// Shaders

	for (u32 i = 0; i < gxEmitterType::DrawMethod_MAX; i++)
	{
		const char* vsName;
		const char* fsName;

		switch (i)
		{
			case gxEmitterType::DrawMethod_Quad:
				vsName = "common/particle_quad_vs";
				fsName = "common/particle_default_fs";
				break;
			case gxEmitterType::DrawMethod_Spark:
				vsName = "common/particle_quad_vs"; // FIXME
				fsName = "common/particle_default_fs";
				break;
			UE_INVALID_CASE(i)
		}

		s_fxSysData_Rendering.m_programs[i].Create(vsName, fsName);
	}

	// Shader constants

	s_fxSysData_Rendering.m_shaderConstants.ParticleScale.Init("ParticleScale", glConstantType_Float2);
}

void gxEffectTypeMgr_Shutdown_SKU()
{
	for (u32 i = 0; i < gxEmitterType::DrawMethod_MAX; i++)
		s_fxSysData_Rendering.m_programs[i].Destroy();

	glStreamFormat_Destroy(s_fxSysData_Rendering.m_SF);
	glIndexBuffer_Destroy(s_fxSysData_Rendering.m_quadIB);
	s_fxSysData_Rendering.m_VBF.Deinit();
}

void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueMat44& proj)
{
	s_fxSysData_Rendering.m_view = view;
	s_fxSysData_Rendering.m_rightVec = view.GetRightVec();
	s_fxSysData_Rendering.m_upVec = view.GetUpVec();

	s_fxSysData_Rendering.m_proj = proj;

	ueMat44::Mul(s_fxSysData_Rendering.m_viewProj, s_fxSysData_Rendering.m_view, s_fxSysData_Rendering.m_proj);
}

void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueRect& projRect)
{
	ueMat44 orthoProj;
	orthoProj.SetOrthoOffCenter(projRect.m_left, projRect.m_right, projRect.m_top, projRect.m_bottom, -1, 1, glCtx_IsRenderingToTexture(ctx));
	gxEffectTypeMgr_BeginDraw(ctx, view, orthoProj);
}

void gxEffectTypeMgr_EndDraw()
{
}

void gxEmitter::Draw(glCtx* ctx)
{
	if (m_state != State_Running && m_state != State_BeingDestroyed)
		return;

	if (m_set.m_count == 0)
		return;

	// Fill up vertex buffer

	glVertexBufferFactory::LockedChunk vbChunk;
	if (!s_fxSysData_Rendering.m_VBF.Allocate(sizeof(fxParticleVertex_Rendering) * m_set.m_count * 4, &vbChunk))
		return;

	fxParticleVertex_Rendering* verts = (fxParticleVertex_Rendering*) vbChunk.m_data;
	for (u32 i = 0; i < m_set.m_count; i++)
	{
		gxParticle* p = &m_set.m_particles[i];

		p->m_pos.Store3(verts->pos);
		verts->color = ueColor32(p->m_color).AsShaderAttr();
		p->m_vel.Store3(verts->vel3_rot);
		verts->vel3_rot[3] = p->m_rot;
		for (u32 j = 1; j < 4; j++)
			verts[j] = *verts;
		for (u32 j = 0; j < 4; j++)
		{
			p->m_size.Store2(verts[j].size2_tex2);
			verts[j].size2_tex2[2] = s_quadTex[j * 2];
			verts[j].size2_tex2[3] = s_quadTex[j * 2 + 1];
		}

		verts += 4;
	}
	s_fxSysData_Rendering.m_VBF.Unlock(&vbChunk);

	// Set render states

	glCtx_SetBlending(ctx, UE_TRUE);
	glCtx_SetBlendFunc(ctx, m_type->m_srcBlend, m_type->m_dstBlend);
	glCtx_SetAlphaTest(ctx, m_type->m_enableAlphaTest);
	glCtx_SetAlphaFunc(ctx, m_type->m_alphaFunc, m_type->m_alphaRef);
	glCtx_SetDepthTest(ctx, m_type->m_enableDepthTest);
	glCtx_SetDepthWrite(ctx, m_type->m_enableDepthWrite);
	glCtx_SetCullMode(ctx, glCullMode_None);

	// Set constants

	glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, gxTexture_GetBuffer(*m_type->m_textures[gxEmitterType::Texture_ColorMap]));
	if (m_type->m_textures[gxEmitterType::Texture_NormalMap].IsValid())
		glCtx_SetSamplerConstant(ctx, gxCommonConstants::NormalMap, gxTexture_GetBuffer(*m_type->m_textures[gxEmitterType::Texture_NormalMap]));

	ueMat44 world;
	if (m_type->m_localSpaceSimulation)
	{
		world.SetTranslation(m_pos);
		world.Rotate(m_rot);
		world.Scale(m_scale);
	}
	else
		world.SetIdentity();
	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, &world);

	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, &s_fxSysData_Rendering.m_viewProj);
	glCtx_SetFloat3Constant(ctx, gxCommonConstants::RightVec, &s_fxSysData_Rendering.m_rightVec);
	glCtx_SetFloat3Constant(ctx, gxCommonConstants::UpVec, &s_fxSysData_Rendering.m_upVec);

	glCtx_SetFloat4Constant(ctx, gxCommonConstants::Color, &m_color);

	glCtx_SetFloat2Constant(ctx, s_fxSysData_Rendering.m_shaderConstants.ParticleScale, &ueVec2::One);

	// Set data and shaders

	glCtx_SetStream(ctx, 0, vbChunk.m_VB, s_fxSysData_Rendering.m_SF, vbChunk.m_offset);
	glCtx_SetIndices(ctx, s_fxSysData_Rendering.m_quadIB);
	glCtx_SetProgram(ctx, s_fxSysData_Rendering.m_programs[m_type->m_drawMethod].GetProgram());

	// Draw

	glCtx_DrawIndexed(ctx, glPrimitive_TriangleList, 0, 0, m_set.m_count * 4, 0, 6 * m_set.m_count);
}
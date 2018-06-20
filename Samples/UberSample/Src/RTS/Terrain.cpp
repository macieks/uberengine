#include "Base/ueRand.h"
#include "Input/inMouse.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCamera.h"
#include "Physics/phLib.h"
#include "Terrain.h"
#include "Cursor.h"
#include "Sample_RTS.h"
#include "SampleApp.h"

void Terrain::Init(ueAllocator* allocator, u32 dimPow, f32 cellSize)
{
	m_allocator = allocator;
	m_dimPow = dimPow;
	m_dim = 1 << m_dimPow;
	m_dimMask = m_dim - 1;
	m_cellSize = cellSize;
	m_heights = (f32*) m_allocator->Alloc(sizeof(f32) * m_dim * m_dim);
	UE_ASSERT(m_heights);
}

void Terrain::Deinit()
{
	m_allocator->Free(m_heights);
	m_heights = NULL;
	m_allocator = NULL;
}

void Terrain::GenerateRandomTerrain(s32 sampleSize)
{
	ueRand* rand = ueRand_Create(0x12345678);

	ueMemSet(m_heights, 0, sizeof(f32) * m_dim * m_dim);

	const u32 numSamples = m_dim * m_dim / (sampleSize * sampleSize) / 3;
	const f32 normalizer = 1.0f / ueSqrt(2.0f * ueSqr(sampleSize));

	for (u32 i = 0; i < numSamples; i++)
	{
		const u32 x = ueRand_U32(rand, m_dim - 1);
		const u32 z = ueRand_U32(rand, m_dim - 1);

		const f32 strength = ueRand_F32(rand);

		for (s32 xs = -sampleSize; xs <= sampleSize; xs++)
			for (s32 zs = -sampleSize; zs <= sampleSize; zs++)
			{
				const s32 xp = x + xs;
				const s32 zp = z + zs;

				f32 h;

				const ueVec2 xz((f32) x, (f32) z);
				const ueVec2 xzp((f32) xp, (f32) zp);
				
				h = ueVec2::Dist(xz, xzp);
				h *= normalizer;
				h = 1.0f - uePow(h, 1.5f);
				h *= strength;

				m_heights[(xp & m_dimMask) * m_dim + (zp & m_dimMask)] += h;
			}
	}

	for (u32 x = 0; x < m_dim; x++)
		for (u32 z = 0; z < m_dim; z++)
		{
			f32& y = m_heights[x * m_dim + z];
			if (y > 0.5f) y *= 3.0f;
		}

	ueRand_Destroy(rand);
}

void Terrain::CreateRenderData()
{
	// Load resources (sync load)

	m_program.Create("rts_sample/terrain_vs", "rts_sample/terrain_fs");

	for (u32 i = 0; i < NUM_TERRAIN_TEXTURES; i++)
	{
		char terrainTextureName[512];
		ueStrFormatS(terrainTextureName, "rts_sample/terrain%u", i);
		m_textures[i].SetByName(terrainTextureName);
	}
	m_detailTexture.SetByName("rts_sample/detailmap");

	// Init shader handles

	for (u32 i = 0; i < NUM_TERRAIN_TEXTURES; i++)
	{
		char terrainMapName[512];
		ueStrFormatS(terrainMapName, "TerrainMap%u", i);
		UE_ASSERT_FUNC( m_shaderConstants.TerrainMaps[i].Init(terrainMapName, glConstantType_Sampler2D) );
	}

	// Create vertex format

	const glVertexElement vfElems[] = 
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(TerrainDrawVertex, x)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(TerrainDrawVertex, u)},
		{glSemantic_Normal, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(TerrainDrawVertex, normal)},
		{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(TerrainDrawVertex, rgba)}
	};

	glStreamFormatDesc vfDesc;
	vfDesc.m_stride = sizeof(TerrainDrawVertex);
	vfDesc.m_numElements = UE_ARRAY_SIZE(vfElems);
	vfDesc.m_elements = vfElems;
	m_SF = glStreamFormat_Create(&vfDesc);
	UE_ASSERT(m_SF);

	// Allocate index & vertex data

	m_numVerts = m_dim * m_dim;
	m_pos = (f32*) m_allocator->Alloc(sizeof(f32) * (m_numVerts * 3));
	m_numIndices = 3 * (m_dim - 1) * (m_dim - 1) * 2;
	m_indices = (u16*) m_allocator->Alloc(sizeof(u16) * m_numIndices);

	// Create vertex buffer

	glVertexBufferDesc vbDesc;
	vbDesc.m_size = sizeof(TerrainDrawVertex) * m_dim * m_dim;
	m_VB = glVertexBuffer_Create(&vbDesc);
	UE_ASSERT(m_VB);

	void* vbData = glVertexBuffer_Lock(m_VB);
	UE_ASSERT(vbData);

	const f32 uvScale = 10.0f;

	const u8_4N compressedDefaultNormal(0.0f, 1.0f, 0.0f, 0.0f);
	const u8_4N whiteRGBA = ueColor32::White.AsRGBA();

	TerrainDrawVertex* verts = (TerrainDrawVertex*) vbData;
	for (u32 x = 0; x < m_dim; x++)
		for (u32 z = 0; z < m_dim; z++)
		{
			TerrainDrawVertex* v = verts;
			v->x = (f32) x * m_cellSize;
			v->y = GetHeight(Point(x, z));
			v->z = (f32) z * m_cellSize;
			v->u = (f32) x / (f32) (m_dim - 1) * uvScale;
			v->v = (f32) z / (f32) (m_dim - 1) * uvScale;
			v->normal = compressedDefaultNormal;
			v->rgba = whiteRGBA;

			verts++;
		}

	verts = (TerrainDrawVertex*) vbData + m_dim + 1;
	for (u32 x = 1; x < m_dim - 1; x++)
		for (u32 z = 1; z < m_dim - 1; z++)
		{
			TerrainDrawVertex* v = verts;

			TerrainDrawVertex* v0 = verts + 1;
			TerrainDrawVertex* v1 = verts - 1;
			TerrainDrawVertex* v2 = verts + m_dim;
			TerrainDrawVertex* v3 = verts - m_dim;

			ueVec3 a, b, dirX, dirZ;

			dirX = ueVec3(&v1->x) - ueVec3(&v0->x);
			dirZ = ueVec3(&v3->x) - ueVec3(&v2->x);

			ueVec3 normal;
			ueVec3::Cross(normal, dirX, dirZ);
			normal.Normalize();
			normal *= 0.5f;
			normal += 0.5f;

			v->normal = normal;

			verts++;
		}

	verts = (TerrainDrawVertex*) vbData;
	for (u32 i = 0; i < m_numVerts; i++)
	{
		m_pos[i * 3 + 0] = verts[i].x;
		m_pos[i * 3 + 1] = verts[i].y;
		m_pos[i * 3 + 2] = verts[i].z;
	}

	glVertexBuffer_Unlock(m_VB);

	// Create index buffer

	glIndexBufferDesc ibDesc;
	ibDesc.m_numIndices = 3 * (m_dim - 1) * (m_dim - 1) * 2;
	UE_ASSERT(ibDesc.m_numIndices < (1 << 16));
	ibDesc.m_indexSize = sizeof(u16);
	m_IB = glIndexBuffer_Create(&ibDesc);
	UE_ASSERT(m_IB);

	void* ibData = glIndexBuffer_Lock(m_IB);
	UE_ASSERT(ibData);

	u16* indices = (u16*) ibData;
	for (u32 x = 0; x < m_dim - 1; x++)
		for (u32 z = 0; z < m_dim - 1; z++)
		{
			const u32 v0 = x * m_dim + z;
			const u32 v1 = (x + 1) * m_dim + z;
			const u32 v2 = (x + 1) * m_dim + z + 1;
			const u32 v3 = x * m_dim + z + 1;

			indices[0] = v0;
			indices[1] = v2;
			indices[2] = v1;

			indices[3] = v0;
			indices[4] = v3;
			indices[5] = v2;

			indices += 6;
		}

	ueMemCpy(m_indices, ibData, sizeof(u16) * m_numIndices);

	glIndexBuffer_Unlock(m_IB);
}

void Terrain::DestroyRenderData()
{
	for (u32 i = 0; i < NUM_TERRAIN_TEXTURES; i++)
		m_textures[i] = NULL;
	m_program.Destroy();

	glVertexBuffer_Destroy(m_VB);
	glIndexBuffer_Destroy(m_IB);
	glStreamFormat_Destroy(m_SF);

	m_allocator->Free(m_pos);
	m_allocator->Free(m_indices);
}

void Terrain::UpdateOverField()
{
	UE_PROF_SCOPE("Terrain::UpdateOverField");

	// Get ray from camera to cursor in world space
	ueVec3 end;
	Cursor::GetWorldPos(end);
	const ueVec3& start = g_camera->GetPosition();
	ueVec3 dir;
	dir.MakeNormal(start, end);
	end = start + dir * 1000.0f;

	// Trace ray
	m_isOverField = TraceRay(start, end, m_overPos, m_overPoint);
}

ueBool Terrain::GetOverField(Point& p) const
{
	p = m_overPoint;
	return m_isOverField;
}

void Terrain::Draw(glCtx* ctx)
{
	UE_PROF_SCOPE("Terrain::Draw");

	glCtx_SetBlending(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_CCW);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetDepthTest(ctx, UE_TRUE);

	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, &g_camera->GetViewProj(), 1);

	for (u32 i = 0; i < NUM_TERRAIN_TEXTURES; i++)
		glCtx_SetSamplerConstant(ctx, m_shaderConstants.TerrainMaps[i], gxTexture_GetBuffer(*m_textures[i]));
	glCtx_SetSamplerConstant(ctx, gxCommonConstants::DetailMap, gxTexture_GetBuffer(*m_detailTexture));

	glCtx_SetStream(ctx, 0, m_VB, m_SF);
	glCtx_SetIndices(ctx, m_IB);

	glCtx_SetProgram(ctx, m_program.GetProgram());

//	glCtx_SetFillMode(ctx, glFillMode_Wire);
	glCtx_DrawIndexed(ctx, glPrimitive_TriangleList, 0, 0, m_dim * m_dim, 0, 3 * (m_dim - 1) * (m_dim - 1) * 2);
//	glCtx_SetFillMode(ctx, glFillMode_Solid);
}

void Terrain::CreateCollisionData()
{
	phMeshDesc desc;
	desc.m_isConvex = UE_FALSE;
	desc.m_numVerts = m_numVerts;
	desc.m_posStride = sizeof(f32) * 3;
	desc.m_pos = m_pos;
	desc.m_indexSize = sizeof(u16);
	desc.m_indexData = m_indices;
	desc.m_numIndices = m_numIndices;

	phMeshShapeDesc meshShapeDesc;
	meshShapeDesc.m_meshDesc = &desc;
	m_shape = phShape_Create(&meshShapeDesc);
	UE_ASSERT(m_shape);

	phSceneDesc sceneDesc;
	m_scene = phScene_Create(&sceneDesc);
	UE_ASSERT(m_scene);

	phBodyDesc bodyDesc;
	bodyDesc.m_shape = m_shape;
	m_body = phBody_Create(m_scene, &bodyDesc);
	UE_ASSERT(m_body);
}

void Terrain::DestroyCollisionData()
{
	if (m_scene)
	{
		phBody_Destroy(m_body);
		m_body = NULL;
		phScene_Destroy(m_scene);
		m_scene = NULL;
		phShape_Destroy(m_shape);
		m_shape = NULL;
	}
}

ueBool Terrain::TraceRay(const ueVec3& start, const ueVec3& end, ueVec3& hitPos, Point& hitPoint)
{
	phRayIntersectionTestDesc testDesc;
	testDesc.m_maxNumResults = 1;
	testDesc.m_start = start;
	testDesc.m_end = end;

	ueVec3 dir;
	dir.MakeNormal(start, end);

	phShapeIntersectionResult result;
	phShapeIntersectionResultSet results;
	results.m_results = &result;
	if (phScene_IntersectRay(m_scene, &testDesc, &results))
	{
		hitPos = start + dir * result.m_distance;
		hitPoint = TriangleIndexToPoint(result.m_triangleIndex);
		return UE_TRUE;
	}
	return UE_FALSE;
}

Point Terrain::TriangleIndexToPoint(u32 triangleIndex) const
{
	const u32 quadIndex = triangleIndex / 2;
	Point p;
	p.y = quadIndex % (m_dim - 1);
	p.x = quadIndex / (m_dim - 1);
	return p;
}
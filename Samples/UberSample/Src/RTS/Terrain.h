#pragma once

#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "Physics/phLib.h"

#include "Point.h"

class Terrain
{
public:
	void Init(ueAllocator* allocator, u32 dimPow, f32 size);
	void Deinit();

	void UpdateOverField();

	void GenerateRandomTerrain(s32 sampleSize);

	void CreateRenderData();
	void DestroyRenderData();
	void Draw(glCtx* ctx);

	void CreateCollisionData();
	void DestroyCollisionData();
	ueBool TraceRay(const ueVec3& start, const ueVec3& end, ueVec3& hitPos, Point& hitPoint);

	UE_INLINE u32 GetDim() const { return m_dim; }
	UE_INLINE f32 GetCellSize() const { return m_cellSize; }
	UE_INLINE f32 GetHeight(Point p) const { UE_ASSERT(0 <= p.x && p.x < (s32) m_dim && 0 <= p.y && p.y < (s32) m_dim); return m_heights[p.x * m_dim + p.y]; }
	UE_INLINE f32 GetHeight(const ueVec2& p) const { return GetHeight(Point((s16) p[0], (s16) p[1])); }

	UE_INLINE void ToWorldPos(ueVec3& out, Point p) const { out.Set((f32) p.x * m_cellSize, GetHeight(p), (f32) p.y * m_cellSize); }
	UE_INLINE void ToWorldPos(ueVec3& out, const ueVec2& p) const { out.Set(p[0] * m_cellSize, GetHeight(p), p[1] * m_cellSize); }

	Point TriangleIndexToPoint(u32 triangleIndex) const;
	ueBool GetOverField(Point& p) const;

private:
	ueAllocator* m_allocator;

	u32 m_dim;
	u32 m_dimPow;
	u32 m_dimMask;
	f32 m_cellSize;
	f32* m_heights;

	// Mesh data

	u32 m_numVerts;
	f32* m_pos;
	u32 m_numIndices;
	u16* m_indices;

	// Render data

	struct TerrainDrawVertex
	{
		f32 x, y, z;
		f32 u, v;
		u8_4N normal;
		u8_4N rgba;
	};

	glVertexBuffer* m_VB;
	glStreamFormat* m_SF;
	glIndexBuffer* m_IB;

#define NUM_TERRAIN_TEXTURES 3

	struct ShaderConstants
	{
		glConstantHandle TerrainMaps[NUM_TERRAIN_TEXTURES];
	} m_shaderConstants;

	gxProgram m_program;
	ueResourceHandle<gxTexture> m_textures[NUM_TERRAIN_TEXTURES];
	ueResourceHandle<gxTexture> m_detailTexture;

	// Collision data

	phShape* m_shape;
	phBody* m_body;
	phScene* m_scene;

	// Selection

	ueBool m_isOverField;
	ueVec3 m_overPos;
	Point m_overPoint;
};
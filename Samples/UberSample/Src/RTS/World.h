#pragma once

#include "Soldier.h"
#include "Field.h"
#include "PathFinder.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxAnimation.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"

class Terrain;

class World
{
public:
	void Init(ueAllocator* allocator);
	void Deinit();

	Field* FindNearestEmptyField(Point p);
	void AddUnit(u32 soldierCount, Point pos, ueColor32 color);

	void Update(f32 dt);
	void Draw(glCtx* ctx);

	void ResetForPathfinding();

	// Coordinate helpers

	UE_INLINE u32 GetDim() const { return m_dim; }

	UE_INLINE Field* GetFieldAt(Point p) const
	{
		return IsValidCoord(p) ? &m_fields[p.x * m_dim + p.y] : NULL;
	}

	UE_INLINE Field* GetEmptyFieldAt(Point p) const
	{
		return IsEmpty(p) ? &m_fields[p.x * m_dim + p.y] : NULL;
	}

	UE_INLINE ueBool IsValidCoord(Point p) const
	{
		return
			0 <= p.x && p.x < (s32) m_dim &&
			0 <= p.y && p.y < (s32) m_dim;
	}

	UE_INLINE ueBool IsEmpty(Point p) const
	{
		return
			IsValidCoord(p) &&
			m_fields[p.x * m_dim + p.y].IsEmpty();
	}

	UE_INLINE void Occupy(Point p, Entity* entity)
	{
		UE_ASSERT(IsEmpty(p));
		m_fields[p.x * m_dim + p.y].m_entity = entity;
	}

	void DrawModelInstance(glCtx* ctx, gxModelInstance* modelInstance);
private:

	ueAllocator* m_allocator;

	u32 m_dim;
	f32 m_cellSize;
	Field* m_fields;

	ueList<Soldier> m_soldiers;

	gxProgram m_soldierProgram;
	gxProgram m_soldierSkinnedProgram;
	ueResourceHandle<gxTexture> m_soldierTexture;
	ueResourceHandle<gxAnimation> m_soldierAnimation;
	ueResourceHandle<gxModel> m_soldierModel;

	ueBool m_isSelecting;
	ueVec2 m_selectionStart;
	ueVec2 m_selectionEnd;
};
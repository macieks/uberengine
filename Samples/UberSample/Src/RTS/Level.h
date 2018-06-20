#pragma once

#include "Terrain.h"
#include "World.h"

class Level
{
public:
	void Init(ueAllocator* allocator);
	void Deinit();

	void Draw(glCtx* ctx);
	void Update(f32 dt);

private:
	ueAllocator* m_allocator;

	Terrain m_terrain;
	World m_world;
	PathFinder m_pathFinder;
};
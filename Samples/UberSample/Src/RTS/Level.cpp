#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShader.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCamera.h"
#include "Level.h"
#include "Sample_RTS.h"

void Level::Init(ueAllocator* allocator)
{
	// Terrain

	g_terrain = &m_terrain;
	m_terrain.Init(allocator, 6, 2.0f);
	m_terrain.GenerateRandomTerrain(2);
	m_terrain.CreateRenderData();
	m_terrain.CreateCollisionData();

	// World

	g_world = &m_world;
	m_world.Init(allocator);

	// Path finder

	g_pathFinder = &m_pathFinder;
	m_pathFinder.Init(allocator);

	// Units
#if 0
	m_world.AddUnit(10, Point(3, 3), ueColor32::Red);
	m_world.AddUnit(3, Point(10, 3), ueColor32::Blue);
	m_world.AddUnit(4, Point(3, 10), ueColor32::Green);
#else
	m_world.AddUnit(1, Point(3, 3), ueColor32::Red);
#endif
}

void Level::Deinit()
{
	g_world = NULL;
	m_world.Deinit();

	g_pathFinder = NULL;
	m_pathFinder.Deinit();

	g_terrain = NULL;
	m_terrain.DestroyCollisionData();
	m_terrain.DestroyRenderData();
	m_terrain.Deinit();
}

void Level::Draw(glCtx* ctx)
{
	m_terrain.Draw(ctx);
	m_world.Draw(ctx);
}

void Level::Update(f32 dt)
{
	m_terrain.UpdateOverField();
	m_world.Update(dt);
}
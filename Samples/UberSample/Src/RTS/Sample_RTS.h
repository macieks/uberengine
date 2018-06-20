#pragma once

#include "Sample.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxShader.h"
#include "GraphicsExt/gxTexture.h"
#include "Input/inSys.h"
#include "Level.h"

struct glCtx;
struct glVertexBuffer;
struct glIndexBuffer;
struct glStreamFormat;

/**
 *	Demonstrates various engine features.
 */
class ueSample_RTS : public ueSample
{
public:
	ueBool Init();
	void Deinit();
	void DoFrame(f32 dt);

private:
	void Update(f32 dt);
	void Draw();

	Level m_level;

	gxCamera m_camera;

	inConsumerId m_inputConsumerId;
};

// Global objects
extern Terrain* g_terrain;
extern World* g_world;
extern PathFinder* g_pathFinder;
extern gxCamera* g_camera;
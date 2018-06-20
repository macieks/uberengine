#include "SampleApp.h"
#include "Sample_RTS.h"
#include "Cursor.h"
#include "Graphics/glLib.h"
#include "Physics/phLib.h"

UE_DECLARE_SAMPLE(ueSample_RTS, "RTS")

Terrain* g_terrain = NULL;
World* g_world = NULL;
PathFinder* g_pathFinder = NULL;
gxCamera* g_camera = NULL;

ueBool ueSample_RTS::Init()
{
	// Physics engine

	phStartupParams phParams;
	phParams.m_allocator = g_app.GetFreqAllocator();
#if defined(PH_PHYSX)
	phParams.m_cookingBufferSize = 1 << 20;
#endif
	phLib_Startup(&phParams);

	// Level

	m_level.Init(g_app.GetFreqAllocator());

	// Camera

	g_camera = &m_camera;

	const ueVec3 eyePos(10, 10, 10);
	m_camera.SetLookAt(eyePos, ueVec3::Zero);

	// Cursor

	Cursor::Startup();

	// Input

	m_inputConsumerId = inSys_RegisterConsumer("RTS sample", 0.0f);

	return UE_TRUE;
}

void ueSample_RTS::Deinit()
{
	inSys_UnregisterConsumer(m_inputConsumerId);

	Cursor::Shutdown();

	g_camera = NULL;

	m_level.Deinit();

	phLib_Shutdown();
}

void ueSample_RTS::DoFrame(f32 dt)
{
	Update(dt);
	Draw();
}

void ueSample_RTS::Update(f32 dt)
{
	if (inSys_IsConsumerActive(m_inputConsumerId))
	{
		gxCamera::RTSCameraParams rtsCameraParams;
		rtsCameraParams.m_minPitch = UE_PI * 0.25f;
		m_camera.UpdateRTSCamera(dt, rtsCameraParams);

		Cursor::Update(dt);
	}

	m_level.Update(dt);
}

void ueSample_RTS::Draw()
{
	glCtx* ctx = g_app.BeginDrawing();
	if (!ctx)
		return;

	m_level.Draw(ctx);
	g_app.DrawAppOverlay();
	g_app.EndDrawing();
}
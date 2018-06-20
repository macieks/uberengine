#include "Input/inSys.h"
#include "GraphicsExt/gxCamera.h"
#include "Cursor.h"
#include "Sample_RTS.h"
#include "SampleApp.h"

ueVec2 Cursor::m_pos;
ueVec2 Cursor::m_screenPos;
ueVec2 Cursor::m_clipPos;
ueVec3 Cursor::m_worldPos;

f32 Cursor::m_speed = 1.0f;

void Cursor::Startup()
{
	m_pos.Set(0.5f, 0.5f);
	UpdatePos();
}

void Cursor::Shutdown()
{
}

void Cursor::Update(f32 dt)
{
	// Mouse

	s32 mx, my;
	inMouse_GetVector(mx, my);

	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	m_pos += ueVec2((f32) mx / params->m_width, (f32) my / params->m_height);

	// Gamepad

	// TODO

	// Recalc clip & screen positions

	UpdatePos();
}

void Cursor::UpdatePos()
{
	const glDeviceStartupParams* params = glDevice_GetStartupParams();
	const u32 screenWidth = params->m_width;
	const u32 screenHeight = params->m_height;

	m_screenPos.Set((f32) screenWidth, (f32) screenHeight);
	m_screenPos *= m_pos;

	m_clipPos = m_pos * 2.0f + 1.0f;

	const ueVec3 cursorProjPos(m_clipPos[0], -m_clipPos[1], 0.0f);
	g_camera->GetViewProjI().TransformCoord(m_worldPos, cursorProjPos);
}
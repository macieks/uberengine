#pragma once

#include "Base/ueMath.h"

class Cursor
{
public:
	static void Startup();
	static void Shutdown();

	static void Update(f32 dt);

	UE_INLINE static void Get01Pos(ueVec2& out) { out = m_pos; }
	UE_INLINE static void GetScreenPos(ueVec2& out) { out = m_screenPos; }
	UE_INLINE static void GetClipPos(ueVec2& out) { out = m_clipPos; }
	UE_INLINE static void GetWorldPos(ueVec3& out) { out = m_worldPos; }

	// Conversion

	UE_INLINE static void Convert01ToClipPos(ueVec2& out, const ueVec2& pos)
	{
		out = pos * 2.0f - 1.0f;
	}
	UE_INLINE static void ConvertClipTo01Pos(ueVec2& out, const ueVec2& clipPos)
	{
		out = clipPos * 0.5f + 0.5f;
	}

private:
	static void UpdatePos();

	static ueVec2 m_pos;
	static ueVec2 m_screenPos;
	static ueVec2 m_clipPos;
	static ueVec3 m_worldPos;

	static f32 m_speed;
};
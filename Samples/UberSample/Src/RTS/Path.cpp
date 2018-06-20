#include "Path.h"
#include "World.h"
#include "Sample_RTS.h"
#include "GraphicsExt/gxShapeDraw.h"

void Path::CalcSmoothedPos(ueVec2& out, u32 pointIndex, f32 pointProgress)
{
	ueVec2 P0, P1, P2, P3;

	if (pointIndex == 0)
		P0 = m_curveStartPoint;
	else
		m_points[pointIndex - 1].AsVec2(P0);
	m_points[pointIndex].AsVec2(P1);
	m_points[pointIndex + 1].AsVec2(P2);
	if (pointIndex == m_numPoints - 1)
		P3 = m_curveEndPoint;
	else
		m_points[pointIndex + 1].AsVec2(P3);

	return ueVec2::CatmullRomInterpolate(out, P0, P1, P2, P3, pointProgress);
}

void Path::Draw(u32 subdivision)
{
	UE_PROF_SCOPE("Path::Draw");

	gxShape_Line line;
	line.m_color = ueColor32::Green;
	
	if (subdivision == 1)
		for (u32 i = 1; i < m_numPoints; i++)
		{
			g_terrain->ToWorldPos(line.m_a, m_points[i - 1]);
			g_terrain->ToWorldPos(line.m_b, m_points[i]);
			gxShapeDraw_DrawLine(line);
		}
	else
	{
		g_terrain->ToWorldPos(line.m_b, m_points[0]);
		for (u32 i = 1; i < m_numPoints; i++)
		{
			for (u32 j = 0; j < subdivision; j++)
			{
				const f32 t = (f32) (j + 1) / (f32) subdivision;

				ueVec2 b;
				CalcSmoothedPos(b, i - 1, t);

				line.m_a = line.m_b;
				g_terrain->ToWorldPos(line.m_b, b);
				gxShapeDraw_DrawLine(line);
			}
		}
	}
}
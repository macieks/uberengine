#pragma once

#include "Base/ueMath.h"

struct Point;

struct Path
{
	u32 m_numPoints;
	Point* m_points;
	ueVec2 m_curveStartPoint;
	ueVec2 m_curveEndPoint;

	void CalcSmoothedPos(ueVec2& out, u32 pointIndex, f32 pointProgress);
	void Draw(u32 subdivision = 1);
};
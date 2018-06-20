#pragma once

#include "Base/ueMath.h"

struct Point
{
	s16 x, y;

	Point() {}
	Point(s16 _x, s16 _y) : x(_x), y(_y) {}

	UE_INLINE Point operator + (const Point& other) const { return Point(x + other.x, y + other.y); }
	UE_INLINE Point operator - (const Point& other) const { return Point(x - other.x, y - other.y); }

	UE_INLINE void AsVec2(ueVec2& out) const { out.Set((f32) x, (f32) y); }

	static UE_INLINE f32 Dist(Point a, Point b)
	{
		return ueSqrt((f32) ueSqr(a.x - b.x) + (f32) ueSqr(a.y - b.y));
	}
};
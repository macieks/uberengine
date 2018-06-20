#ifndef UE_SHAPES_H
#define UE_SHAPES_H

#include "Base/ueMath.h"

//! Shape intersection result
enum usShapeIntersectionResult
{
	usShapeIntersectionResult_Outside = 0,	//!< Outside of the queried shape
	usShapeIntersectionResult_Intersects,	//!< Intersects queried shape, but is not contained inside
	usShapeIntersectionResult_Inside,		//!< Inside of the queried shape

	usShapeIntersectionResult_MAX
};

//!	Rectangle
struct ueRect
{
	f32 m_left;		//!< Left
	f32 m_top;		//!< Top
	f32 m_right;	//!< Right
	f32 m_bottom;	//!< Bottom

	UE_INLINE ueRect() {}
	UE_INLINE ueRect(f32 left, f32 top, f32 right, f32 bottom) { m_left = left; m_top = top; m_right = right; m_bottom = bottom; }
	UE_INLINE ueRect(const ueVec2& leftTop, const ueVec2& rightBottom) { Set(leftTop, rightBottom); }
	UE_INLINE void Reset() { m_left = F32_MAX; m_top = F32_MAX; m_right = F32_MIN; m_bottom = F32_MIN; }

	UE_INLINE void Set(f32 left, f32 top, f32 right, f32 bottom) { m_left = left; m_top = top; m_right = right; m_bottom = bottom; }
	UE_INLINE void Set(const ueVec2& leftTop, const ueVec2& rightBottom) { m_left = leftTop[0]; m_top = leftTop[1]; m_right = rightBottom[0]; m_bottom = rightBottom[1]; }
	UE_INLINE void SetRightBottom(const ueVec2& rightBottom) { m_right = rightBottom[0]; m_bottom = rightBottom[1]; }
	UE_INLINE void SetLeftTopAndSize(f32 left, f32 top, f32 width, f32 height) { m_left = left; m_top = top; m_right = left + width; m_bottom = top + height; }
	UE_INLINE void SetCenterAndSize(f32 centerX, f32 centerY, f32 width, f32 height) { m_left = centerX - width * 0.5f; m_top = centerY - height * 0.5f; m_right = centerX + width * 0.5f; m_bottom = centerY + height * 0.5f; }
	UE_INLINE void SetCenterAndSize(const ueVec2& center, const ueVec2& size) { m_left = center[0] - size[0] * 0.5f; m_top = center[1] - size[1] * 0.5f; m_right = center[0] + size[0] * 0.5f; m_bottom = center[1] + size[1] * 0.5f; }
	UE_INLINE const ueVec2& GetLeftTop() const { return *(const ueVec2*)(const void*) &m_left; }
	UE_INLINE ueVec2 GetRightTop() const { return ueVec2(m_right, m_top);  }
	UE_INLINE const ueVec2& GetRightBottom() const { return *(const ueVec2*)(const void*) &m_right; }
	UE_INLINE ueVec2 GetLeftBottom() const { return ueVec2(m_left, m_bottom);  }

	UE_INLINE void Extend(f32 x, f32 y)
	{
		m_left = ueMin(m_left, x);
		m_top = ueMin(m_top, y);
		m_right = ueMax(x, m_right);
		m_bottom = ueMax(y, m_bottom);
	}
	UE_INLINE void Extend(const ueVec2& xy) { Extend(xy[0], xy[1]); }
	UE_INLINE void Extend(const ueRect& other) { Extend(other.m_left, other.m_top); Extend(m_right, m_bottom); }

	UE_INLINE f32 GetWidth() const { return m_right - m_left; }
	UE_INLINE void SetWidth(f32 width) { m_right = m_left + width; }
	UE_INLINE f32 GetHeight() const { return m_bottom - m_top; }
	UE_INLINE void SetHeight(f32 height) { m_bottom = m_top + height; }
	UE_INLINE ueVec2 GetDims() const { return ueVec2(GetWidth(), GetHeight()); }
	UE_INLINE void SetDims(const ueVec2& dims) { m_right = m_left + dims[0]; m_bottom = m_top + dims[1]; }
	UE_INLINE void SetDims(f32 dimX, f32 dimY) { m_right = m_left + dimX; m_bottom = m_top + dimY; }
	UE_INLINE ueBool IsNonEmpty() const { return m_left < m_right && m_top < m_bottom; }
	UE_INLINE void Translate(f32 x, f32 y) { Translate(ueVec2(x, y)); }
	UE_INLINE void Translate(const ueVec2& offset)
	{
		m_left += offset[0]; m_top += offset[1];
		m_right += offset[0]; m_bottom += offset[1];
	}

	UE_INLINE void SetCenterX(f32 centerX)
	{
		const f32 offset = centerX - CenterX();
		m_left += offset;
		m_right += offset;
	}
	UE_INLINE void SetCenterY(f32 centerY)
	{
		const f32 offset = centerY - CenterY();
		m_top += offset;
		m_bottom += offset;
	}

	UE_INLINE f32 CenterX() const { return (m_left + m_right) * 0.5f; }
	UE_INLINE f32 CenterY() const { return (m_top + m_bottom) * 0.5f; }
	UE_INLINE void Center(ueVec2& out) const { out.Set(CenterX(), CenterY()); }

	UE_INLINE void ScaleCentered(const ueVec2& scale)
	{
		const f32 halfWidth = (m_right - m_left) * 0.5f;
		const f32 halfHeight = (m_bottom - m_top) * 0.5f;

		const f32 invScaleX = scale[0] - 1.0f;
		const f32 invScaleY = scale[1] - 1.0f;

		const f32 offsetX = halfWidth * invScaleX;
		const f32 offsetY = halfHeight * invScaleY;

		m_left -= offsetX;
		m_top -= offsetY;
		m_right += offsetX;
		m_bottom += offsetY;
	}
	UE_INLINE void ScaleCentered(f32 scale)
	{
		ScaleCentered(ueVec2(scale, scale));
	}

	UE_INLINE ueBool Intersect(f32 x, f32 y) const
	{
		return m_left <= x && m_top <= y && x <= m_right && y <= m_bottom;
	}

	UE_INLINE ueBool Intersect(const ueVec2& point) const
	{
		return Intersect(point[0], point[1]);
	}

	UE_INLINE static ueRect Lerp(const ueRect& a, const ueRect& b, f32 scale)
	{
		return ueRect(
			a.m_left + (b.m_left - a.m_left) * scale,
			a.m_top + (b.m_top - a.m_top) * scale,
			a.m_right + (b.m_right - a.m_right) * scale,
			a.m_bottom + (b.m_bottom - a.m_bottom) * scale);
	}

	UE_INLINE ueRect operator + (const ueVec2& translation) const { ueRect r = *this; r.Translate(translation); return r; }
	UE_INLINE ueBool operator == (const ueRect& other) const { return m_left == other.m_left && m_top == other.m_top && m_right == other.m_right && m_bottom == other.m_bottom; }
	UE_INLINE ueBool operator != (const ueRect& other) const { return m_left != other.m_left || m_top != other.m_top || m_right != other.m_right || m_bottom != other.m_bottom; }
};

//! Integer coordinate based rectangle
struct ueRectI
{
	s32 m_left;		//!< Left
	s32 m_top;		//!< Top
	s32 m_width;	//!< Width
	s32 m_height;	//!< Height

	UE_INLINE ueRectI() : m_left(0), m_top(0), m_width(0), m_height(0) {}
	UE_INLINE ueRectI(s32 left, s32 top, s32 width, s32 height) : m_left(left), m_top(top), m_width(width), m_height(height) {}
	UE_INLINE void ToRectF(ueRect& rectF) { rectF.m_left = (f32) m_left; rectF.m_top = (f32) m_top; rectF.m_right = (f32) (m_left + m_width); rectF.m_bottom = (f32) (m_top + m_height); }
	UE_INLINE void FromRectF(const ueRect& rectF) { m_left = (s32) rectF.m_left; m_top = (s32) rectF.m_top; m_width = (s32) (rectF.m_right - rectF.m_left); m_height = (s32) (rectF.m_bottom - rectF.m_top); }
	UE_INLINE s32 CenterX() const { return m_left + (m_width >> 1); }
	UE_INLINE s32 CenterY() const { return m_top + (m_height >> 1); }
	UE_INLINE ueBool Intersect(s32 x, s32 y) const { return m_left <= x && x <= m_left + m_width && m_top <= y && y <= m_top + m_height; }

	UE_INLINE s32 GetRight() const { return m_left + m_width; }
	UE_INLINE s32 GetBottom() const { return m_top + m_height; }
};

//!	Sphere
struct ueSphere
{
	ueVec3 m_center;
	f32 m_radius;

	ueSphere();
	ueSphere(const ueVec3& center, f32 radius);

	ueBool Intersect(const ueVec3& point) const;
	ueBool Intersect(const ueSphere& sphere) const;

	usShapeIntersectionResult IntersectFull(const ueSphere& sphere) const;

	void Transform(const ueMat44& transformation);
};

//!	Axis aligned box
struct ueBox
{
	ueVec3 m_min;
	ueVec3 m_max;

	ueBox();
	ueBox(const ueVec3& min, const ueVec3& max);
	ueBox(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ);

	void Set(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ);

	ueBool operator == (const ueBox& other) const;
	ueBool operator != (const ueBox& other) const;
	ueBool Contains(const ueBox& other) const;

	void Reset();
	ueBool IsValid() const;

	void GetCenter(ueVec3& out) const;
	void GetCorner(ueVec3& out, ueBool minX, ueBool minY, ueBool minZ) const;
	void GetExtents(ueVec3& out) const;

	void Extend(const ueVec3& point);
	void Extend(const ueBox& box);

	ueBool Intersect(const ueVec3& point) const;
	ueBool Intersect(const ueSphere& sphere) const;
	ueBool Intersect(const ueBox& box) const;

	usShapeIntersectionResult IntersectFull(const ueSphere& sphere) const;
	usShapeIntersectionResult IntersectFull(const ueBox& box) const;

	void CalcNearCorner(ueVec3& out, const uePlane& plane) const;
	void CalcFarCorner(ueVec3& out, const uePlane& plane) const;

	void Scale(f32 scale);
	void Scale(f32 x, f32 y, f32 z);
	void Translate(f32 x, f32 y, f32 z);
	void Translate(const ueVec3& translation);
	void Transform(const ueMat44& transformation);
};

//! Convex shape
template <u32 MAX_PLANES>
struct ueConvex
{
	uePlane m_planes[MAX_PLANES];
	u32 m_numPlanes;

	ueConvex(u32 initialNumPlanes = 0);

	void Build(const ueMat44& viewProj);
	void Build(const ueMat44& view, const ueMat44& proj);
	void Build(const ueBox& box);

	void Reset();
	void AddPlane(const uePlane& plane);

	void SetPlane(u32 index, const uePlane& plane);

	u32 GetNumPlanes() const;
	void GetPlane(uePlane& out, u32 index) const;

	void Transform(const ueMat44& transformation);

	// Intersection tests

	ueBool Intersect(const ueVec3& point) const;
	ueBool Intersect(const ueSphere& sphere) const;
	ueBool Intersect(const ueBox& box) const;

	usShapeIntersectionResult IntersectFull(const ueSphere& sphere) const;
	usShapeIntersectionResult IntersectFull(const ueBox& box) const;
	usShapeIntersectionResult IntersectFull(const ueConvex<MAX_PLANES>& convex) const;

	// Intersection tests with particular planes

	ueBool Intersect(const ueBox& box, u32 inPlaneFlags) const;
	ueBool Intersect(const ueBox& box, u32 inPlaneFlags, u32& outPlaneFlags) const;
};

enum ueFrustumPlane
{
	ueFrustumPlane_Left = 0,
	ueFrustumPlane_Right,
	ueFrustumPlane_Top,
	ueFrustumPlane_Bottom,
	ueFrustumPlane_Near,
	ueFrustumPlane_Far,

	ueFrustumPlane_MAX
};

//!	Standard frustum with 6 culling planes
struct ueFrustum : ueConvex<ueFrustumPlane_MAX>
{
	static const u32 PlaneFlags = 1 | 2 | 4 | 8 | 16 | 32;

	UE_INLINE ueFrustum() : ueConvex<ueFrustumPlane_MAX>(ueFrustumPlane_MAX) {}
};

#include "Base/ueShapes_Inline.h"

#endif // UE_SHAPES_H

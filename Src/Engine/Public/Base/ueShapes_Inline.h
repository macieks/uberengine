#pragma once

/*-------------------- ueSphere implementation --------------------------*/

UE_INLINE ueSphere::ueSphere() {}
UE_INLINE ueSphere::ueSphere(const ueVec3& center, f32 radius) :
	m_center(center),
	m_radius(radius)
{}

UE_INLINE ueBool ueSphere::Intersect(const ueVec3& point) const
{
	ueVec3 toCenterVec = point;
	toCenterVec -= m_center;
	return toCenterVec.LenSq() <= ueSqr(m_radius);
}

UE_INLINE ueBool ueSphere::Intersect(const ueSphere& sphere) const
{
	ueVec3 centersDist = sphere.m_center;
	centersDist -= m_center;
	return centersDist.LenSq() < ueSqr(sphere.m_radius + m_radius);
}

UE_INLINE usShapeIntersectionResult ueSphere::IntersectFull(const ueSphere& sphere) const
{
	UE_NOT_IMPLEMENTED();
	return usShapeIntersectionResult_Outside;
}

UE_INLINE void ueSphere::Transform(const ueMat44& transformation)
{
	ueVec3 sphereTop(m_center);
	sphereTop[1] += m_radius;

	transformation.TransformCoord(m_center);
	transformation.TransformCoord(sphereTop);

	ueVec3 sphereTopToCenterVec = m_center;
	sphereTopToCenterVec -= sphereTop;

	m_radius = sphereTopToCenterVec.Len();
}

/*-------------------- ueBox implementation --------------------------*/

UE_INLINE ueBox::ueBox() {}
UE_INLINE ueBox::ueBox(const ueVec3& min, const ueVec3& max) :
	m_min(min),
	m_max(max)
{}
UE_INLINE ueBox::ueBox(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ) :
	m_min(minX, minY, minZ),
	m_max(maxX, maxY, maxZ)
{}

UE_INLINE void ueBox::Set(f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ)
{
	m_min.Set(minX, minY, minZ);
	m_max.Set(maxX, maxY, maxZ);
}

UE_INLINE ueBool ueBox::operator == (const ueBox& other) const
{
	return m_min == other.m_min && m_max == other.m_max;
}

UE_INLINE ueBool ueBox::operator != (const ueBox& other) const
{
	return m_min != other.m_min || m_max != other.m_max;
}

UE_INLINE ueBool ueBox::Contains(const ueBox& other) const
{
	return m_min <= other.m_min && other.m_max <= m_max;
}

UE_INLINE void ueBox::Reset()
{
	m_min.Set(F32_MAX, F32_MAX, F32_MAX);
	m_max.Set(F32_MIN, F32_MIN, F32_MIN);
}

UE_INLINE ueBool ueBox::IsValid() const
{
	return m_min <= m_max;
}

UE_INLINE void ueBox::GetCenter(ueVec3& out) const
{
	out = m_min;
	out += m_max;
	out *= 0.5f;
}

UE_INLINE void ueBox::GetCorner(ueVec3& out, ueBool minX, ueBool minY, ueBool minZ) const
{
	out.Set(
		(minX ? m_min : m_max)[0],
		(minY ? m_min : m_max)[1],
		(minZ ? m_min : m_max)[2]);
}

UE_INLINE void ueBox::GetExtents(ueVec3& out) const
{
	out = m_max;
	out -= m_min;
}

UE_INLINE void ueBox::Extend(const ueVec3& point)
{
	ueVec3::Min(m_min, m_min, point);
	ueVec3::Max(m_max, m_max, point);
}

UE_INLINE void ueBox::Extend(const ueBox& box)
{
	ueVec3::Min(m_min, m_min, box.m_min);
	ueVec3::Max(m_max, m_max, box.m_max);
}

UE_INLINE ueBool ueBox::Intersect(const ueVec3& point) const
{
	return m_min <= point && point <= m_max;
}

UE_INLINE ueBool ueBox::Intersect(const ueSphere& sphere) const
{
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

UE_INLINE ueBool ueBox::Intersect(const ueBox& box) const
{
	return box.m_min <= m_max && m_min <= box.m_max;
}

UE_INLINE usShapeIntersectionResult ueBox::IntersectFull(const ueSphere& sphere) const
{
	UE_NOT_IMPLEMENTED();
	return usShapeIntersectionResult_Outside;
}

UE_INLINE usShapeIntersectionResult ueBox::IntersectFull(const ueBox& box) const
{
	UE_NOT_IMPLEMENTED();
	return usShapeIntersectionResult_Outside;
}

UE_INLINE void ueBox::CalcNearCorner(ueVec3& out, const uePlane& plane) const
{
	out.Set(
		(plane[0] < 0 ? m_min : m_max)[0],
		(plane[1] < 0 ? m_min : m_max)[1],
		(plane[2] < 0 ? m_min : m_max)[2]);
}

UE_INLINE void ueBox::CalcFarCorner(ueVec3& out, const uePlane& plane) const
{
	out.Set(
		(plane[0] >= 0 ? m_min : m_max)[0],
		(plane[1] >= 0 ? m_min : m_max)[1],
		(plane[2] >= 0 ? m_min : m_max)[2]);
}

UE_INLINE void ueBox::Scale(f32 scale)
{
	m_min *= scale;
	m_max *= scale;
}

UE_INLINE void ueBox::Scale(f32 x, f32 y, f32 z)
{
	const ueVec3 scale(x, y, z);

	m_min *= scale;
	m_max *= scale;
}

UE_INLINE void ueBox::Translate(f32 x, f32 y, f32 z)
{
	Translate(ueVec3(x, y, z));
}

UE_INLINE void ueBox::Translate(const ueVec3& translation)
{
	m_min += translation;
	m_max += translation;
}

UE_INLINE void ueBox::Transform(const ueMat44& transformation)
{
	ueBox box = *this;

	Reset();

	ueVec3 corner;
	for (u32 x = 0; x < 2; x++)
	{
		corner[0] = x ? box.m_min[0]: box.m_max[0];
		for (u32 y = 0; y < 2; y++)
		{
			corner[1] = y ? box.m_min[1]: box.m_max[1];
			for (u32 z = 0; z < 2; z++)
			{
				corner[2] = z ? box.m_min[2]: box.m_max[2];
				transformation.TransformCoord(corner);
				Extend(corner);
			}
		}
	}
}

/*-------------------- ueConvex implementation --------------------------*/

template <u32 MAX_PLANES>
UE_INLINE ueConvex<MAX_PLANES>::ueConvex(u32 initialNumPlanes) :
	m_numPlanes(initialNumPlanes)
{}

template <u32 MAX_PLANES>
UE_INLINE void ueConvex<MAX_PLANES>::Build(const ueMat44& view, const ueMat44& proj)
{
	ueMat44 viewProj;
	ueMat44::Mul(viewProj, view, proj);
	Build(viewProj);
}

template <u32 MAX_PLANES>
UE_INLINE void ueConvex<MAX_PLANES>::Reset()
{
	m_numPlanes = 0;
}

template <u32 MAX_PLANES>
UE_INLINE void ueConvex<MAX_PLANES>::AddPlane(const uePlane& plane)
{
	UE_ASSERT(m_numPlanes < MAX_PLANES);
	m_numPlanes++;
}

template <u32 MAX_PLANES>
UE_INLINE void ueConvex<MAX_PLANES>::SetPlane(u32 index, const uePlane& plane)
{
	UE_ASSERT(index < MAX_PLANES);
	m_planes[index] = plane;
}

template <u32 MAX_PLANES>
UE_INLINE u32 ueConvex<MAX_PLANES>::GetNumPlanes() const
{
	return m_numPlanes;
}

template <u32 MAX_PLANES>
UE_INLINE void ueConvex<MAX_PLANES>::GetPlane(uePlane& out, u32 index) const
{
	UE_ASSERT(index < MAX_PLANES);
	out = m_planes[index];
}

template <u32 MAX_PLANES>
UE_INLINE ueBool ueConvex<MAX_PLANES>::Intersect(const ueVec3& point) const
{
	for (u32 i = 0; i < m_numPlanes; i++)
		if (m_planes[i].DotCoord(point) < 0.0f)
			return UE_FALSE;
	return UE_TRUE;
}

template <u32 MAX_PLANES>
UE_INLINE ueBool ueConvex<MAX_PLANES>::Intersect(const ueSphere& sphere) const
{
	const f32 radiusNeg = -sphere.m_radius;
	for (u32 i = 0; i < m_numPlanes; i++)
	{
		const f32 dot = m_planes[i].DotCoord(sphere.m_center);
		if (dot < radiusNeg)
			return UE_FALSE;
	}
	return UE_TRUE;
}

template <u32 MAX_PLANES>
UE_INLINE ueBool ueConvex<MAX_PLANES>::Intersect(const ueBox& box) const
{
	ueVec3 boxCorner;
	for (u32 i = 0; i < m_numPlanes; i++)
	{
		box.CalcNearCorner(boxCorner, m_planes[i]);
		if (m_planes[i].DotCoord(boxCorner) < 0.0f)
			return UE_FALSE;
	}
	return UE_TRUE;
}

template <u32 MAX_PLANES>
UE_INLINE usShapeIntersectionResult ueConvex<MAX_PLANES>::IntersectFull(const ueSphere& sphere) const
{
	const f32 radius = sphere.m_radius;
	const f32 radiusNeg = -sphere.m_radius;
	ueBool intersects = UE_FALSE;
	for (u32 i = 0; i < m_numPlanes; i++)
	{
		const f32 dot = m_planes[i].DotCoord(sphere.m_center);
		if (dot < radiusNeg)
			return usShapeIntersectionResult_Outside;
		intersects |= (dot < radius);
	}
	return intersects ? usShapeIntersectionResult_Intersects : usShapeIntersectionResult_Inside;
}

template <u32 MAX_PLANES>
UE_INLINE usShapeIntersectionResult ueConvex<MAX_PLANES>::IntersectFull(const ueBox& box) const
{
	ueVec3 boxCorner;
	ueBool intersects = UE_FALSE;
	for (u32 i = 0; i < m_numPlanes; i++)
	{
		box.CalcNearCorner(boxCorner, m_planes[i]);
		if (m_planes[i].DotCoord(boxCorner) < 0.0f)
			return usShapeIntersectionResult_Outside;

		if (!intersects)
		{
			boxCorner = box.CalcFarCorner(m_planes[i]);
			intersects = m_planes[i].DotCoord(boxCorner) < 0.0f;
		}
	}
	return intersects ? usShapeIntersectionResult_Intersects : usShapeIntersectionResult_Inside;
}

template <u32 MAX_PLANES>
UE_INLINE usShapeIntersectionResult ueConvex<MAX_PLANES>::IntersectFull(const ueConvex<MAX_PLANES>& convex) const
{
	UE_NOT_IMPLEMENTED();
	return usShapeIntersectionResult_Inside;
}

template <u32 MAX_PLANES>
UE_INLINE ueBool ueConvex<MAX_PLANES>::Intersect(const ueBox& box, u32 inPlaneFlags) const
{
	if (inPlaneFlags == 0)
		return UE_TRUE;

	ueVec3 boxCorner;
	for (u32 i = 0; i < m_numPlanes; i++)
		if (inPlaneFlags & UE_POW2(i))
		{
			box.CalcNearCorner(boxCorner, m_planes[i]);
			if (m_planes[i].DotCoord(boxCorner) < 0.0f)
				return UE_FALSE;
		}
	return UE_TRUE;
}

template <u32 MAX_PLANES>
UE_INLINE ueBool ueConvex<MAX_PLANES>::Intersect(const ueBox& box, u32 inPlaneFlags, u32& outPlaneFlags) const
{
	outPlaneFlags = inPlaneFlags;
	if (inPlaneFlags == 0)
		return UE_TRUE;

	ueVec3 boxCorner;
	for (u32 i = 0; i < m_numPlanes; i++)
		if (inPlaneFlags & UE_POW2(i))
		{
			box.CalcNearCorner(boxCorner, m_planes[i]);
			if (m_planes[i].DotCoord(boxCorner) < 0.0f)
				return UE_FALSE;

			box.CalcFarCorner(boxCorner, m_planes[i]);
			if (m_planes[i].DotCoord(boxCorner) >= 0.0f)
				outPlaneFlags &= ~UE_POW2(i);
		}
	return UE_TRUE;
}

template <u32 MAX_PLANES>
void ueConvex<MAX_PLANES>::Build(const ueMat44& vp)
{
	m_numPlanes = ueFrustumPlane_MAX;

	// Construct all 6 frustum planes

	m_planes[ueFrustumPlane_Left].Set(
		vp(0, 3) - vp(0, 0),
		vp(1, 3) - vp(1, 0),
		vp(2, 3) - vp(2, 0),
		vp(3, 3) - vp(3, 0));

	m_planes[ueFrustumPlane_Right].Set(
		vp(0, 3) + vp(0, 0),
		vp(1, 3) + vp(1, 0),
		vp(2, 3) + vp(2, 0),
		vp(3, 3) + vp(3, 0));

	m_planes[ueFrustumPlane_Top].Set(
		vp(0, 3) + vp(0, 1),
		vp(1, 3) + vp(1, 1),
		vp(2, 3) + vp(2, 1),
		vp(3, 3) + vp(3, 1));

	m_planes[ueFrustumPlane_Bottom].Set(
		vp(0, 3) - vp(0, 1),
		vp(1, 3) - vp(1, 1),
		vp(2, 3) - vp(2, 1),
		vp(3, 3) - vp(3, 1));

	m_planes[ueFrustumPlane_Near].Set(
		vp(0, 3) - vp(0, 2),
		vp(1, 3) - vp(1, 2),
		vp(2, 3) - vp(2, 2),
		vp(3, 3) - vp(3, 2));

	m_planes[ueFrustumPlane_Far].Set(
		vp(0, 3) + vp(0, 2),
		vp(1, 3) + vp(1, 2),
		vp(2, 3) + vp(2, 2),
		vp(3, 3) + vp(3, 2));

	// Normalize the planes

	for (u32 i = 0; i < m_numPlanes; i++)
		m_planes[i].Normalize();
}

template <u32 MAX_PLANES>
void ueConvex<MAX_PLANES>::Build(const ueBox& box)
{
	m_numPlanes = 6;
	m_planes[0].Set(1, 0, 0, -box.m_min[0]);
	m_planes[1].Set(-1, 0, 0, box.m_max[0]);
	m_planes[2].Set(0, 1, 0, -box.m_min[1]);
	m_planes[3].Set(0, -1, 0, box.m_max[1]);
	m_planes[4].Set(0, 0, 1, -box.m_min[2]);
	m_planes[5].Set(0, 0, -1, box.m_max[2]);
}

template <u32 MAX_PLANES>
void ueConvex<MAX_PLANES>::Transform(const ueMat44& transformation)
{
	ueMat44 inverseTransposeTransformation(transformation);
	inverseTransposeTransformation.Transpose();
	inverseTransposeTransformation.Inverse();

	for (u32 i = 0; i < m_numPlanes; i++)
	{
		inverseTransposeTransformation.TransformPlane(m_planes[i]);
		m_planes[i].Normalize();
	}
}

/*-------------------- ueFrustum implementation --------------------------*/

/*UE_INLINE usShapeIntersectionResult ueFrustum::IntersectFull(const ueFrustum& frustum) const
{
	return IntersectFull(frustum);
}*/

#ifndef GX_SHAPE_DRAW_VERTEX_H
#define GX_SHAPE_DRAW_VERTEX_H

#include "GraphicsExt/gxProgram.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Vertex with XYZ position + UV texture coordinates + RGBA color
struct gxPosColTexVertex
{
	f32 x, y, z;
	f32 u, v;
	u32 rgba;

	static gxProgram m_program;
	static glStreamFormat* m_sf;

	UE_INLINE void SetV2(const ueVec2& xy, const ueMat44* transform, const ueVec2& uv, ueColor32 color)
	{
		Set(ueVec3(xy, 0.0f), transform, uv, color);
	}

	UE_INLINE void Set(const ueVec3& _xyz, const ueMat44* transform, const ueVec2& uv, ueColor32 color)
	{
		ueVec3 xyz = _xyz;
		if (transform)
			transform->TransformCoord(xyz);
		xyz.Store3(&x);

		uv.Store2(&u);

		rgba = color.AsShaderAttr();
	}
};

//! Vertex with XYZ position + UV texture coordinates
struct gxPosTexVertex
{
	f32 x, y, z;
	f32 u, v;

	static gxProgram m_program;
	static glStreamFormat* m_sf;

	UE_INLINE void Set(f32 x, f32 y, const ueMat44* transform, const ueVec2& uv)
	{
		SetV2(ueVec2(x, y), transform, uv);
	}

	UE_INLINE void SetV2(const ueVec2& xy, const ueMat44* transform, const ueVec2& uv)
	{
		ueVec3 xyz(xy, 0.0f);
		if (transform)
			transform->TransformCoord(xyz);
		xyz.Store3(&x);

		uv.Store2(&u);
	}
};

//! Vertex with XYZ position + RGBA color
struct gxPosColVertex
{
	f32 x, y, z;
	u32 rgba;

	static gxProgram m_program;
	static glStreamFormat* m_sf;

	UE_INLINE void Set(f32 x, f32 y, f32 z, const ueMat44* transform, ueColor32 color)
	{
		Set(ueVec3(x, y, z), transform, color);
	}

	UE_INLINE void Set(const ueVec2& xy, const ueMat44* transform, ueColor32 color)
	{
		Set(ueVec3(xy, 0), transform, color);
	}

	UE_INLINE void Set(const ueVec3& _xyz, const ueMat44* transform, ueColor32 color)
	{
		ueVec3 xyz = _xyz;
		if (transform)
			transform->TransformCoord(xyz);
		xyz.Store3(&x);

		rgba = color.AsShaderAttr();
	}

	UE_INLINE void SetV2(const ueVec2& xy, const ueMat44* transform, ueColor32 color)
	{
		ueVec3 xyz(xy, 0.0f);
		if (transform)
			transform->TransformCoord(xyz);
		xyz.Store3(&x);

		rgba = color.AsShaderAttr();
	}
};

// @}

#endif // GX_SHAPE_DRAW_VERTEX_H
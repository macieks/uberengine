#include "Base/ueBase.h"
#include <Box2D.h>

#if defined(UE_MARMALADE) && 0

// Stub code

class b2DebugDraw_Default : public b2DebugDraw
{
public:
	f32 m_alpha;

	b2DebugDraw_Default() :
		m_alpha(1.0f)
	{
		SetFlags(e_shapeBit | e_jointBit);
	}

	/// Draw a closed polygon provided in CCW order.
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {}

	/// Draw a solid closed polygon provided in CCW order.
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {}

	/// Draw a circle.
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {}
	
	/// Draw a solid circle.
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {}
	
	/// Draw a line segment.
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {}

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	virtual void DrawXForm(const b2XForm& xf) {}
};

#else

#include "GraphicsExt/gxShapeDraw.h"

class b2DebugDraw_Default : public b2DebugDraw
{
public:

	f32 m_alpha;
	f32 m_scale;

	b2DebugDraw_Default() :
		m_alpha(1.0f),
		m_scale(1.0f)
	{
		SetFlags(e_shapeBit | e_jointBit);
	}

	/// Draw a closed polygon provided in CCW order.
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		gxShape_Line2D line;
		line.m_color = ueColor32::F(color.r, color.g, color.b, m_alpha);
		for (s32 i = 0; i < vertexCount; i++)
		{
			const b2Vec2& p1 = vertices[i];
			const b2Vec2& p2 = vertices[(i + 1) % vertexCount];

			line.m_a.Set(&p1.x);
			line.m_a *= m_scale;
			line.m_b.Set(&p2.x);
			line.m_b *= m_scale;
			gxShapeDraw_DrawLine(line);
		}
	}

	/// Draw a solid closed polygon provided in CCW order.
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		gxShape_Triangle2D triangle;
		triangle.m_wireFrame = UE_FALSE;
		triangle.m_color = ueColor32::F(color.r, color.g, color.b, m_alpha);
		for (s32 i = 1; i < vertexCount; i++)
		{
			const b2Vec2& p1 = vertices[0];
			const b2Vec2& p2 = vertices[i];
			const b2Vec2& p3 = vertices[(i + 1) % vertexCount];

			triangle.m_a.Set(&p1.x);
			triangle.m_a *= m_scale;
			triangle.m_b.Set(&p2.x);
			triangle.m_b *= m_scale;
			triangle.m_c.Set(&p3.x);
			triangle.m_c *= m_scale;
			gxShapeDraw_DrawTriangle(triangle);
		}
	}

	inline b2Vec2 b2Vec2RotScale(f32 angle, f32 scale) const
	{
		return b2Vec2(ueSin(angle) * scale, ueCos(angle) * scale);
	}

	static const u32 s_circleNumEdges = 13;

	#define CIRCLE_VERTEX(index) b2Vec2RotScale((f32) (index) / (f32) s_circleNumEdges * UE_2PI + rot, radius)

	/// Draw a circle.
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		const f32 rot = 0.0f;

		gxShape_Line2D line;
		line.m_color = ueColor32::F(color.r, color.g, color.b, m_alpha);
		for (u32 i = 0; i < s_circleNumEdges; i++)
		{
			const b2Vec2 p1 = center + CIRCLE_VERTEX(i);
			const b2Vec2 p2 = center + CIRCLE_VERTEX((i + 1) % s_circleNumEdges);

			line.m_a.Set(&p1.x);
			line.m_a *= m_scale;
			line.m_b.Set(&p2.x);
			line.m_b *= m_scale;
			gxShapeDraw_DrawLine(line);
		}
	}

	/// Draw a solid circle.
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
#if 0
		DrawCircle(center, radius, color);
#else
		const f32 rot = ueATan2(axis.x, axis.y);

		gxShape_Triangle2D triangle;
		triangle.m_wireFrame = UE_FALSE;
		triangle.m_color = ueColor32::F(color.r, color.g, color.b, m_alpha);
		for (u32 i = 0; i < s_circleNumEdges; i++)
		{
			const b2Vec2& p1 = center;
			const b2Vec2 p2 = center + CIRCLE_VERTEX(i);
			const b2Vec2 p3 = center + CIRCLE_VERTEX((i + 1) % s_circleNumEdges);

			triangle.m_a.Set(&p1.x);
			triangle.m_a *= m_scale;
			triangle.m_b.Set(&p2.x);
			triangle.m_b *= m_scale;
			triangle.m_c.Set(&p3.x);
			triangle.m_c *= m_scale;
			gxShapeDraw_DrawTriangle(triangle);
		}
#endif
	}
	
	/// Draw a line segment.
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		gxShape_Line2D line;
		line.m_color = ueColor32::F(color.r, color.g, color.b, m_alpha);
		line.m_a.Set(&p1.x);
		line.m_a *= m_scale;
		line.m_b.Set(&p2.x);
		line.m_b *= m_scale;
		gxShapeDraw_DrawLine(line);
	}

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	virtual void DrawXForm(const b2XForm& xf)
	{
		// TODO
	}
};

#endif

static ueAllocator* s_allocator = NULL;
static b2DebugDraw_Default s_debugDraw;

void phBox2D_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_allocator);
	s_allocator = allocator;
}

void phBox2D_Shutdown()
{
	UE_ASSERT(s_allocator);
	s_allocator = NULL;
}

b2DebugDraw* phBox2D_GetDefaultDebugDraw()
{
	UE_ASSERT(s_allocator);
	return &s_debugDraw;
}

void phBox2D_SetDebugDrawFlags(u32 flags)
{
	s_debugDraw.SetFlags(flags);
}

void phBox2D_SetDebugDrawAlpha(f32 alpha)
{
	s_debugDraw.m_alpha = alpha;
}

void phBox2D_SetDebugDrawScale(f32 scale)
{
	s_debugDraw.m_scale = scale;
}

void* b2Alloc(int32 size)
{
	UE_ASSERT(s_allocator);
	return s_allocator->Alloc(size);
}

void b2Free(void* mem)
{
	UE_ASSERT(s_allocator);
	s_allocator->Free(mem);
}
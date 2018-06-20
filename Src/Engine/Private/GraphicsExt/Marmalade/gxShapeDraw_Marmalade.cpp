#include "GL/glLib.h"
#ifndef UE_MARMALADE
	#include "GL/glCommonConstants.h"
	#include "GL/glVertexBufferFactory.h"
	#include "GL/glIndexBufferFactory.h"
	#include "GX/gxProgram.h"
#endif
#include "GX/gxShapeDraw_Vertex.h"
#include "GX/gxShapeDraw.h"

struct gxShapeDrawData
{
	ueAllocator* m_allocator;

	// Boxes

	struct Box
	{
		gxPosColVertex m_corners[8];
	};
	u32 m_numSolidBoxes;
	u32 m_numWireBoxes;
	u32 m_maxBoxes;
	Box* m_boxes;

	// Spheres

	struct Sphere
	{
		ueSphere m_sphere;
		ueColor32 m_color;
	};
	u32 m_numSolidSpheres;
	u32 m_numWireSpheres;
	u32 m_maxSpheres;
	Sphere* m_spheres;

	// Triangles

	struct Triangle
	{
		gxPosColVertex m_verts[3];
	};
	u32 m_num3DTriangles;
	u32 m_num2DTriangles;
	u32 m_maxTriangles;
	Triangle* m_triangles;

	// Lines

	struct Line
	{
		gxPosColVertex m_verts[2];
	};
	u32 m_num3DLines;
	u32 m_num2DLines;
	u32 m_maxLines;
	Line* m_lines;

	// Buffers

	CIwSVec3* m_pos;
	CIwColour* m_color;

	// Rendering

	glCtx* m_ctx;
	ueMat44 m_viewProj;
	ueRect m_2DCanvas;
	ueMat44 m_canvasTransform;

	CIwMaterial m_material;
};

static gxShapeDrawData* s_data = NULL;

void gxShapeDraw_Startup(gxShapeDrawStartupParams* params)
{
	ueAssert(!s_data);

	s_data = ueNew<gxShapeDrawData>(params->m_stackAllocator);
	ueAssert(s_data);
	s_data->m_allocator = params->m_stackAllocator;
	s_data->m_ctx = NULL;

	// Buffers

	u32 maxVerts = 0;
	maxVerts = ueMax(maxVerts, params->m_maxBoxBatch * 8);
	maxVerts = ueMax(maxVerts, params->m_maxSphereBatch * 5);
	maxVerts = ueMax(maxVerts, params->m_maxLineBatch * 2);
	maxVerts = ueMax(maxVerts, params->m_maxTriangleBatch * 3);

	s_data->m_numSolidBoxes = 0;
	s_data->m_numWireBoxes = 0;
	s_data->m_maxBoxes = params->m_maxBoxBatch;

	s_data->m_numSolidSpheres = 0;
	s_data->m_numWireSpheres = 0;
	s_data->m_maxSpheres = params->m_maxSphereBatch;

	s_data->m_num3DLines = 0;
	s_data->m_num2DLines = 0;
	s_data->m_maxLines = params->m_maxLineBatch;

	s_data->m_num3DTriangles = 0;
	s_data->m_num2DTriangles = 0;
	s_data->m_maxTriangles = params->m_maxTriangleBatch;

	u8* data = (u8*) s_data->m_allocator->Alloc(
		sizeof(gxShapeDrawData::Box) * s_data->m_maxBoxes +
		sizeof(gxShapeDrawData::Sphere) * s_data->m_maxSpheres +
		sizeof(gxShapeDrawData::Triangle) * s_data->m_maxTriangles +
		sizeof(gxShapeDrawData::Line) * s_data->m_maxLines +
		sizeof(CIwSVec3) * maxVerts +
		sizeof(CIwColour) * maxVerts);
	ueAssert(data);

	s_data->m_boxes = (gxShapeDrawData::Box*) data;
	data += sizeof(gxShapeDrawData::Box) * s_data->m_maxBoxes;

	s_data->m_spheres = (gxShapeDrawData::Sphere*) data;
	data += sizeof(gxShapeDrawData::Sphere) * s_data->m_maxSpheres;

	s_data->m_triangles = (gxShapeDrawData::Triangle*) data;
	data += sizeof(gxShapeDrawData::Triangle) * s_data->m_maxTriangles;

	s_data->m_lines = (gxShapeDrawData::Line*) data;
	data += sizeof(gxShapeDrawData::Line) * s_data->m_maxLines;

	s_data->m_pos = (CIwSVec3*) data;
	data += sizeof(CIwSVec3) * maxVerts;

	s_data->m_color = (CIwColour*) data;
	data += sizeof(CIwColour) * maxVerts;

	// Material

	s_data->m_material.SetAlphaMode(CIwMaterial::ALPHA_BLEND);
	s_data->m_material.SetDepthWriteMode(CIwMaterial::DEPTH_WRITE_DISABLED);
	s_data->m_material.SetCullMode(CIwMaterial::CULL_NONE);
}

void gxShapeDraw_Shutdown()
{
	ueAssert(s_data);

	s_data->m_allocator->Free(s_data->m_boxes);
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxShapeDraw_Begin(const gxShapeDrawBeginParams& params)
{
	ueAssert(!s_data->m_ctx);
	s_data->m_ctx = params.m_ctx;
	s_data->m_viewProj = params.m_viewProj ? *params.m_viewProj : ueMat44_Identity;
	s_data->m_2DCanvas = params.m_2DCanvas;
	s_data->m_canvasTransform = params.m_canvasTransform ? *params.m_canvasTransform : ueMat44_Identity;
}

void gxShapeDraw_End()
{
	ueAssert(s_data->m_ctx);
	gxShapeDraw_Flush();
	s_data->m_ctx = NULL;
}

void gxShapeDraw_CopyVertex(const gxPosColVertex* src, CIwSVec3* dstPos, CIwColour* dstColor, u32 numVerts)
{
	dstPos[numVerts].x = GL_TO_MARMALADE_COORD(src->x);
	dstPos[numVerts].y = GL_TO_MARMALADE_COORD(src->y);
	dstPos[numVerts].z = GL_TO_MARMALADE_COORD(src->z);

	dstColor[numVerts] = src->rgba;
}

void gxShapeDraw_CopyVertex2D(const gxPosColVertex* src, CIwSVec3* dstPos, CIwColour* dstColor, u32 numVerts)
{
	dstPos[numVerts].x = GL_TO_MARMALADE_COORD(src->x);
	dstPos[numVerts].y = GL_TO_MARMALADE_COORD(src->y);
	dstPos[numVerts].z = GL_TO_MARMALADE_COORD(2.0f);

	dstColor[numVerts] = src->rgba;
}

void gxShapeDraw_FlushTriangles()
{
	if (s_data->m_num3DTriangles + s_data->m_num2DTriangles == 0)
		return;

	UE_PROF_SCOPE("gxShapeDraw_FlushTriangles");

	// Set up transforms

	IwGxSetOrtho(
		(iwfixed) GL_TO_MARMALADE_COORD(s_data->m_2DCanvas.m_left),
		(iwfixed) GL_TO_MARMALADE_COORD(s_data->m_2DCanvas.m_right),
		(iwfixed) GL_TO_MARMALADE_COORD(s_data->m_2DCanvas.m_top),
		(iwfixed) GL_TO_MARMALADE_COORD(s_data->m_2DCanvas.m_bottom),
		-1, -1);

	CIwMat viewTransform;
	glUtils_ToMarmaladeMatrix(&viewTransform, &s_data->m_canvasTransform);
	IwGxSetModelMatrix(&viewTransform);

	// Generate vertex data

	CIwSVec3* pos = s_data->m_pos;
	CIwColour* color = s_data->m_color;
	u32 numVerts = 0;

	for (u32 i = 0; i < s_data->m_num3DTriangles; i++)
	{
		const gxShapeDrawData::Triangle& triangle = s_data->m_triangles[i];
		gxShapeDraw_CopyVertex(&triangle.m_verts[0], pos, color, numVerts++);
		gxShapeDraw_CopyVertex(&triangle.m_verts[1], pos, color, numVerts++);
		gxShapeDraw_CopyVertex(&triangle.m_verts[2], pos, color, numVerts++);
	}

	for (u32 i = 0; i < s_data->m_num2DTriangles; i++)
	{
		const gxShapeDrawData::Triangle& triangle = s_data->m_triangles[s_data->m_maxTriangles - i - 1];
		gxShapeDraw_CopyVertex2D(&triangle.m_verts[0], pos, color, numVerts++);
		gxShapeDraw_CopyVertex2D(&triangle.m_verts[1], pos, color, numVerts++);
		gxShapeDraw_CopyVertex2D(&triangle.m_verts[2], pos, color, numVerts++);
	}

	// Draw

	IwGxSetVertStream(pos, numVerts);
	IwGxSetColStream(color, numVerts);
	IwGxSetUVStream(NULL, 0);
	IwGxSetMaterial(&s_data->m_material);

	IwGxDrawPrims(IW_GX_TRI_LIST, NULL, numVerts);
	IwGxFlush();

	// Reset

	s_data->m_num3DTriangles = 0;
	s_data->m_num2DTriangles = 0;
}

void gxShapeDraw_FlushLines()
{
	if (s_data->m_num3DLines + s_data->m_num2DLines == 0)
		return;
/*
	UE_PROF_SCOPE("gxShapeDraw_FlushLines");

	// Allocate space in VB

	glVertexBufferFactory::LockedChunk vbChunk;
	ueAssertFunc(s_data->m_VB.Allocate((s_data->m_num3DLines + s_data->m_num2DLines) * 2 * sizeof(gxPosColVertex), &vbChunk));
	gxPosColVertex* verts = (gxPosColVertex*) vbChunk.m_data;
	u32 numVerts = 0;

	// Generate data for VB

	for (u32 i = 0; i < s_data->m_num3DLines; i++)
	{
		const gxShapeDrawData::Line& line = s_data->m_lines[i];
		ueMemCpy(verts + numVerts, line.m_verts, 2 * sizeof(gxPosColVertex));
		numVerts += 2;
	}

	for (u32 i = 0; i < s_data->m_num2DLines; i++)
	{
		const gxShapeDrawData::Line& line = s_data->m_lines[s_data->m_maxLines - i - 1];
		ueMemCpy(verts + numVerts, line.m_verts, 2 * sizeof(gxPosColVertex));
		numVerts += 2;
	}

	// Unlock VB

	s_data->m_VB.Unlock(&vbChunk);

	// Draw

	if (s_data->m_num3DLines)
		gxShapeDraw_Draw(
			&vbChunk, NULL,
			0, s_data->m_num3DLines * 2,
			0, 0,
			glPrimitive_LineList);

	if (s_data->m_num2DLines)
		gxShapeDraw_Draw(
			&vbChunk, NULL,
			s_data->m_num3DLines * 2, s_data->m_num2DLines * 2,
			0, 0,
			glPrimitive_LineList,
			UE_TRUE);

	// Reset
*/
	s_data->m_num3DLines = 0;
	s_data->m_num2DLines = 0;
}

void gxShapeDraw_FlushBoxes()
{
	ueNotYetImplemented();
}

void gxShapeDraw_FlushSpheres()
{
	ueNotYetImplemented();
}

void gxShapeDraw_Flush()
{
	ueAssert(s_data->m_ctx);

//	gxShapeDraw_FlushBoxes();
//	gxShapeDraw_FlushSpheres();
	gxShapeDraw_FlushTriangles();
	gxShapeDraw_FlushLines();
}

void gxShapeDraw_DrawBox(const gxShape_Box& box)
{
	ueAssert(s_data->m_ctx);
	if (s_data->m_numWireBoxes + s_data->m_numSolidBoxes == s_data->m_maxBoxes)
		gxShapeDraw_FlushBoxes();

	gxShapeDrawData::Box* geom =
		box.m_wireFrame ?
		&s_data->m_boxes[s_data->m_numWireBoxes++] :
		&s_data->m_boxes[s_data->m_maxBoxes - ++s_data->m_numSolidBoxes];

	geom->m_corners[0].Set(ueVec3_GetX(box.m_box.m_min), ueVec3_GetY(box.m_box.m_min), ueVec3_GetZ(box.m_box.m_min), box.m_transform, box.m_color);
	geom->m_corners[1].Set(ueVec3_GetX(box.m_box.m_max), ueVec3_GetY(box.m_box.m_min), ueVec3_GetZ(box.m_box.m_min), box.m_transform, box.m_color);
	geom->m_corners[2].Set(ueVec3_GetX(box.m_box.m_min), ueVec3_GetY(box.m_box.m_max), ueVec3_GetZ(box.m_box.m_min), box.m_transform, box.m_color);
	geom->m_corners[3].Set(ueVec3_GetX(box.m_box.m_max), ueVec3_GetY(box.m_box.m_max), ueVec3_GetZ(box.m_box.m_min), box.m_transform, box.m_color);
	geom->m_corners[4].Set(ueVec3_GetX(box.m_box.m_min), ueVec3_GetY(box.m_box.m_min), ueVec3_GetZ(box.m_box.m_max), box.m_transform, box.m_color);
	geom->m_corners[5].Set(ueVec3_GetX(box.m_box.m_max), ueVec3_GetY(box.m_box.m_min), ueVec3_GetZ(box.m_box.m_max), box.m_transform, box.m_color);
	geom->m_corners[6].Set(ueVec3_GetX(box.m_box.m_min), ueVec3_GetY(box.m_box.m_max), ueVec3_GetZ(box.m_box.m_max), box.m_transform, box.m_color);
	geom->m_corners[7].Set(ueVec3_GetX(box.m_box.m_max), ueVec3_GetY(box.m_box.m_max), ueVec3_GetZ(box.m_box.m_max), box.m_transform, box.m_color);
}

void gxShapeDraw_DrawSphere(const gxShape_Sphere& sphere)
{
	ueAssert(s_data->m_ctx);
	if (s_data->m_numWireSpheres + s_data->m_numSolidSpheres == s_data->m_maxSpheres)
		gxShapeDraw_FlushSpheres();

	gxShapeDrawData::Sphere* geom =
		sphere.m_wireFrame ?
		&s_data->m_spheres[s_data->m_numWireSpheres++] :
		&s_data->m_spheres[s_data->m_maxSpheres - ++s_data->m_numSolidSpheres];
	geom->m_sphere = sphere.m_sphere;
	geom->m_color = sphere.m_color;
	if (sphere.m_transform)
		geom->m_sphere.Transform(*sphere.m_transform);
}

void gxShapeDraw_DrawTriangle(const gxShape_Triangle& triangle)
{
	ueAssert(s_data->m_ctx);

	if (triangle.m_wireFrame)
	{
		gxShape_Line line;
		line.m_color = triangle.m_color;
		line.m_transform = triangle.m_transform;

		line.m_a = triangle.m_a;
		line.m_b = triangle.m_b;
		gxShapeDraw_DrawLine(line);

		line.m_a = triangle.m_c;
		line.m_b = triangle.m_b;
		gxShapeDraw_DrawLine(line);

		line.m_a = triangle.m_c;
		line.m_b = triangle.m_a;
		gxShapeDraw_DrawLine(line);
	}
	else
	{
		if (s_data->m_num3DTriangles + s_data->m_num2DTriangles == s_data->m_maxTriangles)
			gxShapeDraw_FlushTriangles();

		gxShapeDrawData::Triangle* geom = &s_data->m_triangles[s_data->m_num3DTriangles++];
		geom->m_verts[0].Set(triangle.m_a, triangle.m_transform, triangle.m_color);
		geom->m_verts[1].Set(triangle.m_b, triangle.m_transform, triangle.m_color);
		geom->m_verts[2].Set(triangle.m_c, triangle.m_transform, triangle.m_color);
	}
}

void gxShapeDraw_DrawTriangle(const gxShape_Triangle2D& triangle)
{
	ueAssert(s_data->m_ctx);

	if (triangle.m_wireFrame)
	{
		gxShape_Line2D line;
		line.m_color = triangle.m_color;
		line.m_transform = triangle.m_transform;

		line.m_a = triangle.m_a;
		line.m_b = triangle.m_b;
		gxShapeDraw_DrawLine(line);

		line.m_a = triangle.m_c;
		line.m_b = triangle.m_b;
		gxShapeDraw_DrawLine(line);

		line.m_a = triangle.m_c;
		line.m_b = triangle.m_a;
		gxShapeDraw_DrawLine(line);
	}
	else
	{
		if (s_data->m_num3DTriangles + s_data->m_num2DTriangles == s_data->m_maxTriangles)
			gxShapeDraw_FlushTriangles();

		gxShapeDrawData::Triangle* geom = &s_data->m_triangles[s_data->m_maxTriangles - ++s_data->m_num2DTriangles];
		geom->m_verts[0].Set(triangle.m_a, triangle.m_transform, triangle.m_color);
		geom->m_verts[1].Set(triangle.m_b, triangle.m_transform, triangle.m_color);
		geom->m_verts[2].Set(triangle.m_c, triangle.m_transform, triangle.m_color);
	}
}

void gxShapeDraw_DrawPoly(const gxShape_Poly2D& poly)
{
	ueAssert(s_data->m_ctx);

	if (poly.m_wireFrame)
	{
		ueAssert(!"not yet implemented");
	}
	else
	{
		if (s_data->m_num3DTriangles + s_data->m_num2DTriangles == s_data->m_maxTriangles)
			gxShapeDraw_FlushTriangles();

		const u32 numPolyTriangles = poly.m_numVerts - 2;
		s_data->m_num2DTriangles += numPolyTriangles;

		gxShapeDrawData::Triangle* tris = &s_data->m_triangles[s_data->m_maxTriangles - s_data->m_num2DTriangles];
		for (u32 i = 0; i < numPolyTriangles; i++)
		{
			tris[i].m_verts[0].Set(poly.m_verts[0], poly.m_transform, poly.m_color);
			tris[i].m_verts[1].Set(poly.m_verts[i + 1], poly.m_transform, poly.m_color);
			tris[i].m_verts[2].Set(poly.m_verts[i + 2], poly.m_transform, poly.m_color);
		}
	}
}

void gxShapeDraw_DrawLine(const gxShape_Line& line)
{
	ueAssert(s_data->m_ctx);
	if (s_data->m_num3DLines + s_data->m_num2DLines == s_data->m_maxLines)
		gxShapeDraw_FlushLines();

	gxShapeDrawData::Line* geom = &s_data->m_lines[s_data->m_num3DLines++];
	geom->m_verts[0].Set(line.m_a, line.m_transform, line.m_color);
	geom->m_verts[1].Set(line.m_b, line.m_transform, line.m_color);
}

void gxShapeDraw_DrawLine(const gxShape_Line2D& line)
{
	ueAssert(s_data->m_ctx);
	if (s_data->m_num3DLines + s_data->m_num2DLines == s_data->m_maxLines)
		gxShapeDraw_FlushLines();

	gxShapeDrawData::Line* geom = &s_data->m_lines[s_data->m_maxLines - ++s_data->m_num2DLines];
	geom->m_verts[0].Set(line.m_a, line.m_transform, line.m_color);
	geom->m_verts[1].Set(line.m_b, line.m_transform, line.m_color);
}

void gxShapeDraw_DrawRect(const gxShape_Rect& rect)
{
	if (rect.m_wireFrame)
	{
		gxShape_Line2D line;
		line.m_color = rect.m_color;
		line.m_transform = rect.m_transform;

		rect.m_rect.GetLeftTop(line.m_a);
		rect.m_rect.GetRightTop(line.m_b);
		gxShapeDraw_DrawLine(line);

		rect.m_rect.GetRightTop(line.m_a);
		rect.m_rect.GetRightBottom(line.m_b);
		gxShapeDraw_DrawLine(line);

		rect.m_rect.GetRightBottom(line.m_a);
		rect.m_rect.GetLeftBottom(line.m_b);
		gxShapeDraw_DrawLine(line);

		rect.m_rect.GetLeftBottom(line.m_a);
		rect.m_rect.GetLeftTop(line.m_b);
		gxShapeDraw_DrawLine(line);
	}
	else
	{
		gxShape_Triangle2D triangle;
		triangle.m_transform = rect.m_transform;
		triangle.m_color = rect.m_color;
		triangle.m_wireFrame = UE_FALSE;

		rect.m_rect.GetLeftTop(triangle.m_a);
		rect.m_rect.GetRightTop(triangle.m_b);
		rect.m_rect.GetRightBottom(triangle.m_c);
		gxShapeDraw_DrawTriangle(triangle);

		ueVec2_Copy(triangle.m_b, triangle.m_c);
		rect.m_rect.GetLeftBottom(triangle.m_c);
		gxShapeDraw_DrawTriangle(triangle);
	}
}
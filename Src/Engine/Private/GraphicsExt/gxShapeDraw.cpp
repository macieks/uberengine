#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "Graphics/glVertexBufferFactory.h"
#include "Graphics/glIndexBufferFactory.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxShapeDraw_Vertex.h"

glStreamFormat* gxPosColVertex::m_sf = NULL;
gxProgram gxPosColVertex::m_program;
glStreamFormat* gxPosTexVertex::m_sf = NULL;
gxProgram gxPosTexVertex::m_program;
glStreamFormat* gxPosColTexVertex::m_sf = NULL;
gxProgram gxPosColTexVertex::m_program;

static const u16 s_boxWireIndices[] =
{
	0,1, 1,3, 3,2, 2,0,
	4,5, 5,7, 7,6, 6,4,
	3,7, 6,2, 1,5, 4,0
};

static const u16 s_boxSolidIndices[] =
{
	0,3,1, 0,2,3,
	4,5,7, 4,7,6,
	0,4,2, 2,4,6,
	1,3,5, 5,3,7,
	2,6,3, 6,7,3,
	1,5,4, 1,4,0
};

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

	// Rendering

	glCtx* m_ctx;
	ueMat44 m_viewProj;
	ueRect m_2DCanvas;
	ueMat44 m_canvasTransform;

	gxProgram m_fullscreenRectProgram;
	gxProgram m_fullscreenColorRectProgram;

	glVertexBufferFactory m_VB;
	glIndexBufferFactory m_IB;

	glVertexBuffer* m_quadVB;
};

static gxShapeDrawData* s_data = NULL;

void gxShapeDraw_Startup(gxShapeDrawStartupParams* params)
{
	UE_ASSERT(!s_data);

	s_data = ueNew<gxShapeDrawData>(params->m_stackAllocator);
	UE_ASSERT(s_data);
	s_data->m_allocator = params->m_stackAllocator;
	s_data->m_ctx = NULL;

	// Buffers

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
		sizeof(gxShapeDrawData::Line) * s_data->m_maxLines);
	UE_ASSERT(data);

	s_data->m_boxes = (gxShapeDrawData::Box*) data;
	data += sizeof(gxShapeDrawData::Box) * s_data->m_maxBoxes;

	s_data->m_spheres = (gxShapeDrawData::Sphere*) data;
	data += sizeof(gxShapeDrawData::Sphere) * s_data->m_maxSpheres;

	s_data->m_triangles = (gxShapeDrawData::Triangle*) data;
	data += sizeof(gxShapeDrawData::Triangle) * s_data->m_maxTriangles;

	s_data->m_lines = (gxShapeDrawData::Line*) data;

	// Shaders

	gxPosColVertex::m_program.Create(params->m_posColVSName, params->m_posColFSName);
	gxPosTexVertex::m_program.Create(params->m_posTexVSName, params->m_posTexFSName);
	gxPosColTexVertex::m_program.Create(params->m_posColTexVSName, params->m_posColTexFSName);
	s_data->m_fullscreenRectProgram.Create(params->m_posTexNoTransformVSName, params->m_posColTexFSName);
	s_data->m_fullscreenColorRectProgram.Create(params->m_posNoTransformVSName, params->m_posColFSName);

	// Vertex formats

	const glVertexElement posColSFElems[] =
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(gxPosColVertex, x)},
		{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(gxPosColVertex, rgba)}
	};
	glStreamFormatDesc posColSFDesc;
	posColSFDesc.m_stride = sizeof(gxPosColVertex);
	posColSFDesc.m_numElements = UE_ARRAY_SIZE(posColSFElems);
	posColSFDesc.m_elements = posColSFElems;
	gxPosColVertex::m_sf = glStreamFormat_Create(&posColSFDesc);
	UE_ASSERT(gxPosColVertex::m_sf);

	const glVertexElement posTexSFElems[] =
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(gxPosTexVertex, x)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(gxPosTexVertex, u)}
	};
	glStreamFormatDesc posTexSFDesc;
	posTexSFDesc.m_stride = sizeof(gxPosTexVertex);
	posTexSFDesc.m_numElements = UE_ARRAY_SIZE(posTexSFElems);
	posTexSFDesc.m_elements = posTexSFElems;
	gxPosTexVertex::m_sf = glStreamFormat_Create(&posTexSFDesc);
	UE_ASSERT(gxPosTexVertex::m_sf);

	const glVertexElement posColTexSFElems[] =
	{
		{glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE, UE_OFFSET_OF(gxPosColTexVertex, x)},
		{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(gxPosColTexVertex, rgba)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(gxPosColTexVertex, u)}
	};
	glStreamFormatDesc posColTexSFDesc;
	posColTexSFDesc.m_stride = sizeof(gxPosColTexVertex);
	posColTexSFDesc.m_numElements = UE_ARRAY_SIZE(posColTexSFElems);
	posColTexSFDesc.m_elements = posColTexSFElems;
	gxPosColTexVertex::m_sf = glStreamFormat_Create(&posColTexSFDesc);
	UE_ASSERT(gxPosColTexVertex::m_sf);

	// Vertex and index buffers

	u32 vbSize = s_data->m_maxBoxes * 8 * sizeof(gxPosColVertex);
	vbSize = ueMax(vbSize, s_data->m_maxLines * 2 * sizeof(gxPosColVertex));
	vbSize = ueMax(vbSize, s_data->m_maxTriangles * 3 * sizeof(gxPosColVertex));
	s_data->m_VB.Init(vbSize);
	s_data->m_IB.Init(s_data->m_maxBoxes * UE_ARRAY_SIZE(s_boxSolidIndices) * sizeof(gxPosColVertex), sizeof(u16));

	glVertexBufferDesc quadVBDesc;
	quadVBDesc.m_size = sizeof(gxPosTexVertex) * 4;
#if defined(GL_OPENGL) // Create extra y-inversed quad
	quadVBDesc.m_size *= 2;
#endif
	s_data->m_quadVB = glVertexBuffer_Create(&quadVBDesc);
	UE_ASSERT(s_data->m_quadVB);
	gxPosTexVertex* quadVBVerts = (gxPosTexVertex*) glVertexBuffer_Lock(s_data->m_quadVB);
	UE_ASSERT(quadVBVerts);
	quadVBVerts[0].Set(-1, 1, NULL, ueVec2::Zero);
	quadVBVerts[1].Set(-1, -1, NULL, ueVec2::UnitY);
	quadVBVerts[2].Set(1, 1, NULL, ueVec2::UnitX);
	quadVBVerts[3].Set(1, -1, NULL, ueVec2::One);
#if defined(GL_OPENGL)  // Create extra y-inversed quad
	for (u32 i = 4; i < 8; i++)
	{
		quadVBVerts[i] = quadVBVerts[i - 4];
		quadVBVerts[i].y = -quadVBVerts[i].y;
	}
#endif
	glVertexBuffer_Unlock(s_data->m_quadVB);
}

void gxShapeDraw_Shutdown()
{
	UE_ASSERT(s_data);

	glVertexBuffer_Destroy(s_data->m_quadVB);

	s_data->m_VB.Deinit();
	s_data->m_IB.Deinit();

	s_data->m_fullscreenRectProgram.Destroy();
	s_data->m_fullscreenColorRectProgram.Destroy();
	gxPosColVertex::m_program.Destroy();
	gxPosTexVertex::m_program.Destroy();
	gxPosColTexVertex::m_program.Destroy();

	glStreamFormat_Destroy(gxPosColVertex::m_sf);
	glStreamFormat_Destroy(gxPosTexVertex::m_sf);
	glStreamFormat_Destroy(gxPosColTexVertex::m_sf);

	s_data->m_allocator->Free(s_data->m_boxes);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxShapeDraw_SetDrawParams(const gxShapeDrawParams* params)
{
	s_data->m_viewProj = params->m_viewProj ? *params->m_viewProj : ueMat44::Identity;
	s_data->m_2DCanvas = params->m_2DCanvas;
	s_data->m_canvasTransform = params->m_canvasTransform ? *params->m_canvasTransform : ueMat44::Identity;
}

void gxShapeDraw_Begin(glCtx* ctx)
{
	UE_ASSERT(!s_data->m_ctx);
	s_data->m_ctx = ctx;
}

void gxShapeDraw_End()
{
	UE_ASSERT(s_data->m_ctx);
	gxShapeDraw_Flush();
	s_data->m_ctx = NULL;
}

void gxShapeDraw_Draw(glVertexBufferFactory::LockedChunk* vbChunk, glIndexBufferFactory::LockedChunk* ibChunk, u32 firstVertex, u32 numVerts, u32 firstIndex, u32 numIndices, glPrimitive prim, ueBool draw2D = UE_FALSE)
{
	glCtx* ctx = s_data->m_ctx;

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, UE_TRUE);
	glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
	glCtx_SetBlendOp(ctx, glBlendOp_Add);

	if (draw2D)
	{
		glCtx_SetCullMode(ctx, glCullMode_None);
		glCtx_SetDepthWrite(ctx, UE_FALSE);
		glCtx_SetDepthTest(ctx, UE_FALSE);

		ueMat44 orthoProj;
		orthoProj.SetOrthoOffCenter(s_data->m_2DCanvas.m_left, s_data->m_2DCanvas.m_right, s_data->m_2DCanvas.m_top, s_data->m_2DCanvas.m_bottom, -1, 1, glCtx_IsRenderingToTexture(ctx));

		ueMat44 worldViewProj;
		ueMat44::Mul(worldViewProj, s_data->m_canvasTransform, orthoProj);

		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldViewProj, &worldViewProj);
	}
	else
	{
		glCtx_SetCullMode(ctx, glCullMode_CW);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthTest(ctx, UE_TRUE);

		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldViewProj, &s_data->m_viewProj);
	}

	glCtx_SetStream(ctx, 0, vbChunk->m_VB, gxPosColVertex::m_sf, vbChunk->m_offset);
	if (ibChunk)
		glCtx_SetIndices(ctx, ibChunk->m_IB);

	glCtx_SetProgram(ctx, gxPosColVertex::m_program.GetProgram());

	if (ibChunk)
		glCtx_DrawIndexed(ctx, prim, 0, firstVertex, numVerts, ibChunk->m_firstIndex + firstIndex, numIndices);
	else
		glCtx_Draw(ctx, prim, firstVertex, numVerts);

	// Revert default render states

	if (!draw2D)
	{
		glCtx_SetBlending(ctx, UE_FALSE);
		glCtx_SetCullMode(ctx, glCullMode_CW);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthTest(ctx, UE_TRUE);
	}
}

void gxShapeDraw_FlushBoxes()
{
	if (s_data->m_numWireBoxes + s_data->m_numSolidBoxes == 0)
		return;

	UE_PROF_SCOPE("gxShapeDraw_FlushBoxes");

	// Allocate space in VB & IB

	glVertexBufferFactory::LockedChunk vbChunk;
	UE_ASSERT_FUNC(s_data->m_VB.Allocate((s_data->m_numWireBoxes + s_data->m_numSolidBoxes) * 8 * sizeof(gxPosColVertex), &vbChunk));
	gxPosColVertex* verts = (gxPosColVertex*) vbChunk.m_data;
	u32 numVerts = 0;

	glIndexBufferFactory::LockedChunk ibChunk;
	UE_ASSERT_FUNC(s_data->m_IB.Allocate(s_data->m_numSolidBoxes * UE_ARRAY_SIZE(s_boxSolidIndices) + s_data->m_numWireBoxes * UE_ARRAY_SIZE(s_boxWireIndices), &ibChunk));
	u16* indices = (u16*) ibChunk.m_data;
	u32 numIndices = 0;

	// Generate data for VB & IB

	for (u32 i = 0; i < s_data->m_numWireBoxes; i++)
	{
		const gxShapeDrawData::Box& box = s_data->m_boxes[i];

		for (u32 j = 0; j < UE_ARRAY_SIZE(s_boxWireIndices); j++)
			indices[numIndices + j] = s_boxWireIndices[j] + numVerts;
		numIndices += UE_ARRAY_SIZE(s_boxWireIndices);

		ueMemCpy(verts + numVerts, box.m_corners, 8 * sizeof(gxPosColVertex));
		numVerts += 8;
	}

	for (u32 i = s_data->m_maxBoxes - s_data->m_numSolidBoxes; i < s_data->m_maxBoxes; i++)
	{
		const gxShapeDrawData::Box& box = s_data->m_boxes[i];

		for (u32 j = 0; j < UE_ARRAY_SIZE(s_boxSolidIndices); j++)
			indices[numIndices + j] = s_boxSolidIndices[j] + numVerts;
		numIndices += UE_ARRAY_SIZE(s_boxSolidIndices);

		ueMemCpy(verts + numVerts, box.m_corners, 8 * sizeof(gxPosColVertex));
		numVerts += 8;
	}

	// Unlock VB & IB

	s_data->m_VB.Unlock(&vbChunk);
	s_data->m_IB.Unlock(&ibChunk);

	// Draw

	if (s_data->m_numWireBoxes)
		gxShapeDraw_Draw(
			&vbChunk, &ibChunk,
			0, s_data->m_numWireBoxes * 8,
			0, s_data->m_numWireBoxes * UE_ARRAY_SIZE(s_boxWireIndices),
			glPrimitive_LineList);

	if (s_data->m_numSolidBoxes)
		gxShapeDraw_Draw(
			&vbChunk, &ibChunk,
			s_data->m_numWireBoxes * 8, s_data->m_numSolidBoxes * 8,
			s_data->m_numWireBoxes * UE_ARRAY_SIZE(s_boxWireIndices), s_data->m_numSolidBoxes * UE_ARRAY_SIZE(s_boxSolidIndices),
			glPrimitive_TriangleList);

	// Reset

	s_data->m_numWireBoxes = 0;
	s_data->m_numSolidBoxes = 0;
}

void gxShapeDraw_FlushSpheres()
{
//	UE_NOT_IMPLEMENTED();
}

void gxShapeDraw_FlushTriangles()
{
	if (s_data->m_num3DTriangles + s_data->m_num2DTriangles == 0)
		return;

	UE_PROF_SCOPE("gxShapeDraw_FlushTriangles");

	// Allocate space in VB

	glVertexBufferFactory::LockedChunk vbChunk;
	UE_ASSERT_FUNC(s_data->m_VB.Allocate((s_data->m_num3DTriangles + s_data->m_num2DTriangles) * 3 * sizeof(gxPosColVertex), &vbChunk));
	gxPosColVertex* verts = (gxPosColVertex*) vbChunk.m_data;
	u32 numVerts = 0;

	// Generate data for VB

	for (u32 i = 0; i < s_data->m_num3DTriangles; i++)
	{
		const gxShapeDrawData::Triangle& triangle = s_data->m_triangles[i];
		ueMemCpy(verts + numVerts, triangle.m_verts, 3 * sizeof(gxPosColVertex));
		numVerts += 3;
	}

	for (u32 i = 0; i < s_data->m_num2DTriangles; i++)
	{
		const gxShapeDrawData::Triangle& triangle = s_data->m_triangles[s_data->m_maxTriangles - i - 1];
		ueMemCpy(verts + numVerts, triangle.m_verts, 3 * sizeof(gxPosColVertex));
		numVerts += 3;
	}

	// Unlock VB

	s_data->m_VB.Unlock(&vbChunk);

	// Draw

	if (s_data->m_num3DTriangles)
		gxShapeDraw_Draw(
			&vbChunk, NULL,
			0, s_data->m_num3DTriangles * 3,
			0, 0,
			glPrimitive_TriangleList);

	if (s_data->m_num2DTriangles)
		gxShapeDraw_Draw(
			&vbChunk, NULL,
			s_data->m_num3DTriangles * 3, s_data->m_num2DTriangles * 3,
			0, 0,
			glPrimitive_TriangleList,
			UE_TRUE);

	// Reset

	s_data->m_num3DTriangles = 0;
	s_data->m_num2DTriangles = 0;
}

void gxShapeDraw_FlushLines()
{
	if (s_data->m_num3DLines + s_data->m_num2DLines == 0)
		return;

	UE_PROF_SCOPE("gxShapeDraw_FlushLines");

	// Allocate space in VB

	glVertexBufferFactory::LockedChunk vbChunk;
	UE_ASSERT_FUNC(s_data->m_VB.Allocate((s_data->m_num3DLines + s_data->m_num2DLines) * 2 * sizeof(gxPosColVertex), &vbChunk));
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

	s_data->m_num3DLines = 0;
	s_data->m_num2DLines = 0;
}

void gxShapeDraw_Flush()
{
	UE_ASSERT(s_data->m_ctx);

	gxShapeDraw_FlushBoxes();
	gxShapeDraw_FlushSpheres();
	gxShapeDraw_FlushTriangles();
	gxShapeDraw_FlushLines();
}

void gxShapeDraw_DrawBox(const gxShape_Box& box)
{
	UE_ASSERT(s_data->m_ctx);
	if (s_data->m_numWireBoxes + s_data->m_numSolidBoxes == s_data->m_maxBoxes)
		gxShapeDraw_FlushBoxes();

	gxShapeDrawData::Box* geom =
		box.m_wireFrame ?
		&s_data->m_boxes[s_data->m_numWireBoxes++] :
		&s_data->m_boxes[s_data->m_maxBoxes - ++s_data->m_numSolidBoxes];

	u32 corner = 0;
	for (u32 x = 0; x < 2; x++)
		for (u32 y = 0; y < 2; y++)
			for (u32 z = 0; z < 2; z++)
				geom->m_corners[corner++].Set(
					x ? box.m_box.m_min[0] : box.m_box.m_max[0],
					y ? box.m_box.m_min[1] : box.m_box.m_max[1],
					z ? box.m_box.m_min[2] : box.m_box.m_max[2],
					box.m_transform,
					box.m_color);
}

void gxShapeDraw_DrawSphere(const gxShape_Sphere& sphere)
{
	UE_ASSERT(s_data->m_ctx);
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
	UE_ASSERT(s_data->m_ctx);

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
	UE_ASSERT(s_data->m_ctx);

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
	UE_ASSERT(s_data->m_ctx);

	if (poly.m_wireFrame)
	{
		UE_ASSERT(!"not yet implemented");
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
	UE_ASSERT(s_data->m_ctx);
	if (s_data->m_num3DLines + s_data->m_num2DLines == s_data->m_maxLines)
		gxShapeDraw_FlushLines();

	gxShapeDrawData::Line* geom = &s_data->m_lines[s_data->m_num3DLines++];
	geom->m_verts[0].Set(line.m_a, line.m_transform, line.m_color);
	geom->m_verts[1].Set(line.m_b, line.m_transform, line.m_color);
}

void gxShapeDraw_DrawLine(const gxShape_Line2D& line)
{
	UE_ASSERT(s_data->m_ctx);
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

		line.m_a = rect.m_rect.GetLeftTop();
		line.m_b = rect.m_rect.GetRightTop();
		gxShapeDraw_DrawLine(line);

		line.m_a = rect.m_rect.GetRightTop();
		line.m_b = rect.m_rect.GetRightBottom();
		gxShapeDraw_DrawLine(line);

		line.m_a = rect.m_rect.GetRightBottom();
		line.m_b = rect.m_rect.GetLeftBottom();
		gxShapeDraw_DrawLine(line);

		line.m_a = rect.m_rect.GetLeftBottom();
		line.m_b = rect.m_rect.GetLeftTop();
		gxShapeDraw_DrawLine(line);
	}
	else
	{
		gxShape_Triangle2D triangle;
		triangle.m_transform = rect.m_transform;
		triangle.m_color = rect.m_color;
		triangle.m_wireFrame = UE_FALSE;

		triangle.m_a = rect.m_rect.GetLeftTop();
		triangle.m_b = rect.m_rect.GetRightTop();
		triangle.m_c = rect.m_rect.GetRightBottom();
		gxShapeDraw_DrawTriangle(triangle);

		triangle.m_b = triangle.m_c;
		triangle.m_c = rect.m_rect.GetLeftBottom();
		gxShapeDraw_DrawTriangle(triangle);
	}
}

void gxShapeDraw_DrawFullscreenRect(const ueVec4& color, ueBool enableBlending)
{
	glCtx* ctx = s_data->m_ctx ? s_data->m_ctx : glDevice_GetDefaultContext();

	// Set render states

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, enableBlending);
	if (enableBlending)
	{
		glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
		glCtx_SetBlendOp(ctx, glBlendOp_Add);
	}

	glCtx_SetCullMode(ctx, glCullMode_None);
	glCtx_SetDepthWrite(ctx, UE_FALSE);
	glCtx_SetDepthTest(ctx, UE_FALSE);

	// Set shader constants

	glRenderGroup* group = glCtx_GetCurrentRenderGroup(ctx);
	const f32 rtWidth = (f32) glRenderGroup_GetDesc(group)->m_width;
	const f32 rtHeight = (f32) glRenderGroup_GetDesc(group)->m_height;

	ueVec2 pixelOffset;
	if (glDevice_GetCaps()->m_hasHalfPixelOffset)
		pixelOffset.Set(-1.0f / rtWidth, 1.0f / rtHeight);
	else
		pixelOffset.Zeroe();
	glCtx_SetFloat2Constant(ctx, gxCommonConstants::PixelOffset, &pixelOffset);

	glCtx_SetFloat4Constant(ctx, gxCommonConstants::Color, &color);

	// Set stream & shaders

	glCtx_SetStream(ctx, 0, s_data->m_quadVB, gxPosTexVertex::m_sf);
	glCtx_SetProgram(ctx, s_data->m_fullscreenColorRectProgram.GetProgram());

	// Draw

	glCtx_Draw(ctx, glPrimitive_TriangleStrip, 0, 4);

	// Revert default render states

	glCtx_SetBlending(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_CW);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetDepthTest(ctx, UE_TRUE);
}

void gxShapeDraw_DrawTexturedRect(const gxShape_TexturedRect& rect)
{
	ueVec2 pos[4];
	ueVec2 tex[4];

	pos[0] = rect.m_pos.GetLeftTop();
	tex[0] = rect.m_tex.GetLeftTop();

	pos[1] = rect.m_pos.GetRightTop();
	tex[1] = rect.m_tex.GetRightTop();

	pos[2] = rect.m_pos.GetRightBottom();
	tex[2] = rect.m_tex.GetRightBottom();

	pos[3] = rect.m_pos.GetLeftBottom();
	tex[3] = rect.m_tex.GetLeftBottom();

	gxShape_TexturedPoly poly;
	*(gxShape_Textured*) &poly = *(gxShape_Textured*) &rect;
	poly.m_transform = rect.m_transform;
	poly.m_numVerts = 4;
	poly.m_pos2D = pos;
	poly.m_tex = tex;
	gxShapeDraw_DrawTexturedPoly(poly);
}

UE_INLINE u32 glUtils_TriangleFanToStripVertexIndex(u32 i, u32 n)
{
	return i < (n - 1 - i) ? (i << 1) : (((n - 1 - i) << 1) + 1);
}

void gxShapeDraw_DrawTexturedPoly(const gxShape_TexturedPoly& poly)
{
	UE_ASSERT(poly.m_numVerts > 0 && (poly.m_pos || poly.m_pos2D) && poly.m_tex);

	glCtx* ctx = s_data->m_ctx ? s_data->m_ctx : glDevice_GetDefaultContext();

	ueColor32 color32 = ueColor32::White;
	if (poly.m_color)
		color32 = ueColor32(*poly.m_color);

	// Build verts in vertex buffer

	glVertexBufferFactory::LockedChunk vbChunk;
	if (!s_data->m_VB.Allocate(poly.m_numVerts * sizeof(gxPosColTexVertex), &vbChunk))
	{
		// Flush & try again

		gxShapeDraw_Flush();
		UE_ASSERT_FUNC(s_data->m_VB.Allocate(poly.m_numVerts * sizeof(gxPosColTexVertex), &vbChunk));
	}
	gxPosColTexVertex* verts = (gxPosColTexVertex*) vbChunk.m_data;
	if (poly.m_pos)
		for (u32 i = 0; i < poly.m_numVerts; i++)
			verts[glUtils_TriangleFanToStripVertexIndex(i, poly.m_numVerts)].Set(poly.m_pos[i], poly.m_transform, poly.m_tex[i], color32);
	else
	{
		UE_ASSERT(poly.m_pos2D);
		u32 start = 0;
		u32 end = poly.m_numVerts - 1;
		ueBool takeStart = UE_TRUE;
		for (u32 i = 0; i < poly.m_numVerts; i++)
		{
			u32 index = 0;
			if (takeStart)
			{
				index = start++;
				takeStart = !takeStart;
			}
			else
			{
				index = end--;
				takeStart = !takeStart;
			}
			verts[i].SetV2(poly.m_pos2D[index], poly.m_transform, poly.m_tex[index], color32);
		}
	}
	s_data->m_VB.Unlock(&vbChunk);

	// Set render states

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, poly.m_enableBlending);
	if (poly.m_enableBlending)
	{
		glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
		glCtx_SetBlendOp(ctx, glBlendOp_Add);
	}

	glCtx_SetCullMode(ctx, glCullMode_None);
	glCtx_SetDepthWrite(ctx, poly.m_enableDepthWrite);
	glCtx_SetDepthTest(ctx, poly.m_enableDepthTest);

	// Set pixel offset

	glRenderGroup* group = glCtx_GetCurrentRenderGroup(ctx);
	const f32 rtWidth = (f32) glRenderGroup_GetDesc(group)->m_width;
	const f32 rtHeight = (f32) glRenderGroup_GetDesc(group)->m_height;

	// Set render target size

	glCtx_SetFloat4Constant(ctx, gxCommonConstants::RenderTargetSize, &ueVec4(rtWidth, rtHeight, 1.0f / rtWidth, 1.0f / rtHeight));

	// Set texture

	if (poly.m_colorMap)
		glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, poly.m_colorMap, poly.m_colorMapSampler);

	// Set transform

	ueMat44 viewProj;
	if (poly.m_pos)
		viewProj = s_data->m_viewProj;
	else
		viewProj.SetOrthoOffCenter(s_data->m_2DCanvas.m_left, s_data->m_2DCanvas.m_right, s_data->m_2DCanvas.m_top, s_data->m_2DCanvas.m_bottom, -1, 1, glCtx_IsRenderingToTexture(ctx));

	if (glDevice_GetCaps()->m_hasHalfPixelOffset)
	{
		ueMat44 halfTexelOffsetTransform;
		halfTexelOffsetTransform.SetTranslation(-0.5f / rtWidth, -0.5f / rtHeight, 0.0f);
		viewProj *= halfTexelOffsetTransform;
	}

	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldViewProj, &viewProj);

	// Set stream & shaders

	glCtx_SetStream(ctx, 0, vbChunk.m_VB, gxPosColTexVertex::m_sf, vbChunk.m_offset);
	glCtx_SetProgram(ctx, poly.m_program ? poly.m_program : gxPosColTexVertex::m_program.GetProgram());

	// Draw

	glCtx_Draw(ctx, glPrimitive_TriangleStrip, 0, poly.m_numVerts);

	// Revert default render states

	glCtx_SetBlending(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_CW);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetDepthTest(ctx, UE_TRUE);
}

void gxShapeDraw_DrawFullscreenRect(const gxShape_FullscreenRect& rect)
{
	glCtx* ctx = s_data->m_ctx ? s_data->m_ctx : glDevice_GetDefaultContext();

	// Set render states

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, rect.m_enableBlending);
	if (rect.m_enableBlending)
	{
		glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
		glCtx_SetBlendOp(ctx, glBlendOp_Add);
	}

	glCtx_SetCullMode(ctx, glCullMode_None);
	glCtx_SetDepthWrite(ctx, rect.m_enableDepthWrite);
	glCtx_SetDepthTest(ctx, rect.m_enableDepthTest);

	// Set shader constants

	glRenderGroup* group = glCtx_GetCurrentRenderGroup(ctx);
	const f32 rtWidth = (f32) glRenderGroup_GetDesc(group)->m_width;
	const f32 rtHeight = (f32) glRenderGroup_GetDesc(group)->m_height;

	ueVec2 pixelOffset;
	if (glDevice_GetCaps()->m_hasHalfPixelOffset)
		pixelOffset.Set(-1.0f / rtWidth, 1.0f / rtHeight);
	else
		pixelOffset.Zeroe();
	glCtx_SetFloat2Constant(ctx, gxCommonConstants::PixelOffset, &pixelOffset);

	glCtx_SetFloat4Constant(ctx, gxCommonConstants::RenderTargetSize, &ueVec4(rtWidth, rtHeight, 1.0f / rtWidth, 1.0f / rtHeight));

	if (rect.m_color)
		glCtx_SetFloat4Constant(ctx, gxCommonConstants::Color, rect.m_color);

	// Set texture

	if (rect.m_colorMap)
		glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, rect.m_colorMap, rect.m_colorMapSampler);

	// Set stream & shaders

	u32 vbOffset = 0;
#if defined(GL_OPENGL)
	if (glCtx_IsRenderingToTexture(ctx))
		vbOffset = sizeof(gxPosTexVertex) * 4; // Use y-inversed quad
#endif

	glCtx_SetStream(ctx, 0, s_data->m_quadVB, gxPosTexVertex::m_sf, vbOffset);
	glCtx_SetProgram(ctx, rect.m_program ? rect.m_program : s_data->m_fullscreenRectProgram.GetProgram());

	// Draw

	glCtx_Draw(ctx, glPrimitive_TriangleStrip, 0, 4);

	// Revert default render states

	glCtx_SetFillMode(ctx, glFillMode_Solid);
	glCtx_SetBlending(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_CW);
	glCtx_SetDepthWrite(ctx, UE_TRUE);
	glCtx_SetDepthTest(ctx, UE_TRUE);
}

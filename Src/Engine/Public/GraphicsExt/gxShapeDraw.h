#ifndef GX_SHAPE_DRAW_H
#define GX_SHAPE_DRAW_H

#include "Graphics/glLib.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Simple shape draw startup parameters
struct gxShapeDrawStartupParams
{
	ueAllocator* m_stackAllocator;	//!< Prefarably stack allocator

	u32 m_maxBoxBatch;			//!< Max. number of boxes in a single draw batch
	u32 m_maxSphereBatch;		//!< Max. number of spheres in a single draw batch
	u32 m_maxTriangleBatch;		//!< Max. number of triangles in a single draw batch
	u32 m_maxLineBatch;			//!< Max. number of lines in a single draw batch

	const char* m_posNoTransformVSName;	//!< Name of the no-transform "position" vertex shader to be used
	const char* m_posTexNoTransformVSName;	//!< Name of the no-transform "position + texture" vertex shader to be used

	const char* m_posColVSName;	//!< Name of the "position + color" vertex shader to be used
	const char* m_posColFSName;	//!< Name of the "position + color" fragment shader to be used

	const char* m_posTexVSName;	//!< Name of the "position + texture" vertex shader to be used
	const char* m_posTexFSName;	//!< Name of the "position + texture" fragment shader to be used

	const char* m_posColTexVSName;//!< Name of the "position + color + texture" vertex shader to be used
	const char* m_posColTexFSName;//!< Name of the "position + color + texture" fragment shader to be used

	gxShapeDrawStartupParams() :
		m_stackAllocator(NULL),
		m_maxBoxBatch(1 << 8),
		m_maxSphereBatch(1 << 8),
		m_maxTriangleBatch(1 << 10),
		m_maxLineBatch(1 << 10),
		m_posNoTransformVSName("common/pos_no_transform_vs"),
		m_posTexNoTransformVSName("common/pos_tex_no_transform_vs"),
		m_posColVSName("common/pos_col_vs"),
		m_posColFSName("common/pos_col_fs"),
		m_posTexVSName("common/pos_tex_vs"),
		m_posTexFSName("common/pos_tex_fs"),
		m_posColTexVSName("common/pos_col_tex_vs"),
		m_posColTexFSName("common/pos_col_tex_fs")
	{}
};

//! Draw parameters
struct gxShapeDrawParams
{
	const ueMat44* m_viewProj;	//!< 3D drawing only: view projection transform
	ueRect m_2DCanvas;			//!< 2D drawing only: 2D canvas rectangle
	const ueMat44* m_canvasTransform; //!< 2D drawing only: optional canvas transform

	gxShapeDrawParams() :
		m_viewProj(NULL),
		m_2DCanvas(0, 0, 1, 1),
		m_canvasTransform(NULL)
	{}
};

// Drawable shapes
// ---------------------

struct gxShape_Base
{
	const ueMat44* m_transform;	//!< Optional transformation
	ueColor32 m_color;		//!< Color

	gxShape_Base() :
		m_transform(NULL),
		m_color(ueColor32::White)
	{}
};

struct gxShape_Solid : gxShape_Base
{
	ueBool m_wireFrame;		//!< Toggles wire frame or solid rendering

	gxShape_Solid(ueBool wireFrame = UE_TRUE) :
		m_wireFrame(wireFrame)
	{}
};

//! Box shape
struct gxShape_Box : gxShape_Solid
{
	ueBox m_box; //!< Box to draw
};

//! Sphere shape
struct gxShape_Sphere : gxShape_Solid
{
	ueSphere m_sphere; //!< Sphere to draw
};

//! Triangle shape
struct gxShape_Triangle : gxShape_Solid
{
	ueVec3 m_a;	//!< A point
	ueVec3 m_b;	//!< B point
	ueVec3 m_c;	//!< C point
};

//! 2D triangle shape
struct gxShape_Triangle2D : gxShape_Solid
{
	ueVec2 m_a;	//!< A point
	ueVec2 m_b;	//!< B point
	ueVec2 m_c;	//!< C point
};

//! 2D polygon shape
struct gxShape_Poly2D : gxShape_Solid
{
	u32 m_numVerts;			//!< Number of verts
	const ueVec2* m_verts;	//!< Vertices
};

//! Line shape
struct gxShape_Line : gxShape_Base
{
	ueVec3 m_a;	//!< A point
	ueVec3 m_b;	//!< B point
};

//! 2D line shape
struct gxShape_Line2D : gxShape_Base
{
	ueVec2 m_a;	//!< A point
	ueVec2 m_b;	//!< B point
};

//! 2D rectangle shape
struct gxShape_Rect : gxShape_Solid
{
	ueRect m_rect;	//!< Rectangle
};

//! Base textured shape
struct gxShape_Textured
{
	glProgram* m_program;//!< Optional program to use for rendering
	glTextureBuffer* m_colorMap;		//!< Texture
	glSamplerParams* m_colorMapSampler;	//!< Sampler
	ueBool m_enableBlending;			//!< Indicates whether to enable alpha blending or not
	ueBool m_enableDepthTest;			//!< Depth test
	ueBool m_enableDepthWrite;			//!< Depth write
	ueVec4* m_color;					//!< Optional color

	gxShape_Textured() :
		m_program(NULL),
		m_colorMap(NULL),
		m_colorMapSampler(&glSamplerParams::DefaultClamp),
		m_enableBlending(UE_FALSE),
		m_enableDepthTest(UE_FALSE),
		m_enableDepthWrite(UE_FALSE),
		m_color(NULL)
	{}
};

//! Textured polygon shape (2D or 3D)
struct gxShape_TexturedPoly : gxShape_Textured
{
	ueMat44* m_transform;	//!< Optional transformation
	u32 m_numVerts;			//!< Number of vertices
	const ueVec2* m_pos2D;	//!< Optional array of 2D positions
	const ueVec3* m_pos;	//!< Optional array of 3D positions
	const ueVec2* m_tex;	//!< An array of texture coordinates

	gxShape_TexturedPoly() :
		m_transform(NULL),
		m_numVerts(0),
		m_pos2D(NULL),
		m_pos(NULL),
		m_tex(NULL)
	{}
};

//! Textured rectangle shape
struct gxShape_TexturedRect : gxShape_Textured
{
	ueMat44* m_transform;//!< Optional transformation
	ueRect m_pos;		//!< Position
	ueRect m_tex;		//!< Texture coordinates

	gxShape_TexturedRect() :
		m_transform(NULL),
		m_pos(0, 0, 1, 1),
		m_tex(0, 0, 1, 1)
	{}
};

//! Fullscreen rectangle shape
struct gxShape_FullscreenRect : gxShape_Textured
{
	gxShape_FullscreenRect() {}
};

//! Starts up shape drawing utility
void gxShapeDraw_Startup(gxShapeDrawStartupParams* params);
//! Shuts down shape drawing utility
void gxShapeDraw_Shutdown();

//! Sets draw parameters
void gxShapeDraw_SetDrawParams(const gxShapeDrawParams* params);
//! Begins drawing
void gxShapeDraw_Begin(glCtx* ctx);
//! Ends drawing
void gxShapeDraw_End();

//! Finishes rendering of all queued shapes
void gxShapeDraw_Flush();

// Non-textured geometry - batched deferred draw (call gxShapeDraw_Flush to flush commands)

//! Draws box
void gxShapeDraw_DrawBox(const gxShape_Box& box);
//! Draw sphere
void gxShapeDraw_DrawSphere(const gxShape_Sphere& sphere);
//! Draws triangle
void gxShapeDraw_DrawTriangle(const gxShape_Triangle& triangle);
//! Draws 2D triangle
void gxShapeDraw_DrawTriangle(const gxShape_Triangle2D& triangle);
//! Draws 2D polygon
void gxShapeDraw_DrawPoly(const gxShape_Poly2D& poly);
//! Draw line
void gxShapeDraw_DrawLine(const gxShape_Line& line);
//! Draw 2D line
void gxShapeDraw_DrawLine(const gxShape_Line2D& line);
//! Draws rectangle in 3D
void gxShapeDraw_DrawRect(const gxShape_Rect& rect);
//! Draws fullscreen rectangle in given color
void gxShapeDraw_DrawFullscreenRect(const ueVec4& color = ueVec4::Zero, ueBool enableBlending = UE_FALSE);

// Textured geometry - immediate draw

//! Draws textured rectangle in 2D
void gxShapeDraw_DrawTexturedRect(const gxShape_TexturedRect& rect);
//! Draws textured polygon
void gxShapeDraw_DrawTexturedPoly(const gxShape_TexturedPoly& poly);
//! Draws fullscreen rectangle using given shaders in 2D
void gxShapeDraw_DrawFullscreenRect(const gxShape_FullscreenRect& rect);

// @}

#endif // GX_SHAPE_DRAW_H

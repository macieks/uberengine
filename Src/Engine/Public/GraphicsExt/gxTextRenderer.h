#ifndef GX_TEXT_RENDERER_H
#define GX_TEXT_RENDERER_H

#include "Base/ueMath.h"

struct glCtx;
struct gxFont;

/**
 *	@addtogroup gx
 *	@{
 */

//! Description of the text to be rendered
struct gxText
{
	f32 m_x;			//!< Left text corner
	f32 m_y;			//!< Top text corner

	f32 m_scaleX;		//!< Scale of the text along X axis
	f32 m_scaleY;		//!< Scale of the text along Y axis

	gxFont* m_font;		//!< Font to be used; NULL indicates default font (see gxTextRendererStartupParams)

	ueMat44* m_transform;		//!< Optional tranformation to be applied to text

	ueColor32 m_color;			//!< Text color

	const u32* m_utf32Buffer;	//!< Optional UTF-32 buffer; use this to avoid encoding conversion
	const char* m_utf8Buffer;	//!< Optional UTF-8 buffer; using this will cause conversion to UTF-32 before rendering
	u32 m_bufferSize;			//!< Buffer length measured in chars for UTF-8 and u32 for UTF-32; 0 indicates all characters (i.e. assuming NULL termination)

	gxText() :
		m_x(20.0f),
		m_y(20.0f),
		m_scaleX(1.0f),
		m_scaleY(1.0f),
		m_font(NULL),
		m_transform(&ueMat44::Identity),
		m_utf32Buffer(NULL),
		m_utf8Buffer(NULL),
		m_bufferSize(0)
	{}
};

//! Startup parameters for text renderer
struct gxTextRendererStartupParams
{
	ueAllocator* m_stackAllocator;	//!< Prefarably stack allocator
	u32 m_charBufferSize;			//!< Buffer size used to batch-render characters

	const char* m_VSName;			//!< Name of the vertex shader used to draw text
	const char* m_FSName;			//!< Name of the fragment shader used to draw text

	const char* m_defaultFontName;	//!< Optional name of the default font

	gxTextRendererStartupParams() :
		m_stackAllocator(NULL),
		m_charBufferSize(1 << 12),
		m_VSName("common/font_vs"),
		m_FSName("common/font_fs"),
		m_defaultFontName("common/debug_font")
	{}
};

// Text renderer

//! Starts up text renderer
void	gxTextRenderer_Startup(gxTextRendererStartupParams* params);
//! Shuts down text renderer
void	gxTextRenderer_Shutdown();

//! Draws the text
void	gxTextRenderer_Draw(glCtx* ctx, gxText* desc);
//! Draws the text
void	gxTextRenderer_Draw(glCtx* ctx, f32 x, f32 y, const char* utf8Buffer, ueColor32 color = ueColor32::White);
//! Calculates text dimension
void	gxTextRenderer_CalcDimensions(gxText* desc, f32& width, f32& height);

//! Returns default font (or NULL if not specified in gxTextRendererStartupParams)
gxFont*	gxTextRenderer_GetDefaultFont();

// @}

#endif // GX_TEXT_RENDERER_H

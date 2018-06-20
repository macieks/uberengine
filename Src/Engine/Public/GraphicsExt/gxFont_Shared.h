#ifndef GX_FONT_SHARED_H
#define GX_FONT_SHARED_H

#include "Base/ueBase.h"

/**
 *	@addtogroup gx
 *	@{
 */

//! Font options
enum gxFontFlags
{
	gxFontFlags_Bold		=	UE_POW2(0),	//!< Bold
	gxFontFlags_Italic		=	UE_POW2(1),	//!< Italic
	gxFontFlags_Underlined	=	UE_POW2(2)	//!< Underlined
};

//! Font glyph
struct gxFontGlyph
{
	u32 m_channelMask;	//!< RGBA mask used when rendering font

	f32 m_left;			//!< Left coordinate (within glyphs atlas)
	f32 m_top;			//!< Top coordinate (within glyphs atlas)
	f32 m_right;		//!< Right coordinate (within glyphs atlas)
	f32 m_bottom;		//!< Bottom coordinate (within glyphs atlas)

	f32 m_widthInPixels;	//!< Width of the bitmap in pixels
	f32 m_heightInPixels;	//!< Height of the bitmap in pixels

	f32 m_offsetX;		//!< Offset along X axis of the bitmap to apply when drawing
	f32 m_offsetY;		//!< Offset along Y axis of the bitmap to apply when drawing

	f32 m_advanceX;		//!< Number of pixels to advance before drawing next character
};

// @}

#endif // GX_FONT_SHARED_H

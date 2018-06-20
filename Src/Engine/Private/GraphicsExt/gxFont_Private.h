#ifndef GX_FONT_PRIVATE_H
#define GX_FONT_PRIVATE_H

#include "Base/ueResource_Private.h"
#include "Base/Containers/ueHashMap.h"

struct ioPackage;
struct glTextureBuffer;

struct gxFont : ueResource
{
	static const u32 TYPE_ID = UE_BE_4BYTE_SYMBOL('f','o','n','t');

	gxFont();
	~gxFont();

	u32 m_sizeInPixels;		//!< Size of the font in pixels
	u32 m_widthInPixels;	//!< Width of the font in pixels (only valid for fixed width fonts)
	u32 m_options;			//!< See gxFontFlags
	ueHashMap<u32, gxFontGlyph> m_glyphs; //!< Glyphs indexed by unicode codes

	glTextureBuffer* m_tb;	//!< Texture storing all glyphs

	// Used when loading

	ioPackage* m_package;
};

#endif // GX_FONT_PRIVATE_H
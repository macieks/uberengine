#ifndef GX_FONT_H
#define GX_FONT_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Base/ueResource.h"
#include "GraphicsExt/gxFont_Shared.h"

UE_DEFINE_RESOURCE_TYPE(gxFont, UE_BE_4BYTE_SYMBOL('f','o','n','t'));

//! Gets whether font is fixed width
ueBool gxFont_IsFixedWidth(const gxFont* font);
//! Gets font width in pixels
u32 gxFont_GetWidthInPixels(const gxFont* font);
//! Gets font size in pixels
u32 gxFont_GetSizeInPixels(const gxFont* font);
//! Gets glyph for a given code; returns NULL if not found
const gxFontGlyph* gxFont_GetGlyphForCode(gxFont* font, u32 code);

void gxFontMgr_Startup(ueAllocator* allocator, const char* config);
void gxFontMgr_Shutdown();

// @}

#endif // GX_FONT_H

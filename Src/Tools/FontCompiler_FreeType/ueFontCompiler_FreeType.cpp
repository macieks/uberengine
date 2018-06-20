#include "FontCompiler_Common/ueToolFont.h"
#include "GL/glLib_Shared.h"

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library s_library;

bool glUtils_FormatRequiresPos2Dimensions(glBufferFormat format)
{
	return glUtils_Is4x4CompressedFormat(format);
}

bool GenerateFont(ueToolFont& font, ueToolFont::BuildSettings& settings, const ueAssetParams& params)
{
	FT_Error error = FT_Init_FreeType(&s_library);
	if (error)
	{
		ueLogE("Failed to init FreeType library.");
		return false;
	}

	// Load face

	FT_Face face;
	error = FT_New_Face(s_library, font.m_sourcePath.c_str(), 0, &face);
	switch (error)
	{
		case 0:
			break;
		case FT_Err_Unknown_File_Format:
			ueLogE("Unknown font file format of (source path = '%s').", font.m_sourcePath.c_str());
			return false;
		default:
			ueLogE("Failed to load font face from (source path = '%s').", font.m_sourcePath.c_str());
			return false;
	}

	// Set charset

	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	if (error)
	{
		ueLogE("Font doesn't support UNICODE charmap (source path = '%s').", font.m_sourcePath.c_str());
		return false;
	}

	// Set pixel size

	error = FT_Set_Pixel_Sizes(
		face,
		0, // Width in pixels (0 - same as height)
		settings.m_sizeInPixels); // Height in pixels
	if (error)
	{
		ueLogE("Failed to set font size to %d.", settings.m_sizeInPixels);
		return false;
	}

	// Build glyph table

	std::vector<ueTextureRectPacker::Rect> glyphs(settings.m_codes.size());
	for (u32 i = 0; i < settings.m_codes.size(); i++)
	{
		const u32 code = settings.m_codes[i];

		error = FT_Load_Char(face, code, FT_LOAD_RENDER /* | FT_LOAD_MONOCHROME */ );
		if (error)
		{
			ueLogE("Failed to generate font character (Unicode value = %u).", code);
			return false;
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		FT_Bitmap* bitmap = &glyphSlot->bitmap;

		ueTextureRectPacker::Rect& glyph = glyphs[i];
		glyph.m_userData = &(settings.m_codes[i]);
		glyph.x = 0;
		glyph.y = 0;
		glyph.w = bitmap->width;
		glyph.h = bitmap->rows;
	}

	// Determine optimal characters placement

	const u32 numLayers = (font.m_packIntoSeparateChannels && settings.m_packIntoSeparateChannels) ? glUtils_GetFormatBytes(settings.m_textureFormat) : 1;
	const bool pow2Texture = glUtils_FormatRequiresPos2Dimensions(settings.m_textureFormat) || settings.m_makePow2Texture;

	const u32 spaceBetweenChars = 4;

	u32 textureWidth;
	u32 textureHeight;
	ueTextureRectPacker::Solve(spaceBetweenChars, numLayers, pow2Texture, glyphs, textureWidth, textureHeight);

	if (!settings.m_packIntoSeparateChannels)
	{
		const u32 alphaChannelIndex = glUtils_GetAlphaChannelIndex(settings.m_textureFormat);

		for (u32 i = 0; i < glyphs.size(); i++)
			glyphs[i].m_layer = alphaChannelIndex;
	}

	font.m_textureFormat = settings.m_textureFormat;
	font.m_packIntoSeparateChannels = settings.m_packIntoSeparateChannels;
	font.m_sizeInPixels = settings.m_sizeInPixels;
	font.m_options = settings.m_options;
	font.m_texture.Create2D(textureWidth, textureHeight, settings.m_numTextureLevels, settings.m_textureFormat);
//	font.m_texture.FillColor(ueColor32(255, 255, 255, 0));
	font.m_texture.m_enableAutoMips = false;

	// Generate & copy characters into appropriate bytes

	const f32 oneOverWidthF = 1.0f / (f32) textureWidth;
	const f32 oneOverHeightF = 1.0f / (f32) textureHeight;

	const f32 texelOffset = 0.0f;

	u32 rgbaPos[4];
	glUtils_GetChannelIndices(font.m_texture.m_format, rgbaPos[0], rgbaPos[1], rgbaPos[2], rgbaPos[3]);

	for (u32 i = 0; i < glyphs.size(); i++)
	{
		const ueTextureRectPacker::Rect& srcGlyph = glyphs[i];
		const u32 code = *(u32*) srcGlyph.m_userData;

		error = FT_Load_Char(face, code, FT_LOAD_RENDER /* | FT_LOAD_MONOCHROME */ );
		UE_ASSERT(!error);

		FT_GlyphSlot glyphSlot = face->glyph;
		FT_Bitmap* bitmap = &glyphSlot->bitmap;

		gxFontGlyph glyph;
#if 1
		glyph.m_channelMask = 255 << (8 * (3 - srcGlyph.m_layer));
#else
		glyph.m_channelMask = 255 << (8 * rgbaPos[srcGlyph.m_layer]);
#endif

		glyph.m_left = ((f32) srcGlyph.x + texelOffset) * oneOverWidthF;
		glyph.m_top = ((f32) srcGlyph.y + texelOffset) * oneOverHeightF;
		glyph.m_right = ((f32) (srcGlyph.x + srcGlyph.w) + texelOffset) * oneOverWidthF;
		glyph.m_bottom = ((f32) (srcGlyph.y + srcGlyph.h) + texelOffset) * oneOverHeightF;
		glyph.m_widthInPixels = (f32) bitmap->width;
		glyph.m_heightInPixels = (f32) bitmap->rows;
		glyph.m_offsetX = (f32) glyphSlot->bitmap_left;
		glyph.m_offsetY = (f32) glyphSlot->bitmap_top;
		glyph.m_advanceX = (f32) glyphSlot->advance.x / 64.0f; // >> 6);

		font.m_glyphs[code] = glyph;

		font.CopyFrom(srcGlyph, bitmap->buffer, bitmap->pitch);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(s_library);
	return true;
}
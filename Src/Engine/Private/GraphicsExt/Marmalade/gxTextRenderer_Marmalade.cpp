#include "Base/ueBase.h"
#include "Utils/utLocales.h"
#include "GL/glLib.h"
#include "GX/gxFont.h"
#include "GX/gxTextRenderer.h"

struct gxTextRendererData
{
	ueAllocator* m_allocator;

	ueResourceHandle<gxFont> m_defaultFont;

	ueMat44 m_worldViewProj;

	u32 m_numBufferedChars;
	u32 m_maxBufferedChars;
	CIwSVec3* m_pos;
	CIwColour* m_color;
	CIwSVec2* m_uv;

	u32 m_utf32TempBuffer[2048];
};

static gxTextRendererData* s_data = NULL;

void gxFontRenderer_Flush(glCtx* ctx, gxText& desc);

void gxTextRenderer_Startup(gxTextRendererStartupParams* params)
{
	ueAssert(!s_data);

	s_data = ueNew<gxTextRendererData>(params->m_stackAllocator);
	ueAssert(s_data);
	s_data->m_allocator = params->m_stackAllocator;

	// Create vertex buffer

	s_data->m_maxBufferedChars = params->m_charBufferSize;
	const u32 maxVerts = s_data->m_maxBufferedChars * 4;

	s_data->m_numBufferedChars = 0;
	s_data->m_maxBufferedChars = params->m_charBufferSize;
	s_data->m_pos = (CIwSVec3*) s_data->m_allocator->Alloc(sizeof(CIwSVec3) * maxVerts);
	ueAssert(s_data->m_pos);
	s_data->m_color = (CIwColour*) s_data->m_allocator->Alloc(sizeof(CIwColour) * maxVerts);
	ueAssert(s_data->m_color);
	s_data->m_uv = (CIwSVec2*) s_data->m_allocator->Alloc(sizeof(CIwSVec2) * maxVerts);
	ueAssert(s_data->m_uv);

	// Load default font

	if (params->m_defaultFontName)
	{
		s_data->m_defaultFont.SetByName(params->m_defaultFontName, UE_TRUE);
		ueAssert(s_data->m_defaultFont.IsReady());
	}
}

void gxTextRenderer_Shutdown()
{
	ueAssert(s_data);
	
	s_data->m_defaultFont.Release();
	s_data->m_allocator->Free(s_data->m_uv);
	s_data->m_allocator->Free(s_data->m_color);
	s_data->m_allocator->Free(s_data->m_pos);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool gxTextRenderer_GetUTF32Buffer(const gxText* desc, const u32*& buffer, u32& numChars)
{
	ueAssert(!desc->m_utf32Buffer || !desc->m_utf8Buffer);

	if (desc->m_utf32Buffer)
	{
		buffer = desc->m_utf32Buffer;
		if (desc->m_bufferSize > 0)
			numChars = desc->m_bufferSize;
		else
		{
			numChars = 0;
			while (buffer[numChars])
				numChars++;
		}
	}
	else
	{
		u32 utf8BufferSize;
		if (desc->m_bufferSize > 0)
			utf8BufferSize = desc->m_bufferSize;
		else
			utf8BufferSize = ueStrLen(desc->m_utf8Buffer);

		numChars = UE_ARRAY_SIZE(s_data->m_utf32TempBuffer);
		if (!ueStrUTF8ToUTF32((const u8*) desc->m_utf8Buffer, utf8BufferSize, s_data->m_utf32TempBuffer, numChars))
		{
			ueLogE("Failed to convert string from UTF8 to UTF32 (UTF8 string = '%s').\n", desc->m_utf8Buffer);
			return UE_FALSE;
		}

		buffer = s_data->m_utf32TempBuffer;
	}

	return UE_TRUE;
}

void gxTextRenderer_CalcDimensions(gxText* desc, f32& width, f32& height)
{
	ueAssert(s_data);

	if (!desc->m_font)
	{
		desc->m_font = *s_data->m_defaultFont;
		ueAssert(desc->m_font);
	}

	// Get buffer in UTF-32

	u32 numChars;
	const u32* buffer;
	if (!gxTextRenderer_GetUTF32Buffer(desc, buffer, numChars) || numChars == 0)
	{
		width = height = 0;
		return;
	}

	// Calculate text dimensions

	const f32 rowHeight = (f32) desc->m_font->m_sizeInPixels;

	f32 minX = F32_MAX;
	f32 maxX = F32_MIN;
	height = rowHeight;

	f32 x = 0;

	for (u32 i = 0; i < numChars; i++)
	{
		const u32 c = buffer[i];

		// Handle special chars

		switch (c)
		{
		case '\n':
			x = 0;
			height += rowHeight;
			break;
		case '\t':
		case '\r':
			continue;
		}

		// Process char

		const gxFontGlyph* glyph = gxFont_GetGlyphForCode(desc->m_font, c);
		if (!glyph)
			continue;

		const f32 left = x + glyph->m_offsetX;
		const f32 right = left + glyph->m_widthInPixels;

		minX = ueMin(minX, left);
		maxX = ueMax(maxX, right);

		// Proceed to next char

		x += glyph->m_advanceX;
	}

	if (maxX < minX)
		minX = maxX = 0.0f;

	width = (maxX - minX) * desc->m_scaleX;
	height *= desc->m_scaleY;
}

void gxTextRenderer_Flush(glCtx* ctx, gxText* desc)
{
	if (s_data->m_numBufferedChars == 0)
		return;

	// Set data streams

	const u32 numVerts = s_data->m_numBufferedChars * 4;

	IwGxSetVertStream(s_data->m_pos, numVerts);
	IwGxSetUVStream(s_data->m_uv);
	IwGxSetColStream(s_data->m_color, numVerts);

	// Set material

	CIwMaterial* material = glTextureBuffer_GetIwMaterial(desc->m_font->m_tb);
	IwGxSetMaterial(material);

	// Draw

	IwGxDrawPrims(IW_GX_QUAD_LIST, NULL, numVerts);
	IwGxFlush();

	// Reset rendering buffer

	s_data->m_numBufferedChars = 0;
}

void gxTextRenderer_Draw(glCtx* ctx, gxText* desc)
{
	ueAssert(s_data);

	if (!desc->m_font)
	{
		desc->m_font = *s_data->m_defaultFont;
		ueAssert(desc->m_font);
	}

	// Get buffer in UTF-32

	u32 numChars;
	const u32* buffer;
	if (!gxTextRenderer_GetUTF32Buffer(desc, buffer, numChars))
		return;

	// Process all chars

	const f32 rowHeight = (f32) desc->m_font->m_sizeInPixels * desc->m_scaleY;
	CIwColour colorAsShaderAttr;
	colorAsShaderAttr = desc->m_color.AsShaderAttr();

	f32 x = desc->m_x;
	f32 y = desc->m_y + rowHeight;

	const int16 zCoord = GL_TO_MARMALADE_COORD(2.0f);

	for (u32 i = 0; i < numChars; i++)
	{
		const u32 c = buffer[i];

		// Handle special chars

		switch (c)
		{
		case '\n':
			x = desc->m_x;
			y += rowHeight;
			break;
		case '\t':
		case '\r':
			continue;
		}

		// Get glyph

		const gxFontGlyph* glyph = gxFont_GetGlyphForCode(desc->m_font, c);
		if (!glyph)
			continue;

		const int16 left = GL_TO_MARMALADE_COORD(x + glyph->m_offsetX * desc->m_scaleX);
		const int16 right = left + GL_TO_MARMALADE_COORD(glyph->m_widthInPixels * desc->m_scaleX);
		const int16 top = GL_TO_MARMALADE_COORD(y - glyph->m_offsetY * desc->m_scaleY);
		const int16 bottom = top + GL_TO_MARMALADE_COORD(glyph->m_heightInPixels * desc->m_scaleY);

		// Flush if needed

		if (s_data->m_numBufferedChars == s_data->m_maxBufferedChars)
			gxTextRenderer_Flush(ctx, desc);

		// Allocate new 4 vertices

		CIwSVec3* pos = &s_data->m_pos[s_data->m_numBufferedChars << 2];
		CIwColour* color = &s_data->m_color[s_data->m_numBufferedChars << 2];
		CIwSVec2* uv = &s_data->m_uv[s_data->m_numBufferedChars << 2];
		s_data->m_numBufferedChars++;

		pos[0].x = left;
		pos[0].y = top;
		pos[1].x = right;
		pos[1].y = top;
		pos[2].x = right;
		pos[2].y = bottom;
		pos[3].x = left;
		pos[3].y = bottom;

		uv[0].x = GL_TO_MARMALADE_UV(glyph->m_left);
		uv[0].y = GL_TO_MARMALADE_UV(glyph->m_top);
		uv[1].x = GL_TO_MARMALADE_UV(glyph->m_right);
		uv[1].y = GL_TO_MARMALADE_UV(glyph->m_top);
		uv[2].x = GL_TO_MARMALADE_UV(glyph->m_right);
		uv[2].y = GL_TO_MARMALADE_UV(glyph->m_bottom);
		uv[3].x = GL_TO_MARMALADE_UV(glyph->m_left);
		uv[3].y = GL_TO_MARMALADE_UV(glyph->m_bottom);

		for (u32 v = 0; v < 4; v++)
		{
			color[v] = colorAsShaderAttr;
			pos[v].z = zCoord;
		}

		// Proceed to next char

		x += glyph->m_advanceX * desc->m_scaleX;
	}

	gxTextRenderer_Flush(ctx, desc);
}

gxFont*	gxTextRenderer_GetDefaultFont()
{
	ueAssert(s_data);
	return *s_data->m_defaultFont;
}
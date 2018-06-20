#include "Base/ueBase.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "Graphics/glVertexBufferFactory.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxFont_Private.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTextRenderer.h"

struct gxFontVertex
{
	f32_2 m_pos;
	f32_2 m_tex;
	u32 m_color;
	u32 m_channelMask;
};

struct gxTextRendererData
{
	ueAllocator* m_allocator;

	gxProgram m_program;

	ueResourceHandle<gxFont> m_defaultFont;

	ueMat44 m_worldViewProj;

	glStreamFormat* m_SF;

	u32 m_numBufferedChars;
	u32 m_maxBufferedChars;
	gxFontVertex* m_bufferedVerts;

	glVertexBufferFactory m_VB;
	glIndexBuffer* m_IB;

	u32 m_utf32TempBuffer[2048];
};

static gxTextRendererData* s_data = NULL;

void gxFontRenderer_Flush(glCtx* ctx, gxText& desc);

void gxTextRenderer_Startup(gxTextRendererStartupParams* params)
{
	UE_ASSERT(!s_data);

	s_data = ueNew<gxTextRendererData>(params->m_stackAllocator);
	UE_ASSERT(s_data);
	s_data->m_allocator = params->m_stackAllocator;

	// Create vertex buffer

	const u32 vbSize = sizeof(gxFontVertex) * params->m_charBufferSize * 4;

	s_data->m_numBufferedChars = 0;
	s_data->m_maxBufferedChars = params->m_charBufferSize;
	s_data->m_bufferedVerts = (gxFontVertex*) s_data->m_allocator->Alloc(vbSize);
	UE_ASSERT(s_data->m_bufferedVerts);

	// Create vertex buffer (factory)

	s_data->m_VB.Init(vbSize);

	// Create (dummy) index buffer

	glIndexBufferDesc ibDesc;
	ibDesc.m_indexSize = sizeof(u16);
	ibDesc.m_numIndices = params->m_charBufferSize * 6;
	s_data->m_IB = glIndexBuffer_Create(&ibDesc);
	UE_ASSERT(s_data->m_IB);

	u16* ibData = (u16*) glIndexBuffer_Lock(s_data->m_IB);
	UE_ASSERT(ibData);
	for (u32 i = 0; i < params->m_charBufferSize; i++)
	{
		const u32 index = i * 6;
		const u16 firstVertexIndex = i * 4;

		ibData[index] = firstVertexIndex;
		ibData[index + 1] = firstVertexIndex + 1;
		ibData[index + 2] = firstVertexIndex + 2;

		ibData[index + 3] = firstVertexIndex;
		ibData[index + 4] = firstVertexIndex + 2;
		ibData[index + 5] = firstVertexIndex + 3;
	}
	glIndexBuffer_Unlock(s_data->m_IB);

	// Load the shaders

	s_data->m_program.Create(params->m_VSName, params->m_FSName);

	// Load default font

	if (params->m_defaultFontName)
	{
		s_data->m_defaultFont.SetByName(params->m_defaultFontName);
	}

	// Init vertex format

	const glVertexElement vfElems[] = 
	{
		{glSemantic_Position, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(gxFontVertex, m_pos)},
		{glSemantic_TexCoord, 0, ueNumType_F32, 2, UE_FALSE, UE_OFFSET_OF(gxFontVertex, m_tex)},
		{glSemantic_Color, 0, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(gxFontVertex, m_color)},
		{glSemantic_Color, 1, ueNumType_U8, 4, UE_TRUE, UE_OFFSET_OF(gxFontVertex, m_channelMask)}
	};
 
	glStreamFormatDesc vfDesc;
	vfDesc.m_stride = sizeof(gxFontVertex);
	vfDesc.m_numElements = UE_ARRAY_SIZE(vfElems);
	vfDesc.m_elements = vfElems;
	s_data->m_SF = glStreamFormat_Create(&vfDesc);
	UE_ASSERT(s_data->m_SF);
}

void gxTextRenderer_Shutdown()
{
	UE_ASSERT(s_data);
	
	s_data->m_program.Destroy();
	s_data->m_defaultFont.Release();
	glStreamFormat_Destroy(s_data->m_SF);
	glIndexBuffer_Destroy(s_data->m_IB);
	s_data->m_VB.Deinit();
	s_data->m_allocator->Free(s_data->m_bufferedVerts);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool gxTextRenderer_GetUTF32Buffer(const gxText* desc, const u32*& buffer, u32& numChars)
{
	UE_ASSERT(!desc->m_utf32Buffer || !desc->m_utf8Buffer);

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
	UE_ASSERT(s_data);

	if (!desc->m_font)
	{
		desc->m_font = *s_data->m_defaultFont;
		UE_ASSERT(desc->m_font);
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

	width = (maxX - minX) * desc->m_scaleX;
	height *= desc->m_scaleY;
}

void gxTextRenderer_Flush(glCtx* ctx, gxText* desc)
{
	if (s_data->m_numBufferedChars == 0)
		return;

	// Set program

	glCtx_SetProgram(ctx, s_data->m_program.GetProgram());

	// Set up render states

	glCtx_SetBlending(ctx, TRUE);
	glCtx_SetBlendFunc(ctx, glBlendingFunc_SrcAlpha, glBlendingFunc_InvSrcAlpha);
	glCtx_SetAlphaTest(ctx, UE_FALSE);
	glCtx_SetDepthTest(ctx, UE_FALSE);
	glCtx_SetDepthWrite(ctx, UE_FALSE);
	glCtx_SetCullMode(ctx, glCullMode_None);

	// Set shader constants

	glSamplerParams samplerParams;
	samplerParams.m_addressU = glTexAddr_Clamp;
	samplerParams.m_addressV = glTexAddr_Clamp;
	if (desc->m_scaleX == 1.0f && desc->m_scaleY == 1.0f) // No need to filter if scale is 1
	{
		samplerParams.m_minFilter = glTexFilter_Nearest;
		samplerParams.m_magFilter = glTexFilter_Nearest;
		samplerParams.m_mipFilter = glTexFilter_Nearest;
	}

	glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, desc->m_font->m_tb, &samplerParams);

	glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldViewProj, &s_data->m_worldViewProj);

	// Draw

	const u32 vbChunkSize = sizeof(gxFontVertex) * 4 * s_data->m_numBufferedChars;
	glVertexBufferFactory::LockedChunk vbChunk;
	if (s_data->m_VB.Allocate(vbChunkSize, &vbChunk))
	{
		ueMemCpy(vbChunk.m_data, s_data->m_bufferedVerts, vbChunkSize);
		s_data->m_VB.Unlock(&vbChunk);

		glCtx_SetStream(ctx, 0, vbChunk.m_VB, s_data->m_SF, vbChunk.m_offset);
		glCtx_SetIndices(ctx, s_data->m_IB);

		glCtx_DrawIndexed(ctx, glPrimitive_TriangleList, 0, 0, s_data->m_numBufferedChars * 4, 0, s_data->m_numBufferedChars * 6);
	}

	s_data->m_numBufferedChars = 0;
}

void gxTextRenderer_Draw(glCtx* ctx, f32 x, f32 y, const char* utf8Buffer, ueColor32 color)
{
	gxText text;
	text.m_utf8Buffer = utf8Buffer;
	text.m_x = x;
	text.m_y = y;
	text.m_color = color;

	gxTextRenderer_Draw(ctx, &text);
}

void gxTextRenderer_Draw(glCtx* ctx, gxText* desc)
{
	UE_ASSERT(s_data);

	if (!desc->m_font)
	{
		desc->m_font = *s_data->m_defaultFont;
		UE_ASSERT(desc->m_font);
	}

	// Get buffer in UTF-32

	u32 numChars;
	const u32* buffer;
	if (!gxTextRenderer_GetUTF32Buffer(desc, buffer, numChars))
		return;

	// Calculate world-view-proj transform

	const glRenderGroupDesc* rbgDesc = glRenderGroup_GetDesc( glCtx_GetCurrentRenderGroup(ctx) );

	s_data->m_worldViewProj.SetOrthoOffCenter(0, (f32) rbgDesc->m_width, 0, (f32) rbgDesc->m_height, 0.0f, 1.0f, glCtx_IsRenderingToTexture(ctx));
	if (glDevice_GetCaps()->m_hasHalfPixelOffset)
	{
		ueMat44 halfTexelOffsetTransform;
		halfTexelOffsetTransform.SetTranslation(-0.5f / (f32) rbgDesc->m_width, -0.5f / (f32) rbgDesc->m_height, 0.0f);
		s_data->m_worldViewProj *= halfTexelOffsetTransform;
	}
	s_data->m_worldViewProj *= *desc->m_transform;

	// Process all chars

	const f32 rowHeight = (f32) desc->m_font->m_sizeInPixels * desc->m_scaleY;
	const u32 color = desc->m_color.AsShaderAttr();

	f32 x = desc->m_x;
	f32 y = desc->m_y + rowHeight;

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

		const f32 left = x + glyph->m_offsetX * desc->m_scaleX;
		const f32 right = left + glyph->m_widthInPixels * desc->m_scaleX;
		const f32 top = y - glyph->m_offsetY * desc->m_scaleY;
		const f32 bottom = top + glyph->m_heightInPixels * desc->m_scaleY;

		// Flush if needed

		if (s_data->m_numBufferedChars == s_data->m_maxBufferedChars)
			gxTextRenderer_Flush(ctx, desc);

		// Allocate new 4 vertices

		gxFontVertex* verts = &s_data->m_bufferedVerts[s_data->m_numBufferedChars << 2];
		s_data->m_numBufferedChars++;

		verts[0].m_pos.Set(left, top);
		verts[1].m_pos.Set(right, top);
		verts[2].m_pos.Set(right, bottom);
		verts[3].m_pos.Set(left, bottom);

		verts[0].m_tex.Set(glyph->m_left, glyph->m_top);
		verts[1].m_tex.Set(glyph->m_right, glyph->m_top);
		verts[2].m_tex.Set(glyph->m_right, glyph->m_bottom);
		verts[3].m_tex.Set(glyph->m_left, glyph->m_bottom);

		for (u32 v = 0; v < 4; v++)
		{
			verts[v].m_color = color;
			verts[v].m_channelMask = glyph->m_channelMask;
		}

		// Proceed to next char

		x += glyph->m_advanceX * desc->m_scaleX;
	}

	gxTextRenderer_Flush(ctx, desc);
}

gxFont*	gxTextRenderer_GetDefaultFont()
{
	UE_ASSERT(s_data);
	return *s_data->m_defaultFont;
}
#include "Graphics/glLib_Private.h"

u32 glUtils_CalcPrimCount(glPrimitive primitive, u32 indexCount)
{
	switch (primitive)
	{
		case glPrimitive_LineList: return indexCount / 2;
		case glPrimitive_LineStrip: return indexCount - 1;
		case glPrimitive_TriangleList: return indexCount / 3;
		case glPrimitive_TriangleStrip:	return indexCount - 2;
		UE_INVALID_CASE(primitive);
	}
	return 0;
}

u32 glUtils_CalcTextureBufferSize(const glTextureBufferDesc* desc)
{
	u32 size = 0;
	for (u32 i = 0; i < desc->m_numLevels; i++)
		size += glUtils_CalcMipLevelSize(desc, i);
	return size;
}

u32 glUtils_CalcMipLevelSize(const glTextureBufferDesc* desc, u32 level)
{
	u32 levelWidth, levelHeight, levelDepth;
	glUtils_CalcMipLevelDimensions(desc->m_width, desc->m_height, desc->m_depth, level, levelWidth, levelHeight, levelDepth);

	return
		(desc->m_type == glTexType_Cube ? 6 : 1) *
		glUtils_CalcSurfaceSize(desc->m_type, levelWidth, levelHeight, levelDepth, desc->m_format) *
		desc->m_rt.m_MSAALevel;
}

ueBool glUtils_FillTexture(glTextureBuffer* tb, glTextureFillFunc fillFunc)
{
	UE_ASSERT( fillFunc );

	const glTextureBufferDesc& desc = tb->m_desc;

	UE_ASSERT( desc.m_flags & glTextureBufferFlags_IsTexture );
	UE_ASSERT( desc.m_type == glTexType_2D );
	UE_ASSERT( desc.m_format == glBufferFormat_B8G8R8A8 );

	for (u32 i = 0; i < desc.m_numLevels; i++)
	{
		u32 levelWidth, levelHeight, levelDepth;
		glUtils_CalcMipLevelDimensions(desc.m_width, desc.m_height, desc.m_depth, i, levelWidth, levelHeight, levelDepth);

		glTextureData locked;
		if (!glTextureBuffer_Lock(tb, &locked, 0, i, glBufferLockFlags_Write, NULL))
			return UE_FALSE;

		u8* argb = (u8*) locked.m_data;
		for (u32 y = 0; y < levelHeight; y++)
		{
			for (u32 x = 0; x < levelWidth; x++)
				fillFunc(i, levelWidth, levelHeight, levelDepth, x, y, 0, *(u32*) &argb[x << 2]);

			argb += locked.m_rowPitch;
		}

		glTextureBuffer_Unlock(tb, 0, i);
	}
	return UE_TRUE;
}

void glUtils_SetMapSizeConstantValue(glTextureBuffer* tb, ueVec4& value)
{
	value.Set(
		(f32) tb->m_desc.m_width,
		(f32) tb->m_desc.m_height,
		1.0f / (f32) tb->m_desc.m_width,
		1.0f / (f32) tb->m_desc.m_height);
}

void glUtils_ListSupportedFormats()
{
	const glDevice* device = glDevice_Get();

	ueLogI("Supported buffer formats:");
	for (u32 i = 0; i < glBufferFormat_MAX; i++)
	{
		const glCaps::FormatInfo& info = device->m_caps.m_formatInfos[i];

		if (!info.m_supportedAsRTOnly &&
			!info.m_supportedAsTextureOnly &&
			!info.m_supportedAsRTAndTexture &&
			!info.m_supportsFiltering &&
			!info.m_supportsBlending &&
			!info.m_supportsAutoMips)
			continue;

		ueLog(" [%2u] %13s: rt: %s tex: %s rt+tex: %s filtering: %s blending: %s ueto-mips: %s\n",
			i, glBufferFormat_ToString((glBufferFormat) i),
			info.m_supportedAsRTOnly ? "YES" : "NO ",
			info.m_supportedAsTextureOnly ? "YES" : "NO ",
			info.m_supportedAsRTAndTexture ? "YES" : "NO ",
			info.m_supportsFiltering ? "YES" : "NO ",
			info.m_supportsBlending ? "YES" : "NO ",
			info.m_supportsAutoMips ? "YES" : "NO ");
	}

}
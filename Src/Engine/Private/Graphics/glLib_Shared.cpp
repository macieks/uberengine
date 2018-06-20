#include "Graphics/glLib_Shared.h"

void glUtils_CalcMipLevelDimensions(u32 width, u32 height, u32 depth, u32 level, u32& levelWidth, u32& levelHeight, u32& levelDepth)
{
	levelWidth = ueMax(1U, width >> level);
	levelHeight = ueMax(1U, height >> level);
	levelDepth = ueMax(1U, depth >> level);
}

u32 glUtils_CalcSurfaceSize(glTexType type, u32 width, u32 height, u32 depth, glBufferFormat format)
{
	const u32 bitsPerPixel = glUtils_GetFormatBits(format);
	const u32 dimensionAlignment = glUtils_Is4x4CompressedFormat(format) ? 4 : 1;

	u32 numPixels = ueAlignPow2(width, dimensionAlignment);
	numPixels *= ueAlignPow2(height, dimensionAlignment);
	if (type == glTexType_3D)
		numPixels *= ueAlignPow2(depth, dimensionAlignment);

	return (numPixels * bitsPerPixel) >> 3;
}

ueBool glUtils_Is4x4CompressedFormat(glBufferFormat format)
{
	return
		format == glBufferFormat_DXT1 ||
		format == glBufferFormat_DXT1a ||
		format == glBufferFormat_DXT3 ||
		format == glBufferFormat_DXT5 ||
		format == glBufferFormat_DXTN ||
		format == glBufferFormat_PVRTC2_RGB ||
		format == glBufferFormat_PVRTC2_RGBA ||
		format == glBufferFormat_PVRTC4_RGB ||
		format == glBufferFormat_PVRTC4_RGBA;
}

u32 glUtils_CalcNumMipLevels(u32 width, u32 height, u32 depth)
{
	return 1UL + (u32) (ueLogF((f32) ueMax(ueMax(width, height), depth)) / ueLogF(2.0f));
}

void glUtils_CalcSurfaceRowParams(u32 width, u32 height, u32 depth, glBufferFormat format, u32& rowSize, u32& numRows)
{
	if (glUtils_Is4x4CompressedFormat(format))
	{
		const u32 blockSize = (format == glBufferFormat_DXT1 || format == glBufferFormat_DXT1a) ? 8 : 16;
		rowSize = (ueAlignPow2(width, 4U) >> 2) * blockSize;
		numRows = ueAlignPow2(height, 4U) >> 2;
	}
	else
	{
		const u32 pixelSize = glUtils_GetFormatBytes(format);
		rowSize = width * pixelSize;
		numRows = height;
	}
}

ueBool glUtils_ConvertToBGRA(u8* dest, const void* src, glBufferFormat srcFormat)
{
	switch (srcFormat)
	{
	case glBufferFormat_B8G8R8A8:
		*(u32*) dest = *(const u32*) src;
		break;
	case glBufferFormat_A8:
		dest[0] = dest[1] = dest[2] = dest[3] = *(u8*) src;
		break;
	default:
		return UE_FALSE;
	}

	return UE_TRUE;
}

ueBool glUtils_ConvertFromBGRA(void* dest, glBufferFormat destFormat, const u8* src)
{
	switch (destFormat)
	{
	case glBufferFormat_B8G8R8A8:
		*(u32*) dest = *(const u32*) src;
		break;
	case glBufferFormat_A8:
		*(u8*) dest = src[0];
		break;
	default:
		return UE_FALSE;
	}

	return UE_TRUE;
}

u32 glUtils_GetShaderConstantSize(glConstantType type)
{
	switch (type)
	{
		case glConstantType_Float: return sizeof(f32);
		case glConstantType_Float2: return sizeof(f32) * 2;
		case glConstantType_Float3: return sizeof(f32) * 3;
		case glConstantType_Float4: return sizeof(f32) * 4;
		case glConstantType_Float3x3: return sizeof(f32) * 3 * 3;
		case glConstantType_Float4x3: return sizeof(f32) * 4 * 3;
		case glConstantType_Float4x4: return sizeof(f32) * 4 * 4;
		case glConstantType_Bool: return sizeof(ueBool);
		case glConstantType_Bool2: return sizeof(ueBool) * 2;
		case glConstantType_Bool3: return sizeof(ueBool) * 3;
		case glConstantType_Bool4: return sizeof(ueBool) * 4;
		case glConstantType_Int: return sizeof(s32);
		case glConstantType_Int2: return sizeof(s32) * 2;
		case glConstantType_Int3: return sizeof(s32) * 3;
		case glConstantType_Int4: return sizeof(s32) * 4;
	}
	UE_ASSERT(0);
	return 0;
}

ueBool glUtils_ConvertFormat(void* dest, glBufferFormat destFormat, const void* src, glBufferFormat srcFormat)
{
	if (glUtils_Is4x4CompressedFormat(destFormat) || glUtils_Is4x4CompressedFormat(srcFormat))
		return UE_FALSE;

	u8 data[4];
	return glUtils_ConvertToBGRA(data, src, srcFormat) && glUtils_ConvertFromBGRA(dest, destFormat, data);
}

u32 glUtils_GetFormatBits(glBufferFormat format)
{
	switch (format)
	{
		case glBufferFormat_A8: return sizeof(u8) * 8;
		case glBufferFormat_A16: return sizeof(u16) * 8;
		case glBufferFormat_R5G6B5: return sizeof(u16) * 8;
		case glBufferFormat_B4G4R4A4:
		case glBufferFormat_R4G4B4A4: return sizeof(u16) * 8;
		case glBufferFormat_B5G5R5A1:
		case glBufferFormat_R5G5B5A1: return sizeof(u16) * 8;
		case glBufferFormat_R8G8B8:
		case glBufferFormat_B8G8R8: return 3 * sizeof(u8) * 8;
		case glBufferFormat_B8G8R8A8:
		case glBufferFormat_R8G8B8A8: return 4 * sizeof(u8) * 8;

		case glBufferFormat_D16: return sizeof(u16) * 8;
		case glBufferFormat_D24: return 3 * 8;
		case glBufferFormat_D24S8: return sizeof(u32) * 8;
		case glBufferFormat_D32: return sizeof(f32) * 8;

		case glBufferFormat_DXT1a:
		case glBufferFormat_DXT1: return 4;
		case glBufferFormat_DXT3:
		case glBufferFormat_DXT5:
		case glBufferFormat_DXTN: return 8;

		case glBufferFormat_PVRTC2_RGB:
		case glBufferFormat_PVRTC2_RGBA:
			return 2;
		case glBufferFormat_PVRTC4_RGB:
		case glBufferFormat_PVRTC4_RGBA:
			return 4;

		case glBufferFormat_R16F: return sizeof(u16) * 8;
		case glBufferFormat_R16G16F: return 2 * sizeof(u16) * 8;
		case glBufferFormat_R16G16B16A16F: return 4 * sizeof(u16) * 8;

		case glBufferFormat_R32F: return sizeof(f32) * 8;
		case glBufferFormat_R32G32F: return 2 * sizeof(f32) * 8;
		case glBufferFormat_R32G32B32A32F: return 4 * sizeof(f32) * 8;

		UE_INVALID_CASE(format)
	}
	return 0;
}

u32 glUtils_GetFormatBytes(glBufferFormat format)
{
	const u32 bits = glUtils_GetFormatBits(format);
	UE_ASSERT_MSG((bits & 7) == 0, "Do not use glUtils_GetFormatBytes for non byte-aligned formats");
	return bits >> 3;
}

ueBool glUtils_IsDepthFormat(glBufferFormat format)
{
	return
		format == glBufferFormat_D24S8 ||
		format == glBufferFormat_D16 ||
		format == glBufferFormat_D32 ||
		format == glBufferFormat_Depth;
}

ueBool glUtils_IsStencilFormat(glBufferFormat format)
{
	return
		format == glBufferFormat_D24S8;
}

ueBool glUtils_IsSampler(glConstantType type)
{
	return
		type == glConstantType_Sampler1D ||
		type == glConstantType_Sampler2D ||
		type == glConstantType_Sampler3D ||
		type == glConstantType_SamplerCube;
}

ueBool glUtils_IsFloat(glConstantType type)
{
	return
		type == glConstantType_Float ||
		type == glConstantType_Float2 ||
		type == glConstantType_Float3 ||
		type == glConstantType_Float4 ||
		type == glConstantType_Float3x3 ||
		type == glConstantType_Float4x3 ||
		type == glConstantType_Float4x4;
}

ueBool glUtils_IsInt(glConstantType type)
{
	return
		type == glConstantType_Int ||
		type == glConstantType_Int2 ||
		type == glConstantType_Int3 ||
		type == glConstantType_Int4;
}

ueBool glUtils_IsBool(glConstantType type)
{
	return
		type == glConstantType_Bool ||
		type == glConstantType_Bool2 ||
		type == glConstantType_Bool3 ||
		type == glConstantType_Bool4;
}

ueBool glUtils_HasAlpha(glBufferFormat format)
{
	switch (format)
	{
		case glBufferFormat_A8:
		case glBufferFormat_A16:
		case glBufferFormat_B4G4R4A4:
		case glBufferFormat_R4G4B4A4:
		case glBufferFormat_B5G5R5A1:
		case glBufferFormat_B8G8R8A8:
		case glBufferFormat_R8G8B8A8:

		case glBufferFormat_DXT1a:
		case glBufferFormat_DXT5:

		case glBufferFormat_PVRTC2_RGBA:
		case glBufferFormat_PVRTC4_RGBA:
		case glBufferFormat_R16G16B16A16F:
		case glBufferFormat_R32G32B32A32F:
			return UE_TRUE;
	}
	return UE_FALSE;
}

void glUtils_CopyTextureRect(glBufferFormat format, void* dst, u32 dstRowPitch, const ueRectI& dstRect, const void* src, u32 srcRowPitch, u32 numRows)
{
	const ueBool is4x4Compression = glUtils_Is4x4CompressedFormat(format);

	const u32 pixelSize = glUtils_GetFormatBits(format);

	const u32 startDstX = dstRect.m_left / (is4x4Compression ? 4 : 1);
	const u32 startDstY = dstRect.m_top / (is4x4Compression ? 4 : 1);

	u8* dstPtr = (u8*) dst + startDstY * dstRowPitch + startDstX * pixelSize / 8;
	u8* srcPtr = (u8*) src;

	for (u32 row = 0; row < numRows; row++)
	{
		ueMemCpy(dstPtr, srcPtr, srcRowPitch);
		dstPtr += dstRowPitch;
		srcPtr += srcRowPitch;
	}
}

void glUtils_MakeTextureBorder(glBufferFormat format, void* data, u32 width, u32 height, const ueRectI& _rect, u32 border)
{
	ueRectI rect = _rect;

	const u32 pixelSize = glUtils_GetFormatBytes(format);

	for (s32 i = 0; i < (s32) border; i++)
	{
		// Horizontal border

		if (rect.m_height + i < (s32) height)
		{
			const s32 above = (rect.m_top - i) > 0 ? (rect.m_top - i - 1) : ((s32) height - i - 1);
			const s32 below = (rect.GetBottom() + i) < (s32) height ? (rect.GetBottom() + i) : i;
			ueMemCpy(
				(u8*) data + (above * (s32) width + rect.m_left) * pixelSize,
				(u8*) data + (rect.m_top * (s32) width + rect.m_left) * pixelSize,
				rect.m_width * pixelSize);
			ueMemCpy(
				(u8*) data + (below * (s32) width + rect.m_left) * pixelSize,
				(u8*) data + ((rect.GetBottom() - 1) * (s32) width + rect.m_left) * pixelSize,
				rect.m_width * pixelSize);
		}

		// Vertical border

		if (rect.m_width + i < (s32) width)
		{
			const u8* src[2] =
			{
				(u8*) data + (rect.m_top * (s32) width + rect.m_left) * pixelSize,
				(u8*) data + (rect.m_top * (s32) width + rect.GetRight() - 1) * pixelSize,
			};

			const s32 toLeft = (rect.m_left - i) > 0 ? (rect.m_left - i - 1) : ((s32) width - i - 1);
			const s32 toRight = (rect.GetRight() + i < (s32) width) ? (rect.GetRight() + i) : i;
			u8* dst[2] =
			{
				(u8*) data + (rect.m_top * (s32) width + toLeft) * pixelSize,
				(u8*) data + (rect.m_top * (s32) width + toRight) * pixelSize,
			};

			const u32 rowPitch = width * pixelSize;

			for (s32 y = 0; y < rect.m_height; y++)
			{
				ueMemCpy(dst[0], src[0], pixelSize);
				ueMemCpy(dst[1], src[1], pixelSize);

				src[0] += rowPitch;
				src[1] += rowPitch;
				dst[0] += rowPitch;
				dst[1] += rowPitch;
			}
		}

		// TODO: Also write corner pixels
	}
}

u32 glUtils_GetAlphaChannelIndex(glBufferFormat format)
{
	u32 r, g, b, a;
	glUtils_GetChannelIndices(format, r, g, b, a);
	return a;
}

void glUtils_GetChannelIndices(glBufferFormat format, u32& r, u32& g, u32& b, u32& a)
{
	r = g = b = a = 0xFFFFFFFF;

#define RETURN_CHANNELS(format, redIndex, greenIndex, blueIndex, alphaIndex) \
	case glBufferFormat_##format: \
			r = redIndex; \
			g = greenIndex; \
			b = blueIndex; \
			a = alphaIndex; \
			return;

#define NO_CHANNEL 0xFFFFFFFF

	switch (format)
	{
		RETURN_CHANNELS(A8, NO_CHANNEL, NO_CHANNEL, NO_CHANNEL, 0)
		RETURN_CHANNELS(A16, NO_CHANNEL, NO_CHANNEL, NO_CHANNEL, 0)
		RETURN_CHANNELS(R5G6B5, 0, 1, 2, NO_CHANNEL)
		RETURN_CHANNELS(B5G6R5, 2, 1, 0, NO_CHANNEL)
		RETURN_CHANNELS(B4G4R4A4, 3, 2, 1, 0)
		RETURN_CHANNELS(R4G4B4A4, 0, 1, 2, 3)
		RETURN_CHANNELS(B5G5R5A1, 2, 1, 0, 3)
		RETURN_CHANNELS(B8G8R8, 0, 1, 2, NO_CHANNEL)
		RETURN_CHANNELS(R8G8B8, 0, 1, 2, NO_CHANNEL)
		RETURN_CHANNELS(B8G8R8A8, 2, 1, 0, 3)
		RETURN_CHANNELS(R8G8B8A8, 0, 1, 2, 3)
		RETURN_CHANNELS(R16G16B16A16F, 0, 1, 2, 3)
		RETURN_CHANNELS(R32G32B32A32F, 0, 1, 2, 3)
		UE_INVALID_CASE(format)
	}
}

ueBool glUtils_FormatRequiresSquareDimensions(glBufferFormat format)
{
	return
		format == glBufferFormat_PVRTC2_RGB ||
		format == glBufferFormat_PVRTC2_RGBA ||
		format == glBufferFormat_PVRTC4_RGB ||
		format == glBufferFormat_PVRTC4_RGBA;
}
#include "GL/Marmalade/glLib_Private_Marmalade.h"
#include "GL/glLib_Private.h"

void IwImageUtil_Downsize2x(CIwImage* dstImg, CIwImage* srcImg)
{
	// Note: if the width isn't even, then the last column won't be considered
	// And the same with height - if it's not even, then the last row won't be considered

	const u32 srcWidth = srcImg->GetWidth();
	const u32 srcHeight = srcImg->GetHeight();

	const u32 dstWidth = srcWidth / 2;
	const u32 dstHeight = srcHeight / 2;

	const u32 numComponents = CIwImage::GetByteDepth(srcImg->GetFormat());
	u8* dst = (u8*) s3eMalloc(dstWidth * dstHeight * numComponents);
	ueAssert(dst);

	u8* currDst = dst;
	const u32 dstPitch = numComponents * dstWidth;

	u8* currSrc = srcImg->GetTexels();
	const u32 srcPitch = srcImg->GetPitch();
	const u32 srcPadding = (srcWidth & 1) ? numComponents : 0;

	// Apply 2x2 downsizing filter

	for (u32 y = 0; y < dstHeight; y++)
	{
		u8* prevSrc = currSrc;

		for (u32 x = 0; x < dstWidth; x++)
		{
			for (u32 c = 0; c < numComponents; c++)
			{
				// Calculate average out of 4 pixels

				const u32 average =
				   ((u32) *(currSrc + c) +
					(u32) *(currSrc + c + numComponents) +
					(u32) *(currSrc + c + srcPitch) +
					(u32) *(currSrc + c + srcPitch + numComponents))
					>> 2;

				*(currDst + c) = (u8) average;
			}

			currSrc += numComponents * 2;
			currDst += numComponents;
		}

		currSrc = prevSrc + srcPitch * 2;
	}

	dstImg->SetFormat(srcImg->GetFormat());
	dstImg->SetWidth(dstWidth);
	dstImg->SetHeight(dstHeight);
	dstImg->SetPitch(dstWidth * numComponents);
	dstImg->SetOwnedBuffers(dst, NULL);
}

void IwImageUtil_Downsize2x(u8* pixels, u32& width, u32& height, u32& rowPitch, glBufferFormat format)
{
	ueAssert(!glUtils_Is4x4CompressedFormat(format));

	// Note: if the width isn't even, then the last column won't be considered
	// Similarly with height - if it's not even, then the last row won't be considered

	const u32 numComponents = glUtils_GetFormatBytes(format);

	const u32 srcWidth = width;
	const u32 srcHeight = height;
	const u32 srcRowPitch = rowPitch;

	const u32 dstWidth = srcWidth / 2;
	const u32 dstHeight = srcHeight / 2;
	const u32 dstRowPitch = dstWidth * numComponents;

	width = dstWidth;
	height = dstHeight;
	rowPitch = dstRowPitch;

	const u32 srcPitch = srcWidth * numComponents;

	// Apply 2x2 downsizing filter

	u8* src = pixels;
	u8* dst = pixels;
	for (u32 y = 0; y < dstHeight; y++)
	{
		u8* prevSrc = src;

		for (u32 x = 0; x < dstWidth; x++)
		{
			for (u32 c = 0; c < numComponents; c++)
			{
				// Calculate average out of 4 pixels

				const u32 average =
				   ((u32) *(src + c) +
					(u32) *(src + c + numComponents) +
					(u32) *(src + c + srcRowPitch) +
					(u32) *(src + c + srcRowPitch + numComponents))
					>> 2;

				*(dst + c) = (u8) average;
			}

			src += numComponents * 2;
			dst += numComponents;
		}

		src = prevSrc + srcPitch * 2;
	}
}

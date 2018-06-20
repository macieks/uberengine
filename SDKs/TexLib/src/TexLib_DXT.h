/*
 *	Below DXT compression code was taken from:
 *
 *	Real-Time DXT Compression
 *	May 20th 2006 J.M.P. van Waveren
 *	© 2006, Id Software, Inc.
 */

#pragma once

template <TexLib_Format format>
struct DXTBlockCreator
{
	inline void CreateBlock(const unsigned char* src, unsigned char* dst)
	{
		assert(!"abstract");
	}
};

inline unsigned short DXT_ColorTo565(const unsigned char* color)
{
	return ( ( color[2] >> 3 ) << 11 ) | ( ( color[1] >> 2 ) << 5 ) | ( color[0] >> 3 );
}

#define INSET_SHIFT 4 // inset the bounding box with ( range >> shift )

inline void DXT_GetMinMaxRGB(const unsigned char* colorBlock, unsigned char* restrict minColor, unsigned char* restrict maxColor)
{
	int i;
	unsigned char inset[3];
	minColor[0] = minColor[1] = minColor[2] = 255;
	maxColor[0] = maxColor[1] = maxColor[2] = 0;
	for ( i = 0; i < 16; i++ )
	{
		if ( colorBlock[i*4+0] < minColor[0] ) { minColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] < minColor[1] ) { minColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] < minColor[2] ) { minColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+0] > maxColor[0] ) { maxColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] > maxColor[1] ) { maxColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] > maxColor[2] ) { maxColor[2] = colorBlock[i*4+2]; }
	}
	inset[0] = ( maxColor[0] - minColor[0] ) >> INSET_SHIFT;
	inset[1] = ( maxColor[1] - minColor[1] ) >> INSET_SHIFT;
	inset[2] = ( maxColor[2] - minColor[2] ) >> INSET_SHIFT;
	minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255;
	minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255;
	minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255;
	maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0;
	maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0;
	maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0;
}

inline void DXT_GetMinMaxRGBA( const unsigned char* colorBlock, unsigned char* restrict minColor, unsigned char* restrict maxColor)
{
	int i;
	unsigned char inset[4];
	minColor[0] = minColor[1] = minColor[2] = minColor[3] = 255;
	maxColor[0] = maxColor[1] = maxColor[2] = maxColor[3] = 0;
	for ( i = 0; i < 16; i++ ) {
		if ( colorBlock[i*4+0] < minColor[0] ) { minColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] < minColor[1] ) { minColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] < minColor[2] ) { minColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+3] < minColor[3] ) { minColor[3] = colorBlock[i*4+3]; }
		if ( colorBlock[i*4+0] > maxColor[0] ) { maxColor[0] = colorBlock[i*4+0]; }
		if ( colorBlock[i*4+1] > maxColor[1] ) { maxColor[1] = colorBlock[i*4+1]; }
		if ( colorBlock[i*4+2] > maxColor[2] ) { maxColor[2] = colorBlock[i*4+2]; }
		if ( colorBlock[i*4+3] > maxColor[3] ) { maxColor[3] = colorBlock[i*4+3]; }
	}
	inset[0] = ( maxColor[0] - minColor[0] ) >> INSET_SHIFT;
	inset[1] = ( maxColor[1] - minColor[1] ) >> INSET_SHIFT;
	inset[2] = ( maxColor[2] - minColor[2] ) >> INSET_SHIFT;
	inset[3] = ( maxColor[3] - minColor[3] ) >> INSET_SHIFT;
	minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255;
	minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255;
	minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255;
	minColor[3] = ( minColor[3] + inset[3] <= 255 ) ? minColor[3] + inset[3] : 255;
	maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0;
	maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0;
	maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0;
	maxColor[3] = ( maxColor[3] >= inset[3] ) ? maxColor[3] - inset[3] : 0;
}

#define C565_5_MASK 0xF8 // 0xFF minus last three bits
#define C565_6_MASK 0xFC // 0xFF minus last two bits

inline void DXT_WriteRGBIndices(unsigned char* restrict dst, const unsigned char *colorBlock, const unsigned char* minColor, const unsigned char* maxColor)
{
	unsigned short colors[4][4];
	unsigned int result = 0;
	colors[0][0] = ( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 );
	colors[0][1] = ( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 );
	colors[0][2] = ( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 );
	colors[1][0] = ( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 );
	colors[1][1] = ( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 );
	colors[1][2] = ( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 );
	colors[2][0] = ( 2 * colors[0][0] + 1 * colors[1][0] ) / 3;
	colors[2][1] = ( 2 * colors[0][1] + 1 * colors[1][1] ) / 3;
	colors[2][2] = ( 2 * colors[0][2] + 1 * colors[1][2] ) / 3;
	colors[3][0] = ( 1 * colors[0][0] + 2 * colors[1][0] ) / 3;
	colors[3][1] = ( 1 * colors[0][1] + 2 * colors[1][1] ) / 3;
	colors[3][2] = ( 1 * colors[0][2] + 2 * colors[1][2] ) / 3;
	for ( int i = 15; i >= 0; i-- )
	{
		int c0 = colorBlock[i*4+0];
		int c1 = colorBlock[i*4+1];
		int c2 = colorBlock[i*4+2];
		int d0 = abs( colors[0][0] - c0 ) + abs( colors[0][1] - c1 ) + abs( colors[0][2] - c2 );
		int d1 = abs( colors[1][0] - c0 ) + abs( colors[1][1] - c1 ) + abs( colors[1][2] - c2 );
		int d2 = abs( colors[2][0] - c0 ) + abs( colors[2][1] - c1 ) + abs( colors[2][2] - c2 );
		int d3 = abs( colors[3][0] - c0 ) + abs( colors[3][1] - c1 ) + abs( colors[3][2] - c2 );
		int b0 = d0 > d3;
		int b1 = d1 > d2;
		int b2 = d0 > d2;
		int b3 = d1 > d3;
		int b4 = d2 > d3;
		int x0 = b1 & b2;
		int x1 = b0 & b3;
		int x2 = b0 & b4;
		result |= ( x2 | ( ( x0 | x1 ) << 1 ) ) << ( i << 1 );
	}
	*(unsigned int*) dst = result;
}

inline void DXT_WriteAlphaIndices(unsigned char* restrict dst, const unsigned char *colorBlock, const unsigned char minAlpha, const unsigned char maxAlpha )
{
	unsigned char indices[16];
	unsigned char alphas[8];
	alphas[0] = maxAlpha;
	alphas[1] = minAlpha;
	alphas[2] = ( 6 * maxAlpha + 1 * minAlpha ) / 7;
	alphas[3] = ( 5 * maxAlpha + 2 * minAlpha ) / 7;
	alphas[4] = ( 4 * maxAlpha + 3 * minAlpha ) / 7;
	alphas[5] = ( 3 * maxAlpha + 4 * minAlpha ) / 7;
	alphas[6] = ( 2 * maxAlpha + 5 * minAlpha ) / 7;
	alphas[7] = ( 1 * maxAlpha + 6 * minAlpha ) / 7;
	colorBlock += 3;
	for ( int i = 0; i < 16; i++ ) {
		int minDistance = INT_MAX;
		unsigned char a = colorBlock[i*4];
		for ( int j = 0; j < 8; j++ ) {
			int dist = abs( a - alphas[j] );
			if ( dist < minDistance ) {
				minDistance = dist; indices[i] = j;
			}
		}
	}
	*dst++ = (indices[ 0] >> 0) | (indices[ 1] << 3) | (indices[ 2] << 6);
	*dst++ = (indices[ 2] >> 2) | (indices[ 3] << 1) | (indices[ 4] << 4) | (indices[ 5] << 7);
	*dst++ = (indices[ 5] >> 1) | (indices[ 6] << 2) | (indices[ 7] << 5);
	*dst++ = (indices[ 8] >> 0) | (indices[ 9] << 3) | (indices[10] << 6);
	*dst++ = (indices[10] >> 2) | (indices[11] << 1) | (indices[12] << 4) | (indices[13] << 7);
	*dst++ = (indices[13] >> 1) | (indices[14] << 2) | (indices[15] << 5);
}

template <>
struct DXTBlockCreator<TexLib_Format_DXT1>
{
	inline void CreateBlock(const unsigned char* src, unsigned char* dst)
	{
		unsigned char minColor[3];
		unsigned char maxColor[3];
		DXT_GetMinMaxRGB(src, minColor, maxColor);

		*(unsigned short*) dst = DXT_ColorTo565(maxColor);
		*(unsigned short*) (dst + 2) = DXT_ColorTo565(minColor);
		DXT_WriteRGBIndices(dst + 4, src, minColor, maxColor);
	}
};

template <>
struct DXTBlockCreator<TexLib_Format_DXT5>
{
	inline void CreateBlock(const unsigned char* src, unsigned char* dst)
	{
		unsigned char minColor[4];
		unsigned char maxColor[4];
		DXT_GetMinMaxRGBA(src, minColor, maxColor);

		*dst = maxColor[3];
		*(dst + 1) = minColor[3];
		DXT_WriteAlphaIndices(dst, src, minColor[3], maxColor[3]);

		*(unsigned short*) (dst + 8) = DXT_ColorTo565(maxColor);
		*(unsigned short*) (dst + 10) = DXT_ColorTo565(minColor);
		DXT_WriteRGBIndices(dst + 12, src, minColor, maxColor);
	}
};

inline void DXT_Complete4x4Block(int filledWidth, int filledHeight, unsigned char* argb)
{
	// Fill in bottom part (excluding right-bottom corner)

	for (int y = filledHeight; y < 4; y++)
		for (int x = 0; x < filledWidth; x++)
			*((unsigned int*) argb + y * 4 + x) = *((unsigned int*) argb + y * (filledHeight - 1) + x);

	// Fill in right part (including right-bottom corner)

	for (int y = 0; y < 4; y++)
		for (int x = filledWidth; x < 4; x++)
			*((unsigned int*) argb + y * 4 + x) = *((unsigned int*) argb + y * 4 + filledWidth - 1);
}

template <typename ImageBlockProvider, TexLib_Format format>
bool ConvertToDXT(unsigned char* dst, int width, int height, ImageBlockProvider& provider)
{
	assert(width % 4 == 0);
	assert(height % 4 == 0);

	const int numBlocksX = (width + 3) / 4;
	const int numBlocksY = (height + 3) / 4;
	const int dstBlockStep = IsDXT1(format) ? 8 : 16;

	const int rightBlockWidth = width & 3;
	const int bottomBlockHeight = height & 3;

	const int widthWithoutPartialBlock = width & ~3;
	const int heightWithoutPartialBlock = height & ~3;

	unsigned char tempData[4 * 4 * 4];

	DXTBlockCreator<format> dxtBlockCreator;

	unsigned char* dstBlock = dst;
	for (int y = 0; y < heightWithoutPartialBlock; y += 4)
	{
		for (int x = 0; x < widthWithoutPartialBlock; x += 4)
		{
			if (!provider.GetBlock_ARGB(x, y, 4, 4, tempData))
				return false;
			dxtBlockCreator.CreateBlock(tempData, dstBlock);
			dstBlock += dstBlockStep;
		}

		// Handle right-most block

		if (width & 3)
		{
			if (!provider.GetBlock_ARGB(widthWithoutPartialBlock, y, rightBlockWidth, 4, tempData))
				return false;
			DXT_Complete4x4Block(rightBlockWidth, 4, tempData);
			dxtBlockCreator.CreateBlock(tempData, dstBlock);
			dstBlock += dstBlockStep;
		}
	}

	// Handle bottom row

	if (height & 3)
	{
		for (int x = 0; x < widthWithoutPartialBlock; x += 4)
		{
			if (!provider.GetBlock_ARGB(x, heightWithoutPartialBlock, 4, bottomBlockHeight, tempData))
				return false;
			DXT_Complete4x4Block(4, bottomBlockHeight, tempData);
			dxtBlockCreator.CreateBlock(tempData, dstBlock);
			dstBlock += dstBlockStep;
		}

		// Handle right-bottom block

		if (width & 3)
		{
			if (!provider.GetBlock_ARGB(widthWithoutPartialBlock, heightWithoutPartialBlock, rightBlockWidth, bottomBlockHeight, tempData))
				return false;
			DXT_Complete4x4Block(rightBlockWidth, bottomBlockHeight, tempData);
			dxtBlockCreator.CreateBlock(tempData, dstBlock);
			dstBlock += dstBlockStep;
		}
	}
	return true;
}
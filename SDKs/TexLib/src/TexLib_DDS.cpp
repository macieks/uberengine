#include "TexLib_Private.h"

typedef unsigned __int32 uint32;

static const uint32 FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' ');
static const uint32 FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
static const uint32 FOURCC_DXT2 = MAKEFOURCC('D', 'X', 'T', '2');
static const uint32 FOURCC_DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
static const uint32 FOURCC_DXT4 = MAKEFOURCC('D', 'X', 'T', '4');
static const uint32 FOURCC_DXT5 = MAKEFOURCC('D', 'X', 'T', '5');
static const uint32 FOURCC_RXGB = MAKEFOURCC('R', 'X', 'G', 'B');
static const uint32 FOURCC_ATI1 = MAKEFOURCC('A', 'T', 'I', '1');
static const uint32 FOURCC_ATI2 = MAKEFOURCC('A', 'T', 'I', '2');

static const uint32 FOURCC_A2XY = MAKEFOURCC('A', '2', 'X', 'Y');

static const uint32 FOURCC_DX10 = MAKEFOURCC('D', 'X', '1', '0');

static const uint32 DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U;

struct DDSPixelFormat
{
	uint32 size;
	uint32 flags;
	uint32 fourcc;
	uint32 bitcount;
	uint32 rmask;
	uint32 gmask;
	uint32 bmask;
	uint32 amask;
};

struct DDSCaps
{
	uint32 caps1;
	uint32 caps2;
	uint32 caps3;
	uint32 caps4;
};

struct DDSHeader10
{
	uint32 dxgiFormat;
	uint32 resourceDimension;
	uint32 miscFlag;
	uint32 arraySize;
	uint32 reserved;
};

struct DDSHeader
{
	uint32 fourcc;
	uint32 size;
	uint32 flags;
	uint32 height;
	uint32 width;
	uint32 pitch;
	uint32 depth;
	uint32 mipmapcount;
	uint32 reserved[11];
	DDSPixelFormat pf;
	DDSCaps caps;
	uint32 notused;
};

bool TexLib_LoadDDS(TexLib_Context* ctx)
{
	// Read texture header

	DDSHeader ddsHeader;
	if (!ctx->m_read(ctx, &ddsHeader, sizeof(DDSHeader)))
		return false;

	DDSHeader10 ddsHeader10;
	if ((ddsHeader.pf.fourcc == 0 || ddsHeader.pf.fourcc == FOURCC_DX10) && !ctx->m_read(ctx, &ddsHeader10, sizeof(DDSHeader10)))
		return false;

	// Extract relevant texture info

	TexLib_Header header;
	header.m_width = ddsHeader.width;
	header.m_height = ddsHeader.height;
	header.m_depth = ddsHeader.depth;
	header.m_numFaces = (ddsHeader.caps.caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) ? 6 : 1;
	header.m_numLevels = ddsHeader.mipmapcount;
	if (header.m_numLevels == 0)
#if 1
		header.m_numLevels = 1;
#else
	{
		int dim = max(max(header.m_width, header.m_height), header.m_depth);
		while (dim)
		{
			header.m_numLevels++;
			dim /= 2;
		}
	}
#endif
	header.m_hasColor = true;
	header.m_hasAlpha = ddsHeader.pf.fourcc != FOURCC_DXT1;
	switch (ddsHeader.pf.fourcc)
	{
		case FOURCC_DXT1: header.m_format = TexLib_Format_DXT1; break;
		case FOURCC_DXT3: header.m_format = TexLib_Format_DXT3; break;
		case FOURCC_DXT5: header.m_format = TexLib_Format_DXT5; break;
		default:
			return false;
	}

	if (!ctx->m_onHeader(ctx, &header))
		return false;

	// Read in all faces

	const int blockSize = header.m_format == TexLib_Format_DXT1 ? 8 : 16;
	const int numBlocksX = (header.m_width + 3) / 4;
	const int numBlocksY = (header.m_height + 3) / 4;
	const int rowSize = numBlocksX * blockSize;

	for (int i = 0; i < header.m_numFaces; i++)
	{
		for (int j = 0; j < header.m_numLevels; j++)
		{
			TexLib_LevelData data;
			if (!ctx->m_onBeginFaceLevel(ctx, &header, i, j, &data))
				return false;
			if (data.m_format != header.m_format)
				return false;

			if (data.m_rowPitch != rowSize)
			{
				for (int y = 0; y < numBlocksY; y++)
					if (!ctx->m_read(ctx, data.m_data + data.m_rowPitch * y, rowSize))
						return false;
			}
			else if (!ctx->m_read(ctx, data.m_data, data.m_dataSize))
				return false;

			ctx->m_onEndFaceLevel(ctx, &header, i, j);
		}
	}

	return true;
}
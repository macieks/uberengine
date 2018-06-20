#include "TexLib_Private.h"

struct ImageBlockProvider_8BitTGA : ImageBlockProvider_Base
{
	unsigned char* m_palette;
	unsigned char* m_indices;

	unsigned char* m_lines[4];
	int m_linesRead;

	bool Init(TexLib_Context* ctx, const TexLib_Header* header)
	{
		InitBase(ctx, header);

		m_palette = (unsigned char*) m_ctx->m_alloc(m_ctx, 256 * 3 + header->m_width * m_numComponents * (1 + 4));
		if (!m_palette)
			return false;
		m_indices = m_palette + 256 * 3;
		m_lines[0] = m_indices + header->m_width * m_numComponents;
		m_lines[1] = m_lines[0] + m_header->m_width * m_numComponents * 1;
		m_lines[2] = m_lines[0] + m_header->m_width * m_numComponents * 2;
		m_lines[3] = m_lines[0] + m_header->m_width * m_numComponents * 3;

		if (!m_read(m_ctx, m_palette, 256 * 3))
			return false;

		return true;
	}

	~ImageBlockProvider_8BitTGA()
	{
		m_ctx->m_free(m_ctx, m_palette);
	}

	inline bool GetBlock_ARGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;
		while (m_linesRead < bottom)
		{
			if (!m_read(m_ctx, m_indices, m_header->m_width * m_numComponents))
				return false;

			unsigned char* line = m_lines[m_linesRead & 3];
			for (int x = 0; x < m_header->m_width; x++)
			{
				line[0] = m_palette[m_indices[x] * 3 + 2];
				line[1] = m_palette[m_indices[x] * 3 + 1];
				line[2] = m_palette[m_indices[x] * 3 + 0];
				line += 3;
			}

			m_linesRead++;
		}

		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLine = m_lines[y & 3] + left * 3;
			for (int x = left; x < right; y++)
			{
				dstARGB[0] = 255;
				dstARGB[1] = srcLine[0];
				dstARGB[2] = srcLine[1];
				dstARGB[3] = srcLine[2];
				dstARGB += 4;
				srcLine += 3;
			}
		}
		return true;
	}

	inline bool GetBlock_RGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;
		while (m_linesRead < bottom)
		{
			if (!m_read(m_ctx, m_indices, m_header->m_width * m_numComponents))
				return false;

			unsigned char* line = m_lines[m_linesRead & 3];
			for (int x = 0; x < m_header->m_width; x++)
			{
				line[0] = m_palette[m_indices[x] * 3 + 2];
				line[1] = m_palette[m_indices[x] * 3 + 1];
				line[2] = m_palette[m_indices[x] * 3 + 0];
				line += 3;
			}

			m_linesRead++;
		}

		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLine = m_lines[y & 3] + left * 3;
			for (int x = left; x < right; y++)
			{
				dstARGB[0] = srcLine[0];
				dstARGB[1] = srcLine[1];
				dstARGB[2] = srcLine[2];
				dstARGB += 3;
				srcLine += 3;
			}
		}
		return true;
	}
};

struct ImageBlockProvider_CompressedTGA : ImageBlockProvider_Base
{
	int m_currDecodedOffset;
	unsigned char* m_decoded[8];
	int m_decodedBottom;

	bool Init(TexLib_Context* ctx, const TexLib_Header* header)
	{
		InitBase(ctx, header);

		unsigned char* scratchPad = (unsigned char*) m_ctx->m_alloc(m_ctx, header->m_width * m_numComponents * 8);
		if (!scratchPad)
			return false;
		for (int i = 0; i < 8; i++)
			m_decoded[i] = scratchPad + header->m_width * m_numComponents * i;

		m_decodedBottom = 0;
		m_currDecodedOffset = 0;
		return true;
	}

	~ImageBlockProvider_CompressedTGA()
	{
		m_ctx->m_free(m_ctx, m_decoded[0]);
	}

	inline bool GetBlock_ARGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;
		if (bottom > m_decodedBottom && !DecodeToLine(bottom))
			return false;
		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLineStart = m_decoded[y & 7];
			int srcLineOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				dstARGB[dstOffset + 3] = 255; // Clear alpha
				for (int i = 0; i < m_numComponents; i++) // Copy RGB(A)
					dstARGB[dstOffset + i] = srcLineStart[srcLineOffset + i];
				dstOffset += 4;
				srcLineOffset += m_numComponents;
			}
			srcLineOffset += (m_header->m_width - width) * m_numComponents;
		}
		return true;
	}

	inline bool GetBlock_RGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;
		if (bottom > m_decodedBottom && !DecodeToLine(bottom))
			return false;
		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLineStart = m_decoded[y & 7];
			int srcLineOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				for (int i = 0; i < 3; i++)
					dstARGB[dstOffset + i] = srcLineStart[srcLineOffset + i];
				dstOffset += 3;
				srcLineOffset += m_numComponents;
			}
			srcLineOffset += (m_header->m_width - width) * m_numComponents;
		}
		return true;
	}

private:
	inline bool DecodeToLine(int bottom)
	{
		while (m_decodedBottom <= bottom)
		{
			unsigned char chunkHeader;
			if (!m_read(m_ctx, &chunkHeader, 1))
				return false;
			if (chunkHeader < 128) // Read raw color
			{
				chunkHeader++;
				for (int i = 0; i < chunkHeader; i++)
				{
					if (m_currDecodedOffset == m_numComponents * m_header->m_width)
					{
						m_decodedBottom++;
						m_currDecodedOffset = 0;
					}
					unsigned char* dst = m_decoded[m_decodedBottom & 7] + m_currDecodedOffset;
					if (!m_read(m_ctx, dst, m_numComponents))
						return false;
					m_currDecodedOffset += m_numComponents;
				}
			}
			else
			{
				chunkHeader -= 127;
				unsigned char color[4];
				if (!m_read(m_ctx, color, m_numComponents))
					return false;

				for (int i = 0; i < chunkHeader; i++)
				{
					if (m_currDecodedOffset == m_numComponents * m_header->m_width)
					{
						m_decodedBottom++;
						m_currDecodedOffset = 0;
					}
					unsigned char* dst = m_decoded[m_decodedBottom & 7] + m_currDecodedOffset;
					memcpy(dst, color, m_numComponents);
					m_currDecodedOffset += m_numComponents;
				}
			}
		}
		return true;
	}
};

bool TexLib_LoadTGA(TexLib_Context* ctx)
{
	TexLib_Header header;

	// Get texture dimensions from the header

	static const unsigned char uncompressed8BitTGAHeader[12]	= {0, 1, 1, 0, 0, 0, 1, 24,0, 0, 0, 0};
	static const unsigned char uncompressedTGAHeader[12]		= {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const unsigned char compressedTGAHeader[12]			= {0, 0, 10,0, 0, 0, 0, 0, 0, 0, 0, 0};

	unsigned char cmpHeader[12];
	if (!ctx->m_read(ctx, cmpHeader, 12))
		return false;

	unsigned char headerData[6];
	if (!ctx->m_read(ctx, headerData, 6))
		return false;

	header.m_width = (int) headerData[1] * 256 + (int) headerData[0];
	header.m_height = (int) headerData[3] * 256 + (int) headerData[2];
	header.m_depth = 0;
	header.m_numFaces = 1;
	header.m_numLevels = 1;

	const int numBits = (int) headerData[4];
	if (numBits & 7)
		return false;
	header.m_hasColor = true;
	header.m_hasAlpha = numBits == 32;
	if (header.m_hasColor && header.m_hasAlpha)
		header.m_format = TexLib_Format_ARGB;
	else if (header.m_hasColor)
		header.m_format = TexLib_Format_RGB;
	else
		header.m_format = TexLib_Format_A;

	if (!ctx->m_onHeader(ctx, &header))
		return false;

	if (!memcmp(uncompressedTGAHeader, cmpHeader, sizeof(uncompressedTGAHeader)))
		return TexLib_TransformImage<ImageBlockProvider_GenericInversedY>(ctx, &header);
	else if (!memcmp(compressedTGAHeader, cmpHeader, sizeof(compressedTGAHeader)))
		return TexLib_TransformImage<ImageBlockProvider_CompressedTGA>(ctx, &header);
	else if (!memcmp(uncompressed8BitTGAHeader, cmpHeader, sizeof(uncompressed8BitTGAHeader)))
		return TexLib_TransformImage<ImageBlockProvider_8BitTGA>(ctx, &header);

	return false;
}
#pragma once

#include <assert.h>

#ifdef WIN32
	#include <windows.h>   /* for assert */
	#define restrict __restrict
#elif defined(MARMALADE)
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <math.h>
	#include <time.h>
	#include "s3e.h"
	inline char tolower(char c) { return ('a' <= c && c <= 'z') ? c : (c + 'a' - 'A'); }
	inline int min(int a, int b) { return a < b ? a : b; }
	inline int max(int a, int b) { return a > b ? a : b; }
	#define restrict
#endif

#include "TexLib.h"

#ifdef TEXLIB_HAS_DDS

#include "TexLib_DXT.h"

inline bool IsDXT(TexLib_Format format)
{
	return TexLib_Format_DXT1 <= format && format <= TexLib_Format_DXT5;
}

inline bool IsDXT1(TexLib_Format format)
{
	return TexLib_Format_DXT1 <= format && format <= TexLib_Format_DXT1a;
}

#endif


inline int NumBytesPerPixel(TexLib_Format format)
{
	if (format == TexLib_Format_ARGB) return 4;
	if (format == TexLib_Format_RGB) return 3;
	assert(!"DXT formats not supported here");
	return 0;
}

struct ImageBlockProvider_Base
{
	TexLib_Context* m_ctx;
	const TexLib_Header* m_header;
	TexLib_ReadFunc m_read;
	void* m_userData;
	int m_numComponents;

	void InitBase(TexLib_Context* ctx, const TexLib_Header* header)
	{
		m_ctx = ctx;
		m_header = header;
		m_read = ctx->m_read;
		m_userData = ctx->m_userData;
		m_numComponents = (header->m_hasColor ? 3 : 0) + (header->m_hasAlpha ? 1 : 0);
	}
};

struct ImageBlockProvider_Generic : ImageBlockProvider_Base
{
	unsigned char* m_lines[4];
	int m_linesRead;

	bool Init(TexLib_Context* ctx, const TexLib_Header* header)
	{
		InitBase(ctx, header);

		m_lines[0] = (unsigned char*) m_ctx->m_alloc(m_ctx, m_header->m_width * m_numComponents * 4);
		if (!m_lines[0])
			return false;
		m_lines[1] = m_lines[0] + m_header->m_width * m_numComponents * 1;
		m_lines[2] = m_lines[0] + m_header->m_width * m_numComponents * 2;
		m_lines[3] = m_lines[0] + m_header->m_width * m_numComponents * 3;

		m_linesRead = 0;
		return true;
	}

	~ImageBlockProvider_Generic()
	{
		if (m_lines[0])
			m_ctx->m_free(m_ctx, m_lines[0]);
	}

	inline bool GetBlock_ARGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;
		while (m_linesRead < bottom)
		{
			if (!m_read(m_ctx, m_lines[m_linesRead & 3], m_header->m_width * m_numComponents))
				return false;
			m_linesRead++;
		}

		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLine = m_lines[y & 3];
			int srcOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				dstARGB[dstOffset + 3] = 255; // Clear alpha
				for (int i = 0; i < m_numComponents; i++) // Copy RGB(A)
					dstARGB[dstOffset + i] = srcLine[srcOffset + i];
				dstOffset += 4;
				srcOffset += m_numComponents;
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
			if (!m_read(m_ctx, m_lines[m_linesRead & 3], m_header->m_width * m_numComponents))
				return false;
			m_linesRead++;
		}

		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			const unsigned char* srcLine = m_lines[y & 3];
			int srcOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				for (int i = 0; i < 3; i++)
					dstARGB[dstOffset + i] = srcLine[srcOffset + i];
				dstOffset += 3;
				srcOffset += m_numComponents;
			}
		}
		return true;
	}
};

struct ImageBlockProvider_GenericInversedY : ImageBlockProvider_Base
{
	unsigned char* m_data;
	int m_rowSize;

	bool Init(TexLib_Context* ctx, const TexLib_Header* header)
	{
		InitBase(ctx, header);

		m_rowSize = m_header->m_width * m_numComponents;

		m_data = (unsigned char*) m_ctx->m_alloc(m_ctx, m_rowSize * (m_header->m_height + 1));
		if (!m_data)
			return false;

		// Read in whole image

		if (!m_read(m_ctx, m_data, m_rowSize * m_header->m_height))
			return false;

		// Invert Y

		unsigned char* tempBuffer = m_data + m_rowSize * m_header->m_height;
		const int halfHeight = m_header->m_height / 2;
		for (int y = 0; y < halfHeight; y++)
		{
			const int y2 = header->m_height - 1 - y;
			memcpy(tempBuffer, m_data + m_rowSize * y, m_rowSize);
			memcpy(m_data + m_rowSize * y, m_data + m_rowSize * y2, m_rowSize);
			memcpy(m_data + m_rowSize * y2, tempBuffer, m_rowSize);
		}

		return true;
	}

	~ImageBlockProvider_GenericInversedY()
	{
		if (m_data)
			m_ctx->m_free(m_ctx, m_data);
	}

	inline bool GetBlock_ARGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;

		const unsigned char* srcLine = m_data + m_rowSize * top;
		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			int srcOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				dstARGB[dstOffset + 3] = 255; // Clear alpha
				for (int i = 0; i < m_numComponents; i++) // Copy RGB(A)
					dstARGB[dstOffset + i] = srcLine[srcOffset + i];
				dstOffset += 4;
				srcOffset += m_numComponents;
			}
			srcLine += m_rowSize;
		}
		return true;
	}

	inline bool GetBlock_RGB(int left, int top, int width, int height, unsigned char* dstARGB)
	{
		const int right = left + width;
		const int bottom = top + height;

		const unsigned char* srcLine = m_data + m_rowSize * top;
		int dstOffset = 0;
		for (int y = top; y < bottom; y++)
		{
			int srcOffset = left * m_numComponents;
			for (int x = left; x < right; x++)
			{
				for (int i = 0; i < 3; i++)
					dstARGB[dstOffset + i] = srcLine[srcOffset + i];
				dstOffset += 3;
				srcOffset += m_numComponents;
			}
			srcLine += m_rowSize;
		}
		return true;
	}
};

template <typename ImageBlockProvider>
bool TexLib_TransformImage(TexLib_Context* ctx, const TexLib_Header* header)
{
	// Create image provider

	ImageBlockProvider blockProvider;
	if (!blockProvider.Init(ctx, header))
		return false;

	// Begin image

	TexLib_LevelData data;
	if (!ctx->m_onBeginFaceLevel(ctx, header, 0, 0, &data))
		return false;

	// Load and output image

	switch (data.m_format)
	{
#ifdef TEXLIB_HAS_DDS
	case TexLib_Format_DXT1:
		ConvertToDXT<ImageBlockProvider, TexLib_Format_DXT1>((unsigned char*) data.m_data, header->m_width, header->m_height, blockProvider);
		break;
	case TexLib_Format_DXT1a:
		ConvertToDXT<ImageBlockProvider, TexLib_Format_DXT1a>((unsigned char*) data.m_data, header->m_width, header->m_height, blockProvider);
		break;
	case TexLib_Format_DXT3:
		ConvertToDXT<ImageBlockProvider, TexLib_Format_DXT3>((unsigned char*) data.m_data, header->m_width, header->m_height, blockProvider);
		break;
	case TexLib_Format_DXT5:
		ConvertToDXT<ImageBlockProvider, TexLib_Format_DXT5>((unsigned char*) data.m_data, header->m_width, header->m_height, blockProvider);
		break;
#endif
	case TexLib_Format_ARGB:
		blockProvider.GetBlock_ARGB(0, 0, header->m_width, header->m_height, (unsigned char*) data.m_data);
		break;
	case TexLib_Format_RGB:
		blockProvider.GetBlock_RGB(0, 0, header->m_width, header->m_height, (unsigned char*) data.m_data);
		break;
	}

	// End image

	ctx->m_onEndFaceLevel(ctx, header, 0, 0);

	return true;
}

struct TexLib_Data_RAW
{
	unsigned char* m_curr;
	unsigned char* m_end;
};

inline bool TexLib_ReadFunc_RAW(TexLib_Context* ctx, void* data, int size)
{
	TexLib_Data_RAW* rawData = (TexLib_Data_RAW*) ctx->m_privateData;
	if ((intptr_t) rawData->m_curr + size > (intptr_t) rawData->m_end)
		return false;
	memcpy(data, rawData->m_curr, size);
	rawData->m_curr += size;
	return true;
}

inline bool TexLib_TransformRawImage(TexLib_Context* ctx, const TexLib_Header* header, TexLib_Data_RAW* rawData)
{
	TexLib_Context tmpCtx = *ctx;
	tmpCtx.m_read = TexLib_ReadFunc_RAW;
	tmpCtx.m_privateData = rawData;

	return TexLib_TransformImage<ImageBlockProvider_Generic>(&tmpCtx, header);
}

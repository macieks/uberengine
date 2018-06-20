#include "Utils/utCompression.h"
#include "Base/Containers/ueVector.h"

struct utCompression
{
	ueVector<utCompressionLib*> m_libs;
};

static utCompression s_data;

void utCompression_Startup(ueAllocator* stackAllocator, u32 maxLibs)
{
	UE_ASSERT_FUNC(s_data.m_libs.Init(stackAllocator, maxLibs));
}

void utCompression_Shutdown()
{
	for (u32 i = 0; i < s_data.m_libs.Size(); i++)
		s_data.m_libs[i]->m_deinitFunc(s_data.m_libs[i]);
	s_data.m_libs.Deinit();
}

void utCompression_RegisterLib(utCompressionLib* lib)
{
	UE_ASSERT(!utCompression_FindLib(lib->m_symbol));
	UE_ASSERT(lib->m_symbol != UT_NONE_COMPRESSION_SYMBOL);
	UE_ASSERT(lib->m_symbol != UT_BEST_COMPRESSION_SYMBOL);

	s_data.m_libs.PushBack(lib);
	lib->m_initFunc(lib);
}

utCompressionLib* utCompression_FindLib(u32 symbol)
{
	for (u32 i = 0; i < s_data.m_libs.Size(); i++)
		if (s_data.m_libs[i]->m_symbol == symbol)
			return s_data.m_libs[i];
	return NULL;
}

u32 utCompression_GetNumLibs()
{
	return s_data.m_libs.Size();
}

utCompressionLib* utCompression_GetLibByIndex(u32 index)
{
	return s_data.m_libs[index];
}

const char* utCompression_GetLibName(u32 libSymbol)
{
	if (libSymbol == UT_NONE_COMPRESSION_SYMBOL) return "NONE";
	if (libSymbol == UT_BEST_COMPRESSION_SYMBOL) return "BEST";
	utCompressionLib* lib = utCompression_FindLib(libSymbol);
	if (!lib) return "UNKNOWN";
	return lib->m_getNameFunc(lib);
}

ueBool utCompression_Decompress(u32 libSymbol, void* dst, ueSize dstSize, const void* src, ueSize srcSize)
{
	utCompressionLib* lib = utCompression_FindLib(libSymbol);
	if (!lib)
	{
		ueLogW("Failed to decompress data, reason: library '%s' not found", UE_4BYTE_SYMBOL_STR(libSymbol));
		return UE_FALSE;
	}
	return lib->m_decompressFunc(lib, dst, dstSize, src, srcSize);
}

ueSize utCompression_EstimateCompressedSize(u32 libSymbol, const void* src, ueSize srcSize)
{
	if (s_data.m_libs.Size() == 0 || libSymbol == UT_NONE_COMPRESSION_SYMBOL)
	{
		libSymbol = UT_NONE_COMPRESSION_SYMBOL;
		return srcSize;
	}

	if (libSymbol == UT_BEST_COMPRESSION_SYMBOL)
	{
		ueSize maxEstimate = 0;
		for (u32 i = 0; i < s_data.m_libs.Size(); i++)
		{
			const ueSize estimate = s_data.m_libs[i]->m_estimateCompressedSizeFunc(s_data.m_libs[i], src, srcSize);
			if (estimate > maxEstimate)
				maxEstimate = estimate;
		}
		return maxEstimate;
	}

	utCompressionLib* lib = utCompression_FindLib(libSymbol);
	if (!lib) return UE_FALSE;
	return lib->m_estimateCompressedSizeFunc(lib, src, srcSize);
}

ueBool utCompression_Compress(u32& libSymbol, void* dst, ueSize& dstSize, const void* src, ueSize srcSize)
{
	UE_ASSERT(libSymbol != UT_NONE_COMPRESSION_SYMBOL);

	if (s_data.m_libs.Size() == 0) return 0;

	if (libSymbol == UT_BEST_COMPRESSION_SYMBOL)
	{
		u32 bestSizeLibIndex = U32_MAX;
		ueSize bestSize = -1;
		for (u32 i = 0; i < s_data.m_libs.Size(); i++)
		{
			ueSize dstSizeCopy = dstSize;
			if (!s_data.m_libs[i]->m_compressFunc(s_data.m_libs[i], dst, dstSizeCopy, src, srcSize))
				continue;
			if (bestSizeLibIndex == U32_MAX || dstSizeCopy < bestSize)
			{
				bestSizeLibIndex = i;
				bestSize = dstSizeCopy;
			}
		}

		if (bestSizeLibIndex == U32_MAX)
			return UE_FALSE;

		// Best size larger than uncompressed?

		if (bestSize >= srcSize)
		{
			libSymbol = UT_NONE_COMPRESSION_SYMBOL;
			ueMemCpy(dst, src, srcSize);
			dstSize = srcSize;
			return UE_TRUE;
		}

		// Success!

		libSymbol = s_data.m_libs[bestSizeLibIndex]->m_symbol;

		if (bestSizeLibIndex == s_data.m_libs.Size() - 1) // No need to compress again
		{
			dstSize = bestSize;
			return UE_TRUE;
		}

		return s_data.m_libs[bestSizeLibIndex]->m_compressFunc(s_data.m_libs[bestSizeLibIndex], dst, dstSize, src, srcSize);
	}

	utCompressionLib* lib = utCompression_FindLib(libSymbol);
	if (!lib)
		return UE_FALSE;

	return lib->m_compressFunc(lib, dst, dstSize, src, srcSize);
}

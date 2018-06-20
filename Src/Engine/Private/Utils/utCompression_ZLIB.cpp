#include "Utils/utCompression.h"
#include "zlib.h"

struct ueLib_ZLIB : utCompressionLib
{
	static ueLib_ZLIB s_lib;

	ueLib_ZLIB()
	{
		m_symbol = UE_BE_4BYTE_SYMBOL('z', 'l', 'i', 'b');
		m_initFunc = InitFunc;
		m_deinitFunc = DeinitFunc;
		m_getNameFunc = GetNameFunc;
		m_decompressFunc = DecompressFunc;
		m_estimateCompressedSizeFunc = EstimateCompressedSizeFunc;
		m_compressFunc = CompressFunc;
	}

	static void InitFunc(utCompressionLib* lib)
	{
		UE_ASSERT(lib == &s_lib);
	}

	static void DeinitFunc(utCompressionLib* lib)
	{
		UE_ASSERT(lib == &s_lib);
	}

	static const char* GetNameFunc(utCompressionLib* lib)
	{
		UE_ASSERT(lib == &s_lib);
		return "ZLIB";
	}

	static ueBool DecompressFunc(utCompressionLib* lib, void* dst, ueSize dstSize, const void* src, ueSize srcSize)
	{
		UE_ASSERT(lib == &s_lib);
		if (srcSize == dstSize)
		{
			ueMemCpy(dst, src, srcSize);
			return UE_TRUE;
		}
		uLong dstSizeZLIB = (uLong) dstSize;
		const int result = uncompress((Bytef*) dst, &dstSizeZLIB, (const Bytef*) src, (uLong) srcSize);
		return result == Z_OK;
	}

	static ueSize EstimateCompressedSizeFunc(utCompressionLib* lib, const void* src, ueSize srcSize)
	{
		UE_ASSERT(lib == &s_lib);
		return compressBound((uLong) srcSize);
	}

	static ueBool CompressFunc(utCompressionLib* lib, void* dst, ueSize& dstSize, const void* src, ueSize srcSize)
	{
		UE_ASSERT(lib == &s_lib);

		uLongf compressedSizeZLIB;
		if (compress((Bytef*) dst, &compressedSizeZLIB, (const Bytef*) src, (uLong) srcSize) != Z_OK)z
			return UE_FALSE;

		dstSize = compressedSizeZLIB;
		if (dstSize >= srcSize)
		{
			dstSize = srcSize;
			ueMemCpy(dst, src, srcSize);
		}
		return UE_TRUE;
	}
};

ueLib_ZLIB ueLib_ZLIB::s_lib;

void ueRegisterCompressionLib_ZLIB()
{
	utCompression_RegisterLib(&ueLib_ZLIB::s_lib);
}

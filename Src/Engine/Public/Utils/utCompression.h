#ifndef UT_COMPRESSION_H
#define UT_COMPRESSION_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

//! Symbol for "no compression" mode
const u32 UT_NONE_COMPRESSION_SYMBOL	= UE_BE_4BYTE_SYMBOL('n', 'o', 'n', 'e');
//! Symbol indicating to choose the best available compression library (which can be expensive)
const u32 UT_BEST_COMPRESSION_SYMBOL	= UE_BE_4BYTE_SYMBOL('b', 'e', 's', 't');

//! Compression library description
struct utCompressionLib
{
	typedef void (*InitFunc)(utCompressionLib* lib);
	typedef void (*DeinitFunc)(utCompressionLib* lib);

	typedef const char* (*GetNameFunc)(utCompressionLib* lib);
	typedef ueBool (*DecompressFunc)(utCompressionLib* lib, void* dst, ueSize dstSize, const void* src, ueSize srcSize);
	typedef ueSize (*EstimateCompressedSizeFunc)(utCompressionLib* lib, const void* src, ueSize srcSize);
	typedef ueBool (*CompressFunc)(utCompressionLib* lib, void* dst, ueSize& dstSize, const void* src, ueSize srcSize);

	u32 m_symbol;	//!< Unique compression library symbol

	InitFunc m_initFunc;
	DeinitFunc m_deinitFunc;
	GetNameFunc m_getNameFunc;
	DecompressFunc m_decompressFunc;
	EstimateCompressedSizeFunc m_estimateCompressedSizeFunc;
	CompressFunc m_compressFunc;

	utCompressionLib() { ueMemSet(this, 0, sizeof(utCompressionLib)); }
};

//! Starts up compression system
void				utCompression_Startup(ueAllocator* stackAllocator, u32 maxLibs = 16);
//! Shuts down compression system
void				utCompression_Shutdown();

//! Registers compression library
void				utCompression_RegisterLib(utCompressionLib* lib);
//! Finds compression library by symbol
utCompressionLib*	utCompression_FindLib(u32 symbol);
//! Gets number of registered compression libraries
u32					utCompression_GetNumLibs();
//! Gets compression library by index (from 0 to utCompression_GetNumLibs() - 1)
utCompressionLib*	utCompression_GetLibByIndex(u32 index);
//! Gets compression library name by symbol
const char*			utCompression_GetLibName(u32 libSymbol);
//! Decompresses data using library of given symbol (can be UT_NONE_COMPRESSION_SYMBOL and UT_BEST_COMPRESSION_SYMBOL)
ueBool				utCompression_Decompress(u32 libSymbol, void* dst, ueSize dstSize, const void* src, ueSize srcSize);
//! Estimates compressed data size using library of given symbol (can be UT_NONE_COMPRESSION_SYMBOL and UT_BEST_COMPRESSION_SYMBOL)
ueSize				utCompression_EstimateCompressedSize(u32 libSymbol, const void* src, ueSize srcSize);
//! Compresses data using library of given symbol (can be UT_NONE_COMPRESSION_SYMBOL and UT_BEST_COMPRESSION_SYMBOL)
ueBool				utCompression_Compress(u32& libSymbol, void* dst, ueSize& dstSize, const void* src, ueSize srcSize);

// @}

#endif // UT_COMPRESSION_H

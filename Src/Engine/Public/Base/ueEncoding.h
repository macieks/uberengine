#ifndef UT_ENCODING_H
#define UT_ENCODING_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

u32		ueEncoding_EstimateBase64(u32 length);
ueBool	ueEncoding_Base64(const char* src, u32 srcLength, char* dst, u32 dstCapacity, u32* dstLength = NULL);

u32		ueEncoding_EstimateUrl(const char* src);
ueBool	ueEncoding_EncodeUrl(const char* str, char* dst, u32 dstCapacity);

// @}

#endif // UT_ENCODING_H

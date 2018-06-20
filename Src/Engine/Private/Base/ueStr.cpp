#include "Base/ueBase.h"

#if defined(UE_WIN32)
	#include <shlwapi.h>
#endif

s32 ueStrFirstIndexOf(const char* s, char c)
{
	if (!s) return -1;
	const char* ptr = s;
	while (*ptr)
	{
		if (*ptr == c) return (s32) ((ueSize) ptr - (ueSize) s);
		ptr++;
	}
	return -1;
}

u32 ueStrLen(const char* buffer)
{
	if (!buffer) return 0;
	return (u32) strlen(buffer);
}

ueBool ueStrLenGreater(const char* buffer, u32 value)
{
	if (!buffer) return UE_FALSE;

	u32 length = 0;
	while (buffer[length] && length <= value)
		length++;
	return buffer[length] && value < length;
}

s32 ueStrCmp(const char* str1, const char* str2)
{
	if (!str1 || !*str1)
	{
		if (!str2 || !*str2)
			return 0;
		return -1;
	}
	else if (!str2 || !*str2)
		return 1;
	return (s32) strcmp(str1, str2);
}

s32 ueStrNCmp(const char* str1, const char* str2, u32 num)
{
	return (s32) strncmp(str1, str2, num);
}

s32 ueStrNICmp(const char* str1, const char* str2, u32 num)
{
#if defined(UE_WIN32)
	return _strnicmp(str1, str2, num);
#else
	return (s32) strncasecmp(str1, str2, num);
#endif
}

s32 ueStrICmp(const char* str1, const char* str2)
{
#if defined(UE_WIN32)
	return _stricmp(str1, str2);
#else
	return strcasecmp(str1, str2);
#endif
}

ueBool ueStrLCmpEnd(const char* string, const char* pattern)
{
	const u32 stringLength = ueStrLen(string);
	const u32 patternLength = ueStrLen(pattern);

	return stringLength > patternLength && !ueStrICmp(string + stringLength - patternLength, pattern);
}

char* ueStrCpy(char* dest, u32 destSize, const char* src)
{
	UE_ASSERT(ueStrLen(src) + 1 <= destSize);
	if (!src)
	{
		dest[0] = '\0';
		return dest;
	}

#if defined(UE_WIN32)
	strcpy_s(dest, destSize, src);
	return dest;
#else
	return strcpy(dest, src);
#endif
}

char* ueStrNCpy(char* dest, u32 destSize, const char* src, u32 num)
{
	UE_ASSERT(num + 1 <= destSize);
	ueMemCpy(dest, src, num);
	dest[num] = '\0';
	return dest;
}

char* ueStrDup(ueAllocator* allocator, const char* src)
{
	const u32 len = ueStrLen(src);
	char* s = (char*) allocator->Alloc(len + 1);
	if (!s)
		return NULL;
	ueMemCpy(s, src, len + 1);
	return s;
}

char* ueStrCat(char* dest, u32 destSize, const char* src)
{
	UE_ASSERT(ueStrLen(dest) + ueStrLen(src) + 1 <= destSize);

#if defined(UE_WIN32)
	strcat_s(dest, destSize, src);
	return dest;
#else
	return strcat(dest, src);
#endif
}

char* ueStrCatFit(char* dst, u32 dstSize, const char* src)
{
	const u32 srcLen = ueStrLen(src);
	const u32 dstLen = ueStrLen(dst);
	return ueStrNCat(dst, dstSize, src, dstSize - dstLen - 1);
}

void ueStrCat2(char*& s, const char* t)
{
	while (*t)
		*s++ = *t++;
	*s = 0;
}

char* ueStrNCat(char* dest, u32 destSize, const char* src, u32 srcSize)
{
#if defined(UE_DEBUG)
	u32 srcActualSize = ueStrLen(src);
	if (srcActualSize > srcSize)
		srcActualSize = srcSize;
	UE_ASSERT(ueStrLen(dest) + srcActualSize + 1 <= destSize);
#endif
#if defined(UE_WIN32)
	strncat_s(dest, destSize, src, srcSize);
	return dest;
#else
	return strncat(dest, src, srcSize);
#endif
}

char* ueStrStr(const char* buffer, const char* match, ueBool caseSensitive)
{
	if (caseSensitive)
		return (char*) strstr(buffer, match);

#if defined(UE_WIN32)
	return (char*) StrStrI(buffer, match);
#else
	if (!buffer || !*buffer || !match || !*match) return NULL;
	while (*buffer)
	{
		const char* iter = match;
		const char* bufferPtr = buffer;
		while (*iter)
		{
			if (ueStrToLower(*iter) != ueStrToLower(*bufferPtr))
				break;
			iter++;
			bufferPtr++;
		}
		if (!*iter)
			return (char*) buffer;
		buffer++;
	}
	return NULL;
#endif
}

void ueStrToUpper(char* s)
{
	while (*s)
	{
		*s = ueStrToUpper(*s);
		s++;
	}
}

void ueStrToLower(char* s)
{
	while (*s)
	{
		*s = ueStrToLower(*s);
		s++;
	}
}

ueBool ueStrIsLower(const char* s)
{
	while (*s)
		if (!ueStrIsLower(*s))
			return UE_FALSE;
		else
			s++;
	return UE_TRUE;
}

void ueStrToUpper(char* dest, u32 destSize, const char* src)
{
	UE_ASSERT(ueStrLen(src) + 1 <= destSize);
	while (*src)
	{
		*dest = ueStrToUpper(*src);
		src++;
		dest++;
	}
	*dest = 0;
}

void ueStrToLower(char* dest, u32 destSize, const char* src)
{
	UE_ASSERT(ueStrLen(src) + 1 <= destSize);
	while (*src)
	{
		*dest = ueStrToLower(*src);
		src++;
		dest++;
	}
	*dest = 0;
}

void ueStrReplaceAll(char* dest, u32 destSize, const char* src, const char* match, const char* replace)
{
	UE_ASSERT(dest);
	UE_ASSERT(src);
	UE_ASSERT(match);
	UE_ASSERT(replace);
	UE_ASSERT(src != dest);

	u32 matchStringSize = ueStrLen(match);
	const char* inStr = src;
	char* outStr = dest;
	outStr[0] = '\0';
	const char* str = ueStrStr(src, match);
	while (str != 0)
	{
		const u32 numCharsToCopy = (u32) (str - inStr);
		ueStrNCat(outStr, destSize, inStr, numCharsToCopy);
		ueStrCat(outStr, destSize, replace);
		inStr = str + matchStringSize;

		str = ueStrStr(inStr, match);
	}
	ueStrCat(outStr, destSize, inStr);
}

s32 ueStrFormatVArgs(char* buffer, u32 bufferSize, const char* format, va_list args)
{
	if (!format || *format == 0)
	{
		*buffer = 0;
		return 0;
	}
#if defined(UE_WIN32) || defined(UE_X360)
	return _vsnprintf_s(buffer, bufferSize, bufferSize, format, args);
#else
    return vsnprintf(buffer, bufferSize, format, args);
#endif
}

s32 ueStrFormat(char* buffer, u32 bufferSize, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const s32 result = ueStrFormatVArgs(buffer, bufferSize, format, args);
	va_end(args);

	return result;
}

char* ueStrTok(char* src, const char* separators)
{
	if (!src) return NULL;
	char* context = NULL;
#if defined(UE_WIN32) || defined(UE_X360)
	return strtok_s(src, separators, &context);
#else
    return strtok(src, separators);
#endif
}

ueBool ueStrTokenize(char* tokens, const char* separators, char** tokensPtrs, u32* _numTokens)
{
	char* nextToken;
	u32 tokensLength;

	UE_ASSERT(NULL != tokens);
	UE_ASSERT(NULL != tokensPtrs);
	UE_ASSERT(NULL != _numTokens);
	UE_ASSERT(0 < *_numTokens);

	u32 numTokens = 0;
	tokensLength = ueStrLen(tokens);
	nextToken = tokens;

	while (tokensPtrs[numTokens] = ueStrTok(nextToken, separators))
	{
		if (numTokens == *_numTokens)
			return UE_FALSE;

		numTokens++;
		nextToken += ueStrLen(tokensPtrs[numTokens - 1]) + 1;
		if (((ueSize) nextToken - (ueSize) tokens) >= tokensLength)
			break;
	}

	*_numTokens = numTokens;
	return UE_TRUE;
}

char* ueStrStrSkip(const char* s, const char* p, ueBool caseSensitive)
{
	char* result = ueStrStr(s, p, caseSensitive);
	if (!result) return NULL;
	return result + ueStrLen(p);
}

char* ueStrLast(const char* s, char p)
{
	if (!s) return NULL;
	const char* result = NULL;
	while (*s)
	{
		if (*s == p) result = s;
		++s;
	}
	return (char*) result;
}

char* ueStrLastSkip(const char* s, char p)
{
	char* result = ueStrLast(s, p);
	if (!result) return NULL;
	return result + 1;
}

ueBool ueStrToBool(const char* buffer, ueBool& value)
{
	if (!ueStrICmp(buffer, "true"))
	{
		value = true;
		return true;
	}
	if (!ueStrICmp(buffer, "false"))
	{
		value = UE_FALSE;
		return true;
	}
	return UE_FALSE;
}

ueBool ueStrToS32(const char* buffer, s32& value)
{
	return ueStrScanf(buffer, "%d", &value) == 1;
}

ueBool ueStrToU32(const char* buffer, u32& value)
{
	return ueStrScanf(buffer, "%u", &value) == 1;
}

ueBool ueStrToF32(const char* buffer, f32& value)
{
	return ueStrScanf(buffer, "%f", &value) == 1;
}

const char* ueStrFromBool(ueBool value)
{
	return value ? "true" : "false";
}

ueBool ueStrFromBool(char* buffer, u32 bufferSize, ueBool value)
{
	return ueStrCpy(buffer, bufferSize, value ? "true" : "false") != NULL;
}

ueBool ueStrFromS32(char* buffer, u32 bufferSize, s32 value)
{
	return ueStrFormat(buffer, bufferSize, "%d", value) == 1;
}

ueBool ueStrFromU32(char* buffer, u32 bufferSize, u32 value)
{
	return ueStrFormat(buffer, bufferSize, "%u", value) == 1;
}

ueBool ueStrFromF32(char* buffer, u32 bufferSize, f32 value, u32 precision)
{
	if (precision == U32_MAX)
		return ueStrFormat(buffer, bufferSize, "%f", value) == 1;

	precision = ueMin(precision, (u32) 20);
	char format[6] = "%.?ff";
	if (precision >= 10)
	{
		format[2] = (precision / 10) + '0';
		format[3] = (precision % 10) + '0';
	}
	else
	{
		format[2] = precision + '0';
		format[4] = '\0';
	}
	return ueStrFormat(buffer, bufferSize, format, value) == 1;
}

void ueStrFormatBytes(char* buffer, u32 bufferSize, u32 value, u32 num1024s)
{
	UE_ASSERT(bufferSize >= num1024s * 4 - 1);
	char* ptr = buffer + num1024s * 4 - 1;
	*ptr = 0;
	for (u32 i = 0; i < num1024s; i++)
	{
		for (u32 j = 0; j < 3; j++)
		{
			const u32 digit = value % 10;
			value = value / 10;
			*(--ptr) = '0' + digit;
			if (!value)
				break;
		}
		if (!value)
			break;
		if (ptr != buffer)
			*(--ptr) = ' ';
	}
	while (ptr != buffer)
		*(--ptr) = ' ';
}

// -----------------------------------------------------------------------------

/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (u32) 0x0000FFFD
#define UNI_MAX_BMP (u32) 0x0000FFFF
#define UNI_MAX_UTF16 (u32) 0x0010FFFF
#define UNI_MAX_UTF32 (u32) 0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (u32) 0x0010FFFF

#define UNI_SUR_HIGH_START  (u32) 0xD800
#define UNI_SUR_HIGH_END    (u32) 0xDBFF
#define UNI_SUR_LOW_START   (u32) 0xDC00
#define UNI_SUR_LOW_END     (u32) 0xDFFF

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static const char trailingBytesForUTF8[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during u8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const u32 offsetsFromUTF8[6] =
{ 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const u8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns UE_FALSE.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

ueBool ueUTF8_IsLegalChar(const u8* source, u32 length)
{
    u8 a;
    const u8 *srcptr = source + length;
    switch (length)
	{
		default: return UE_FALSE;

		// Everything else falls through when "UE_TRUE"...
		case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return UE_FALSE;
		case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return UE_FALSE;
		case 2: if ((a = (*--srcptr)) > 0xBF) return UE_FALSE;

		switch (*source)
		{
			// no fall-through in this inner switch
			case 0xE0: if (a < 0xA0) return UE_FALSE; break;
			case 0xED: if (a > 0x9F) return UE_FALSE; break;
			case 0xF0: if (a < 0x90) return UE_FALSE; break;
			case 0xF4: if (a > 0x8F) return UE_FALSE; break;
			default:   if (a < 0x80) return UE_FALSE;
		}

		case 1: if (*source >= 0x80 && *source < 0xC2) return UE_FALSE;
    }
    if (*source > 0xF4) return UE_FALSE;
    return UE_TRUE;
}

ueBool ueStrUTF8ToUTF32(const u8* src, u32 srcSize, u32* dst, u32& dstSize)
{
	const u32* dstStart = dst;
	const u32* dstEnd = dst + dstSize;
	const u8* srcEnd = src + srcSize;
    while (src < srcEnd)
	{
		u32 ch = 0;
		const u32 extraBytesToRead = trailingBytesForUTF8[*src];
		if (extraBytesToRead >= srcSize)
			return UE_FALSE;

		// Do this check whether lenient or strict
		if (!ueUTF8_IsLegalChar(src, extraBytesToRead + 1))
			return UE_FALSE;

		// The cases all fall through. See "Note A" below.
		switch (extraBytesToRead)
		{
			case 5: ch += *src++; ch <<= 6;
			case 4: ch += *src++; ch <<= 6;
			case 3: ch += *src++; ch <<= 6;
			case 2: ch += *src++; ch <<= 6;
			case 1: ch += *src++; ch <<= 6;
			case 0: ch += *src++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (dst >= dstEnd)
			return UE_FALSE;

		if (ch <= UNI_MAX_LEGAL_UTF32)
		{
			// UTF-16 surrogate values are illegal in UTF-32, and anything
			// over Plane 17 (> 0x10FFFF) is illegal.
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
#if defined(UE_DEBUG)
			{
				ueLogE("Attempted to convert invalid UTF-8 string to UTF-32 (string = '%s')\n", src);
				return UE_FALSE;
			}
#else
				*dst++ = UNI_REPLACEMENT_CHAR;
#endif
			else
				*dst++ = ch;
		} else // i.e., ch > UNI_MAX_LEGAL_UTF32
#if defined(UE_DEBUG)
		{
			ueLogE("Attempted to convert invalid UTF-8 string to UTF-32 (string = '%s')\n", src);
			return UE_FALSE;
		}
#else
			*dst++ = UNI_REPLACEMENT_CHAR;
#endif
    }
	dstSize = (u32) ((ueSize) dst - (ueSize) dstStart) / sizeof(u32);
    return UE_TRUE;
}

void ueStrCpyWToA(char* dst, u32 dstSize, const wchar_t* src)
{
	while (*src && dstSize)
	{
		*dst++ = (char) *src++;
		dstSize--;
	}
}

const char* ueStrSkipSpaces(const char* s)
{
	while (*s && ueStrIsSpace(*s))
		s++;
	return s;
}

const char* ueStrSkipLine(const char* s)
{
	while (*s)
	{
		if (*s == '\n')
			return ++s;
		s++;
	}
	return s;
}

const char* ueStrSkipToNextInLine(const char c, const char* s)
{
	while (!ueStrIsEOL(*s) && *s != c)
		s++;
	return s;
}

const char* ueStrSkipToNextSpaceInLine(const char* s)
{
	while (!ueStrIsEOL(*s) && !ueStrIsSpace(*s))
		s++;
	return s;
}
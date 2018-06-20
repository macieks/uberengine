#ifndef UE_STR_H
#define UE_STR_H

/**
 *	@addtogroup ue
 *	@{
 */

// Length

//! Gets string length
u32 ueStrLen(const char* str);
//! Tells whether string length is greater than given value
ueBool ueStrLenGreater(const char* str, u32 value);

// Copying

//! Copies string
char* ueStrCpy(char* dest, u32 destSize, const char* src);
//! Copies string
#define ueStrCpyS(dest, src) ueStrCpy(dest, UE_ARRAY_SIZE(dest), src)
//! Copies up to given number of characters of the string
char* ueStrNCpy(char* dest, u32 destSize, const char* src, u32 num);
//! Duplicates string allocating memory from given allocator
char* ueStrDup(ueAllocator* allocator, const char* src);

// Appending

//! Concatenates string
char* ueStrCat(char* dest, u32 destSize, const char* src);
//! Concatenates string
#define ueStrCatS(dest, src) ueStrCat(dest, UE_ARRAY_SIZE(dest), src)
//! Appends src into dst and offsets dst appropriately
void ueStrCat2(char*& dst, const char* src);
//! Concatenates up to given number of characters into destination string
char* ueStrNCat(char* dst, u32 dstSize, const char* src, u32 srcSize);
//! Concatenates src into dst; cuts src if needed
char* ueStrCatFit(char* dst, u32 dstSize, const char* src);

// Searching

//! Returns pointer to first found occurence of string pattern; return NULL if not found
char* ueStrStr(const char* str, const char* match, ueBool caseSensitive = UE_TRUE);
//! Returns pointer to the end of first found occurence of string pattern; return NULL if not found
char* ueStrStrSkip(const char* str, const char* match, ueBool caseSensitive = UE_TRUE);
//! Returns pointer to last found occurence of string pattern; return NULL if not found
char* ueStrLast(const char* str, char match);
//! Returns pointer to the end of last found occurence of string pattern; return NULL if not found
char* ueStrLastSkip(const char* str, char match);
//! Retusn index of the first occurence of character c withing given string; returns -1 if not found
s32 ueStrFirstIndexOf(const char* s, char c);

// Comparisons

//! Compares string
s32 ueStrCmp(const char* str1, const char* str2);
//! Compares up to given number of characters within given strings
s32 ueStrNCmp(const char* str1, const char* str2, u32 num);
//! Performs case-insensitive string comparison
s32 ueStrICmp(const char* str1, const char* str2);
//! Performs case-insensitive string comparison up to give number of characters within given string
s32 ueStrNICmp(const char* str1, const char* str2, u32 num);
//! Performs "less" string comparison between suffix of string and pattern
ueBool ueStrLCmpEnd(const char* string, const char* pattern);

// Upper / lower case

//! Converts all string characters to upper case
void ueStrToUpper(char* dest, u32 destSize, const char* src);
//! Converts all string characters to lower case
void ueStrToLower(char* dest, u32 destSize, const char* src);
//! Converts all string characters to upper case
void ueStrToUpper(char* s);
//! Converts all string characters to lower case
void ueStrToLower(char* s);
//! Tells whether given string is all lower case
ueBool ueStrIsLower(const char* s);

//! Tells whether given character is lower case
UE_INLINE ueBool ueStrIsLower(char c) { return c < 'A' || 'Z' < c; }
//! Converts character to upper case
UE_INLINE char ueStrToUpper(char c) { return ('a' <= c && c <= 'z') ? (c + 'A' - 'a') : c; }
//! Converts character to lower case
UE_INLINE char ueStrToLower(char c) { return ('A' <= c && c <= 'Z') ? (c + 'a' - 'A') : c; }

// Formatting / scanning

//! Formats string
s32 ueStrFormat(char* dest, u32 destSize, const char* format, ...);
//! Formats string
#define ueStrFormatS(buffer, format, ...) ueStrFormat(buffer, UE_ARRAY_SIZE(buffer), format, __VA_ARGS__)
//! Formats string
s32 ueStrFormatVArgs(char* dest, u32 destSize, const char* format, va_list args);

//! Appends formatted string
#define ueStrCatFormatS(buffer, format, ...) { const u32 len = ueStrLen(buffer); if (len < UE_ARRAY_SIZE(buffer)) ueStrFormat(&buffer[len], UE_ARRAY_SIZE(buffer) - len, format, __VA_ARGS__); }

#if defined(UE_WIN32)
	//! Parses string into values
	#define ueStrScanf(buffer, format, ...) (sscanf_s(buffer, format, __VA_ARGS__))
#elif defined(UE_LINUX) || defined(UE_MARMALADE)
    #define ueStrScanf(buffer, format, ...) (sscanf(buffer, format, __VA_ARGS__))
#else
	"unsupported platform"
#endif

//! Converts kilo/mega/giga-byte value into readable 1024-separated string
void ueStrFormatBytes(char* buffer, u32 bufferSize, u32 value, u32 num1024s);

// Misc.

//! Replaces all occurences of given pattern with another string
void ueStrReplaceAll(char* dest, u32 destSize, const char* src, const char* match, const char* replace);

// Parsing

//! Tokenizes string; returns pointer to next found token; note: modifies input string
char* ueStrTok(char* src, const char* separators);
//! Tokenizes string; pointers to found tokens
ueBool ueStrTokenize(char* tokens, const char* separators, char** tokensPtrs, u32* numTokens);

//! Tells whether given character is considered text space (space or tab)
UE_INLINE ueBool ueStrIsSpace(char c) { return c == ' ' || c == '\t'; }
//! Tells whether given character is end-of-line character
UE_INLINE ueBool ueStrIsEOL(char s) { return s == '\0' || s == '\r' || s == '\n'; }

//! Skips string spaces
const char* ueStrSkipSpaces(const char* s);
//! Skips to the next string line
const char* ueStrSkipLine(const char* s);
//! Skips to next character in current line
const char* ueStrSkipToNextInLine(const char c, const char* s);
//! Skips to next space in current line
const char* ueStrSkipToNextSpaceInLine(const char* s);

// Conversions

//! Converts string to boolean value
ueBool ueStrToBool(const char* buffer, ueBool& value);
//! Converts string to s32 value
ueBool ueStrToS32(const char* buffer, s32& value);
//! Converts string to u32 value
ueBool ueStrToU32(const char* buffer, u32& value);
//! Converts string to f32 value
ueBool ueStrToF32(const char* buffer, f32& value);

//! Gets boolean value as string
const char* ueStrFromBool(ueBool value);
//! Gets boolean value as string
ueBool ueStrFromBool(char* buffer, u32 bufferSize, ueBool value);
//! Gets s32 value as string
ueBool ueStrFromS32(char* buffer, u32 bufferSize, s32 value);
//! Gets u32 value as string
ueBool ueStrFromU32(char* buffer, u32 bufferSize, u32 value);
//! Gets f32 value as string
ueBool ueStrFromF32(char* buffer, u32 bufferSize, f32 value, u32 precision = U32_MAX);

//! Converts from UTF-8 to UTF-32
ueBool ueStrUTF8ToUTF32(const u8* src, u32 srcSize, u32* dst, u32& dstSize);

void ueStrCpyWToA(char* dst, u32 dstSize, const wchar_t* src);

// @}

#endif // UE_STR_H
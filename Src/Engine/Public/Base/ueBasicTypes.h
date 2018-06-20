#ifndef UE_BASIC_TYPES_H
#define UE_BASIC_TYPES_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/uePlatform.h"

#if defined(UE_WIN32)

	#define UE_FALSE false
	#define UE_TRUE true
	typedef bool ueBool;

	typedef __int8 s8;
	typedef unsigned __int8 u8;
	typedef __int16 s16;
	typedef unsigned __int16 u16;
	typedef __int32 s32;
	typedef unsigned __int32 u32;
	typedef __int64 s64;
	typedef unsigned __int64 u64;
	typedef float f32;
	typedef double f64;

#elif defined(UE_X360)

	#define UE_FALSE ((u32) 0)
	#define UE_TRUE ((u32) U32_MAX)
	typedef unsigned __int32 ueBool;

	typedef __int8 s8;
	typedef unsigned __int8 u8;
	typedef __int16 s16;
	typedef unsigned __int16 u16;
	typedef __int32 s32;
	typedef unsigned __int32 u32;
	typedef __int64 s64;
	typedef unsigned __int64 u64;
	typedef float f32;
	typedef double f64;

#elif defined(UE_LINUX) || defined(UE_MARMALADE)

	#define UE_FALSE false
	#define UE_TRUE true
	typedef bool ueBool;

	typedef signed char s8;
	typedef unsigned char u8;
	typedef signed short s16;
	typedef unsigned short u16;
	typedef signed int s32;
	typedef unsigned int u32;
	typedef signed long long s64;
	typedef unsigned long long u64;
	typedef float f32;
	typedef double f64;

#endif

typedef size_t ueSize;

//! Time ticks (the actual unit meaning is hardware / platform specific)
typedef u64 ueTime;

// @}

#endif // UE_BASIC_TYPES_H

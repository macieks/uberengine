#ifndef UE_ENDIANESS_H
#define UE_ENDIANESS_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Value used to indicate little endianess
#define UE_ENDIANESS_LITTLE 0x01010101
//! Value used to indicate big endianess
#define UE_ENDIANESS_BIG	0x02020202

//! Type of the value indicating endianess
typedef u32 ueEndianess;

#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_MAC) || defined(UE_IPHONE) || defined(UE_MARMALADE)
	#define UE_ENDIANESS UE_ENDIANESS_LITTLE
#elif defined(UE_X360) || defined(UE_PS3) || defined(UE_WII)
	#define UE_ENDIANESS UE_ENDIANESS_BIG
#endif

//! Gets endianess for given platform
UE_INLINE u32 uePlatformEndianess(uePlatform platform)
{
	switch (platform)
	{
		case uePlatform_Win32:
		case uePlatform_Linux:
		case uePlatform_Mac:
		case uePlatform_Marmalade:
			return UE_ENDIANESS_LITTLE;
		case uePlatform_X360:
		case uePlatform_PS3:
		case uePlatform_WII:
			return UE_ENDIANESS_BIG;
		UE_ASSERT(!"Unknown platform");
	}
	return UE_ENDIANESS_LITTLE;
}

/*---------------------- Endianess swapping functions -------------------------*/

UE_INLINE ueBool ueSwapEndian(ueBool value)
{
	return value;
}

UE_INLINE s8 ueSwapEndian(s8 value)
{
	return value;
}

UE_INLINE u8 ueSwapEndian(u8 value)
{
	return value;
}

UE_INLINE s16 ueSwapEndian(s16 value)
{
	return (s16) ((value << 8) | ((value >> 8) & 0xff));
}

UE_INLINE u16 ueSwapEndian(u16 value)
{
	return (u16) ((value << 8) | ((value >> 8) & 0xff));
}

UE_INLINE s32 ueSwapEndian(s32 value)
{
	return (s32) (value << 24) | ((value << 8) & 0x00ff0000) | ((value >> 8) & 0x0000ff00) | ((value >> 24) & 0x000000ff);
}

UE_INLINE u32 ueSwapEndian(u32 value)
{
	return (u32) (value << 24) | ((value << 8) & 0x00ff0000) | ((value >> 8) & 0x0000ff00) | ((value >> 24) & 0x000000ff);
}

UE_INLINE s64 ueSwapEndian(s64 value)
{
	return (s64)
#if defined(UE_WIN32)
		(((value << 56) & 0xff00000000000000) |
		((value << 40) & 0x00ff000000000000) |
		((value << 24) & 0x0000ff0000000000) |
		((value <<  8) & 0x000000ff00000000) |
#else
		(((value << 56) & 0xff00000000000000LL) |
		((value << 40) & 0x00ff000000000000LL) |
		((value << 24) & 0x0000ff0000000000LL) |
		((value <<  8) & 0x000000ff00000000LL) |
#endif
		((value >>  8) & 0x00000000ff000000) |
		((value >> 24) & 0x0000000000ff0000) |
		((value >> 40) & 0x000000000000ff00) |
		((value >> 56) & 0x00000000000000ff));
}

UE_INLINE u64 ueSwapEndian(u64 value)
{
	return (u64)
#if defined(WIN32) || defined(X86)
		(((value << 56) & 0xff00000000000000) |
		((value << 40) & 0x00ff000000000000) |
		((value << 24) & 0x0000ff0000000000) |
		((value <<  8) & 0x000000ff00000000) |
#else
		(((value << 56) & 0xff00000000000000LL) |
		((value << 40) & 0x00ff000000000000LL) |
		((value << 24) & 0x0000ff0000000000LL) |
		((value <<  8) & 0x000000ff00000000LL) |
#endif
		((value >>  8) & 0x00000000ff000000) |
		((value >> 24) & 0x0000000000ff0000) |
		((value >> 40) & 0x000000000000ff00) |
		((value >> 56) & 0x00000000000000ff));
}

UE_INLINE f32 ueSwapEndian(f32 value)
{
	const u32 tempValue = ueSwapEndian(*(u32*) &value);
	return *(const f32*) &tempValue;
}

UE_INLINE f64 ueSwapEndian(f64 value)
{
	const u64 tempValue = ueSwapEndian(*(u64*) &value);
	return *(const f64*) &tempValue;
}

#if UE_ENDIANESS == UE_ENDIANESS_LITTLE
	#define ueLittleEndian(value) (value)
	#define ueBigEndian(value) ueSwapEndian(value)
#else
	#define ueLittleEndian(value) ueSwapEndian(value)
	#define ueBigEndian(value) (value)
#endif

//! Converts given value from one endianess to another endianess
#define ueFromToEndianess(value, srcEndianess, dstEndianess) ((srcEndianess == dstEndianess) ? (value) : ueSwapEndian(value))
//! Converts given value in current endianess to given endianess
#define ueToEndianess(value, dstEndianess) ueFromToEndianess(value, UE_ENDIANESS, dstEndianess)
//! Converts given value from given endianess to current endianess
#define ueFromEndianess(value, srcEndianess) ueFromToEndianess(value, srcEndianess, UE_ENDIANESS)

// @}

#endif // UE_ENDIANESS_H

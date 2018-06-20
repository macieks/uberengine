#ifndef UE_BASIC_MATH_H
#define UE_BASIC_MATH_H

/**
 *	@addtogroup ue
 *	@{
 */

#include <math.h>
#include <float.h>

//! Power of 2
#define UE_POW2(i)						(1 << (i))
//! PI
#define UE_PI							3.141592654f
//! 2 PI
#define UE_2PI							(UE_PI * 2.0f)
//! 1.0 / PI
#define UE_INVPI						(1.0f / UE_PI)

//! Max s64 value
#define S64_MAX							((s64) 0x7FFFFFFFFFFFFFFFLL)
//! Min s64 value
#define S64_MIN							((s64) 0x8000000000000000LL)

//! Max s32 value
#define S32_MAX							((s32) 0x7FFFFFFF)
//! Min s32 value
#define S32_MIN							((s32) 0x80000000)

//! Max u64 value
#define U64_MAX							(0xFFFFFFFFFFFFFFFFLLU)
//! Max u32 value
#define U32_MAX							(0xFFFFFFFFU)
//! Max u16 value
#define U16_MAX							(0xFFFFU)
//! Max u8 value
#define U8_MAX							(0xFFU)

//! Max f32 value
#define F32_MAX							FLT_MAX
//! Min f32 value
#define F32_MIN							FLT_MIN

//! Helper union used to do bitwise convertion between u32 and f32 types
union f32_u32
{
	f32 m_f32;
	u32 m_u32;
};

//! Performs bitwise conversion from f32 to u32
UE_INLINE u32 ueF32ToU32(const f32 v)
{
	f32_u32 u;
	u.m_f32 = v;
	return u.m_u32;
}

//! Performs bitwise conversion from u32 to f32
UE_INLINE f32 ueU32ToF32(const u32 v)
{
	f32_u32 u;
	u.m_u32 = v;
	return u.m_f32;
}

//! Checks if number is power of 2
template <class TYPE>
UE_INLINE ueBool ueIsPow2(const TYPE value)
{
	return !((value - 1) & value) && value;
}

//! Aligns value to next power of 2
template <class TYPE>
UE_INLINE TYPE ueAlignPow2(const TYPE value, const TYPE alignment)
{
	UE_ASSERT( ueIsPow2(alignment) );
	return (value + alignment - 1) & ~(alignment - 1);
}

//! Aligns address to next power of 2
UE_INLINE void* ueAlignPow2(void* value, const ueSize alignment)
{
	UE_ASSERT( ueIsPow2(alignment) );
	return (void*) (((ueSize) value + alignment - 1) & ~(alignment - 1));
}

//! Tells whether value is aligned
template <class TYPE>
UE_INLINE ueBool ueIsAligned(const TYPE value, TYPE alignment) { return !(value % alignment); }

//! Tells whether pointer is aligned
UE_INLINE ueBool ueIsAligned(const void* ptr, ueSize alignment) { return !((ueSize) ptr % alignment); }

//! Adds offset to pointer
template <class TYPE>
TYPE* ueAddPtr(TYPE* ptr, u32 offset) { return (TYPE*) ((char*) ptr + offset); }

//! Gets minimum of the 2 values
template <class TYPE>
UE_INLINE TYPE ueMin(const TYPE a, const TYPE b) { return a < b ? a : b; }

//! Gets maximum of the 2 values
template <class TYPE>
UE_INLINE TYPE ueMax(const TYPE a, const TYPE b) { return a > b ? a : b; }

//! Gets minimum of the 3 values
template <typename TYPE>
inline TYPE ueMin3(const TYPE a, const TYPE b, const TYPE c) { return a < b ? (a < c ? a : c) : (b < c ? b : c); }

//! Gets maximum of the 3 values
template <typename TYPE>
inline TYPE ueMax3(const TYPE a, const TYPE b, const TYPE c) { return a > b ? (a > c ? a : c) : (b > c ? b : c); }

//! Clamps value to min/max range
template <class TYPE>
UE_INLINE TYPE ueClamp(const TYPE value, const TYPE minValue, const TYPE maxValue)
{
	return value < minValue ? minValue : (maxValue < value ? maxValue : value);
}

//! Gets square of given value
template <class TYPE>
UE_INLINE TYPE ueSqr(const TYPE value)
{
	return value * value;
}

//! Gets less or equal power of 2
UE_INLINE u32 ueLEPow2(const u32 a)
{
	u32 b = 1;
	while (b * 2 <= a) b *= 2;
	return b;
}

//! Gets greater or equal power of 2
UE_INLINE u32 ueGEPow2(u32 v)
{
#if 0
	u32 b = 1;
	while (b < v) b *= 2;
	return b;
#else
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
#endif
}

//! Gets nearest power of 2
UE_INLINE u32 ueNearestPow2(const u32 a)
{
	const u32 lePow2 = ueLEPow2(a);
	const u32 nextPow2 = lePow2 * 2;
	return ((a - lePow2) <= (nextPow2 - a)) ? lePow2 : nextPow2;
}

//! Converts degrees to radians
UE_INLINE f32 ueDegToRad(const f32 degree) { return degree * (UE_PI / 180.0f); }
//! Converts radians to degrees
UE_INLINE f32 ueRadToDeg(const f32 radian) { return radian * (180.0f / UE_PI); }

//! Gets sinus value
UE_INLINE f32 ueSin(const f32 angle) { return sinf(angle); }
//! Gets cosinus value
UE_INLINE f32 ueCos(const f32 angle) { return cosf(angle); }
//! Gets arcus cosinus value
UE_INLINE f32 ueACos(const f32 cos) { return acosf(cos); }
//! Gets arcus sinus value
UE_INLINE f32 ueASin(const f32 sin) { return asinf(sin); }
//! Gets tangens value
UE_INLINE f32 ueTan(const f32 angle) { return sinf(angle) / cosf(angle); }
//! Gets arcus tangens value
UE_INLINE f32 ueATan(const f32 angle) { return atanf(angle); }
//! Gets arcus tangens value
UE_INLINE f32 ueATan2(const f32 y, const f32 x) { return atan2f(y, x); }

//! Calculates natural logarithm of given value
UE_INLINE f32 ueLogF(const f32 a) { return logf(a); }
//! Calculates square root of given value
UE_INLINE f32 ueSqrt(const f32 a) { return sqrtf(a); }

//! Calculates power of given value
UE_INLINE f32 uePow(const f32 value, const f32 power) { return powf(value, power); }
//! Gets floor of given value
UE_INLINE f32 ueFloor(const f32 a) { return floorf(a); }
//! Gets ceiling of given value
UE_INLINE f32 ueCeil(const f32 a) { return ceilf(a); }
//! Rounds to nearest number
UE_INLINE f32 ueRound(const f32 value) { return floorf(value + 0.5f); }

//! Calculates a modulo b
UE_INLINE f32 ueMod(const f32 a, const f32 b) { return fmodf(a, b); }
//! Increases an integer value with given modulo
UE_INLINE u32 ueIncMod(u32 x, u32 mod) { return (++x == mod) ? 0 : x; }
//! Gets fractional part of the value
UE_INLINE f32 ueFrac(const f32 a) { return a - ueFloor(a); }
//! Gets sign of the value; returns 0 for 0.0f, 1.0f for positive and -1.0f for negative value
UE_INLINE f32 ueSign(const f32 value) { return value == 0.0f ? 0 : (value > 0.0f ? 1.0f : -1.0f); }

//! Lerps between 2 values
UE_INLINE f32 ueLerp(const f32 a, const f32 b, const f32 scale) { return a + (b - a) * scale; }

//! Calculates absolute value
template <typename TYPE>
UE_INLINE TYPE ueAbs(const TYPE value) { return value >= 0 ? value : -value; }

//! Calculates field of view for a given aspect ratio that preserves the same visible area as a reference aspect ratio / field of view combination
f32 ueCalcFOVForAspectRatio(const f32 aspectRatio, const f32 refFov, const f32 refAspectRatio);

//! Function that calculates 'bounce-interpolated' value (useful for e.g. popping up UI controls)
f32 ueBounce(const f32 from, const f32 to, const f32 numBounces, const f32 progress);

//! Helper function to determine which sampling interpolation parameters
UE_INLINE void ueCalcTimelineSampleParams(u32 numSamples, f32 time, u32& a, u32& b, f32& scale)
{
	UE_ASSERT(0.0f <= time && time <= 1.0f);

	if (numSamples == 1)
	{
		a = 0;
		b = 0;
		scale = 0.0f;
		return;
	}

	const f32 indexF = (f32) numSamples * time;
	const f32 aF = ueFloor(indexF);

	scale = indexF - aF;
	a = ueClamp((u32) aF, (u32) 0, numSamples);
	b = ueIncMod(a, numSamples);
}

// Bit operations

//! Counts bits set to 1
UE_INLINE u32 ueCountBits(u32 n)
{
#if 0
	u32 bits = 0;
	for (u32 i = 0; i < 32; i++)
		if (value & UE_POW2(i))
			bits++;
	return bits;
#else
	n = n - ((n >> 1) & 0x55555555);	   						// Reuse input as temporary
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);				// Temp
	return (((n + (n >> 4) & 0xF0F0F0F) * 0x1010101) >> 24);	// Count
#endif
}

//! Calculates number of trailing zeros 
UE_INLINE u32 ueCalcNumTrailingZeros(const u32 v)
{
	static const u32 MultiplyDeBruijnBitPosition[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };
	return MultiplyDeBruijnBitPosition[((u32) ((v & -(s32)v) * 0x077CB531U)) >> 27];
}

//! Tells whether bit at given index within 32-bit integer array is set to 1
UE_INLINE ueBool ueIsBitSet(const u32* bits, const u32 bitIndex)
{
	return (bits[bitIndex >> 5] & (bitIndex & 31)) != 0;
}

//! Calculates number of bits required needed to store values within 0..MAX_VALUE-1 range
template <u32 MAX_VALUE>
struct ueCalcBitsRequired
{
	enum DummyEnum { VALUE = 1 + ueCalcBitsRequired<MAX_VALUE / 2>::VALUE };
};

template <>
struct ueCalcBitsRequired<0>
{
	enum { VALUE = 0 };
};

//! An efficient iterator over bits within unsigned integer
class ueBitIterator
{
public:
	//! Initializes iterator
	UE_INLINE ueBitIterator(u32 value) : m_value(value) {}

	//! Skips to the next bit; returns UE_TRUE if next bit set to 1 is found, UE_FALSE otherwise
	UE_INLINE ueBool Next()
	{
		if (m_value)
		{
			m_value &= (m_value - 1);
			return UE_TRUE;
		}
		return UE_FALSE;
	}

	//! Gets index of the next bit set to 1; returns U32_MAX if there's no more bits set to 1
	UE_INLINE u32 NextIndex()
	{
		if (m_value)
		{
			const u32 oldValue = m_value;
			m_value &= (m_value - 1);
			return ueCalcNumTrailingZeros(oldValue ^ m_value);
		}
		return U32_MAX;
	}

private:
	u32 m_value;
};

// @}

#endif // UE_BASIC_MATH_H

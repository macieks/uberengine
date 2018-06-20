#ifndef UE_NUMERICAL_TYPES_H
#define UE_NUMERICAL_TYPES_H

#include "Base/ueMath.h"

//! Numerical types including variety of compressed ones
enum ueNumType
{
	ueNumType_F32 = 0,	//!< 32-bit float
	ueNumType_F16,		//!< 16-bit float
	ueNumType_S32,		//!< 32-bit signed integer
	ueNumType_U32,		//!< 32-bit unsigned integer
	ueNumType_S16,		//!< 16-bit signed integer
	ueNumType_U16,		//!< 16-bit unsigned integer
	ueNumType_S8,		//!< 8-bit signed integer
	ueNumType_U8,		//!< 8-bit unsigned integer
	ueNumType_U10_3,	//!< 3 10-bit unsigned integers + 2 wasted bits
	ueNumType_S10_3,	//!< 3 10-bit signed integers + 2 wasted bits

	ueNumType_MAX
};

UE_ENUM_BEGIN(ueNumType)
UE_ENUM_VALUE(ueNumType, F32)
UE_ENUM_VALUE(ueNumType, F16)
UE_ENUM_VALUE(ueNumType, S32)
UE_ENUM_VALUE(ueNumType, U32)
UE_ENUM_VALUE(ueNumType, S16)
UE_ENUM_VALUE(ueNumType, U16)
UE_ENUM_VALUE(ueNumType, S8)
UE_ENUM_VALUE(ueNumType, U8)
UE_ENUM_VALUE(ueNumType, U10_3)
UE_ENUM_VALUE(ueNumType, S10_3)
UE_ENUM_END(ueNumType)

//! Gets size (in bytes) of the value of given type
u32 ueNumType_GetTypeSize(ueNumType type, u32 count, ueBool normalized);

//! Converts an array of 32-bit floats into destination type
void ueNumType_FromF32(u32 srcCount, const f32* src,
					   ueNumType dstType, u32 dstCount, ueBool dstNormalized, void* dst, ueEndianess dstEndianess = UE_ENDIANESS);

//! Converts an array of values into 32-bit floats
void ueNumType_ToF32(ueNumType srcType, u32 srcCount, ueBool srcNormalized, const void* src,
					 u32 dstCount, f32* dst, ueEndianess dstEndianess = UE_ENDIANESS);

//! Converts an array of 32-bit integers into destination type
void ueNumType_FromS32(u32 srcCount, const s32* src,
					   ueNumType dstType, u32 dstCount, ueBool dstNormalized, void* dst, ueEndianess dstEndianess = UE_ENDIANESS);

//! Converts an array of values into 32-bit integers
void ueNumType_ToS32(ueNumType srcType, u32 srcCount, ueBool srcNormalized, const void* src,
					 u32 dstCount, s32* dst, ueEndianess dstEndianess = UE_ENDIANESS);

//! 2 32-bit floats
typedef ueVec2 f32_2;

//! 3 32-bit floats
typedef ueVec3 f32_3;

//! 4 32-bit floats
typedef ueVec4 f32_4;

//! 16-bit float
struct f16
{
	u16 P;
	UE_INLINE f16() {}
	UE_INLINE f16(f32 v)
	{
		ueNumType_FromF32(
			1, &v,
			ueNumType_F16, 1, UE_FALSE, &P);
	}
	UE_INLINE f32 AsF32() const
	{
		f32 temp;
		ueNumType_ToF32(
			ueNumType_F16, 1, UE_FALSE, &P,
			1, &temp);
		return temp;
	}
};

//! 2 16-bit floats
struct f16_2
{
	f16 P[2];
	UE_INLINE f16_2() {}
	UE_INLINE f16_2(f32 x, f32 y) { P[0] = x; P[1] = y; }
	UE_INLINE void operator = (const ueVec2& v) { P[0] = v[0]; P[1] = v[1]; }
};

//! 4 16-bit floats
struct f16_4
{
	f16 P[4];
	UE_INLINE f16_4() {}
	UE_INLINE f16_4(f32 x, f32 y, f32 z, f32 w) { P[0] = x; P[1] = y; P[2] = z; P[3] = w; }
};

//! 4 unsigned 8-bit normalized integers
struct u8_4N
{
	u32 P;
	UE_INLINE u8_4N(u32 value = 0) : P(value) {}
	UE_INLINE u8_4N(f32 x, f32 y, f32 z, f32 w)
	{
		*this = ueVec4(x, y, z, w);
	}
	UE_INLINE u8_4N(const ueVec3& xyz, f32 w)
	{
		*this = ueVec4(xyz, w);
	}
	UE_INLINE u8_4N(const ueVec3& xyz)
	{
		*this = xyz;
	}

	UE_INLINE void operator = (const ueVec3& xyz)
	{
		*this = ueVec4(xyz, 0.0f);
	}
	UE_INLINE u8_4N(const ueVec4& vec)
	{
		*this = vec;
	}
	UE_INLINE void operator = (const ueVec4& vec)
	{
		ueNumType_FromF32(
			4, (const f32*) vec,
			ueNumType_U8, 4, UE_TRUE, &P);
	}
	UE_INLINE void operator = (u32 value)
	{
		P = value;
	}
};

//! 4 unsigned 8-bit integers
struct u8_4
{
	u32 P;
	UE_INLINE u8_4() {}
	UE_INLINE u8_4(f32 x, f32 y, f32 z, f32 w)
	{
		const f32 v[4] = {x, y, z, w};
		ueNumType_FromF32(
			4, v,
			ueNumType_U8, 4, UE_FALSE, &P);
	}
	UE_INLINE u8_4(const ueVec3& xyz, f32 w)
	{
		f32 v[4];
		v[0] = xyz[0]; v[1] = xyz[1]; v[2] = xyz[2];
		v[3] = w;
		ueNumType_FromF32(
			4, v,
			ueNumType_U8,  4, UE_FALSE, &P);
	}
	UE_INLINE void operator = (const ueVec4& vec)
	{
		ueNumType_FromF32(
			4, (const f32*) vec,
			ueNumType_U8,  4, UE_FALSE, &P);
	}
};

#endif // UE_NUMERICAL_TYPES_H

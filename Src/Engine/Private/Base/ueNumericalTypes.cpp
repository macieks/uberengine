#include "Base/ueNumericalTypes.h"

#if 1

#include "d3dx9.h"

void ueF16To32Array(f32* dst, const f16* src, u32 count)
{
    D3DXFloat16To32Array(dst, (const D3DXFLOAT16*) src, count);
}

void ueF32To16Array(f16* dst, const f32* src, u32 count)
{
    D3DXFloat32To16Array((D3DXFLOAT16*) dst, src, count);
}

#else

f16 ueF32ToF16(f32 x)
{
	union {
		f32 floatI;
		u32 i;
	};
	floatI = x;

	u32 e = ((i >> 23) & 0xFF) - 112;
	u32 m =  i & 0x007FFFFF;

	u16 sh = (i >> 16) & 0x8000;
	if (e <= 0){
		// Denorm
		m = ((m | 0x00800000) >> (1 - e)) + 0x1000;
		sh |= (m >> 13);
	} else if (e == 143){
		sh |= 0x7C00;
		if (m != 0){
			// NAN
			m >>= 13;
			sh |= m | (m == 0);
		}
	} else {
		m += 0x1000;
		if (m & 0x00800000){
			// Mantissa overflow
			m = 0;
			e++;
		}
		if (e >= 31){
			// Exponent overflow
			sh |= 0x7C00;
		} else {
			sh |= (e << 10) | (m >> 13);
		}
	}

    f16 result;
    result.P = sh;
	return result;
}

f32 ueF16ToF32(f16 src)
{
    u16 sh = src.P;

	union {
		unsigned int s;
		float result;
	};

	s = (sh & 0x8000) << 16;
	unsigned int e = (sh >> 10) & 0x1F;
	unsigned int m = sh & 0x03FF;

	if (e == 0){
		// +/- 0
		if (m == 0) return result;

		// Denorm
		while ((m & 0x0400) == 0){
			m += m;
			e--;
		}
		e++;
		m &= ~0x0400;
	} else if (e == 31){
		// INF / NAN
		s |= 0x7F800000 | (m << 13);
		return result;
	}

	s |= ((e + 112) << 23) | (m << 13);

	return result;
}

void ueF16To32Array(f32* dst, const f16* src, u32 count)
{
    for (u32 i = 0; i < count; i++)
        dst[i] = ueF16ToF32(src[i]);
}

void ueF32To16Array(f16* dst, const f32* src, u32 count)
{
    for (u32 i = 0; i < count; i++)
        dst[i] = ueF32ToF16(src[i]);
}

#endif

u32 ueNumType_GetTypeSize(ueNumType type, u32 count, ueBool normalized)
{
	UE_ASSERT(1 <= count && count <= 4);
	switch (type)
	{
		case ueNumType_F32: return sizeof(f32) * count;
		case ueNumType_F16: return sizeof(f16) * count;
		case ueNumType_S32: return sizeof(s32) * count;
		case ueNumType_U32: return sizeof(u32) * count;
		case ueNumType_S16: return sizeof(s16) * count;
		case ueNumType_U16: return sizeof(u16) * count;
		case ueNumType_S8: return sizeof(s8) * count;
		case ueNumType_U8: return sizeof(u8) * count;
		case ueNumType_U10_3:
			UE_ASSERT(count == 1);
			return sizeof(u32);
		UE_INVALID_CASE(type);
	}
	return 0;
}

void ueNumType_ToF32(ueNumType srcType, u32 srcCount, ueBool srcNormalized, const void* src,
					 u32 dstCount, f32* dst, ueEndianess dstEndianess)
{
	UE_ASSERT(1 <= srcCount && srcCount <= 4);
	UE_ASSERT(1 <= dstCount && dstCount <= 4);

	const u32 count = ueMin(srcCount, dstCount);

    switch (srcType)
    {
		case ueNumType_F32:
			UE_ASSERT(!srcNormalized);
			for (u32 i = 0; i < count; i++)
				dst[i] = ((f32*) src)[i];
			break;

		case ueNumType_F16:
		{
			UE_ASSERT(!srcNormalized);
			ueF16To32Array(dst, (const f16*) src, count);
			break;
		}

		case ueNumType_S32:
			UE_ASSERT(!srcNormalized);
			for (u32 i = 0; i < count; i++)
				dst[i] = (f32) ((s32*) src)[i];
			break;

		case ueNumType_U32:
			UE_ASSERT(!srcNormalized);
			for (u32 i = 0; i < count; i++)
				dst[i] = (f32) ((u32*) src)[i];
			break;

		case ueNumType_S16:
			for (u32 i = 0; i < count; i++)
				if (srcNormalized)
				{
					s16 value = ((s16*) src)[i];
					value += (value == -32768);
					dst[i] = (f32) value * (1.0f / 32767.0f);
				}
				else
					dst[i] = (f32) ((s16*) src)[i];
			break;

		case ueNumType_U16:
			for (u32 i = 0; i < count; i++)
				if (srcNormalized)
					dst[i] = (f32) ((u16*) src)[i] * (1.0f / 65535.0f);
				else
					dst[i] = (f32) ((s16*) src)[i];
			break;

		case ueNumType_S8:
			for (u32 i = 0; i < count; i++)
				if (srcNormalized)
				{
					s8 value = ((s8*) src)[i];
					value += (value == -128);
					dst[i] = (f32) value * (1.0f / 127.0f);
				}
				else
					dst[i] = (f32) ((s8*) src)[i];
			break;

		case ueNumType_U8:
			for (u32 i = 0; i < count; i++)
				if (srcNormalized)
					dst[i] = (f32) ((u8*) src)[i] * (1.0f / 255.0f);
				else
					dst[i] = (f32) ((u8*) src)[i];
			break;

		case ueNumType_S10_3:
		{
			UE_ASSERT(srcCount == 1);
			UE_ASSERT(dstCount == 3);
			const u32 temp = *(u32*) src;
			s32 values[3] =
			{
				(temp << 22) >> 22,
				(temp << 12) >> 22,
				(temp <<  2) >> 22
			};
			if (srcNormalized)
				for (u32 i = 0; i < 3; i++)
				{
					values[i] += (values[i] == -512);
					dst[i] = (f32) values[i] * (1.0f / 511.0f);
				}
			else
				for (u32 i = 0; i < 3; i++)
					dst[i] = (f32) values[i];
			break;
		}
		case ueNumType_U10_3:
		{
			UE_ASSERT(srcCount == 1);
			UE_ASSERT(dstCount == 3);
			const u32 temp = *(u32*) src;
			if (srcNormalized)
			{
				u32 values[3] =
				{
					(temp << 22) >> 22,
					(temp << 12) >> 22,
					(temp <<  2) >> 22
				};
				for (u32 i = 0; i < 3; i++)
					dst[i] = (f32) values[i] * (1.0f / 1023.0f);
			}
			else
			{
				dst[0] = (f32) ((temp >>  0) & 0x3ff);
				dst[1] = (f32) ((temp >> 10) & 0x3ff);
				dst[2] = (f32) ((temp >> 20) & 0x3ff);
			}
			break;
		}
		UE_INVALID_CASE(srcType);
	}

	for (u32 i = srcCount; i < dstCount; i++)
		dst[i] = 0.0f;

	for (u32 i = 0; i < dstCount; i++)
		dst[i] = ueToEndianess(dst[i], dstEndianess);
}

void ueNumType_FromF32(u32 srcCount, const f32* _src,
					   ueNumType dstType, u32 dstCount, ueBool dstNormalized, void* dst, ueEndianess dstEndianess)
{
	UE_ASSERT(1 <= srcCount && srcCount <= 4);
	UE_ASSERT(1 <= dstCount && dstCount <= 4);

	f32 src[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	for (u32 i = 0; i < srcCount; i++)
		src[i] = _src[i];

	const u32 count = ueMin(srcCount, dstCount);

	switch (dstType)
    {
		case ueNumType_F32:
			UE_ASSERT(!dstNormalized);
			for (u32 i = 0; i < dstCount; i++)
				((f32*) dst)[i] = ueToEndianess(src[i], dstEndianess);
			break;

		case ueNumType_F16:
		{
			UE_ASSERT(!dstNormalized);
			f16 temp[4];
			ueF32To16Array(temp, src, dstCount);
			for (u32 i = 0; i < dstCount; i++)
				((u16*) dst)[i] = ueToEndianess(((u16*) temp)[i], dstEndianess);
			break;
		}

		case ueNumType_S8:
			for (u32 i = 0; i < dstCount; i++)
				if (dstNormalized)
					((s8*) dst)[i] = (s8) (ueClamp(src[i], 0.0f, 1.0f) * 127.0f + 0.5f);
				else
					((s8*) dst)[i] = (s8) (src[i] + 0.5f);
			break;

		case ueNumType_U8:
			for (u32 i = 0; i < dstCount; i++)
				if (dstNormalized)
					((u8*) dst)[i] = (u8) (ueClamp(src[i], 0.0f, 1.0f) * 255.0f + 0.5f);
				else
					((u8*) dst)[i] = (u8) (src[i] + 0.5f);
			break;

		case ueNumType_S16:
			for (u32 i = 0; i < dstCount; i++)
				if (dstNormalized)
					((s16*) dst)[i] = (s16) (ueClamp(src[i], 0.0f, 1.0f) * 32767.0f + 0.5f);
				else
					((s16*) dst)[i] = (s16) (src[i] + 0.5f);
			for (u32 i = 0; i < dstCount; i++)
				((s16*) dst)[i] = ueToEndianess(((s16*) dst)[i], dstEndianess);
			break;

		case ueNumType_U16:
			for (u32 i = 0; i < dstCount; i++)
				if (dstNormalized)
					((u16*) dst)[i] = (u16) (ueClamp(src[i], 0.0f, 1.0f) * 65535.0f + 0.5f);
				else
					((u16*) dst)[i] = (u16) (src[i] + 0.5f);
			for (u32 i = 0; i < dstCount; i++)
				((u16*) dst)[i] = ueToEndianess(((u16*) dst)[i], dstEndianess);
			break;

		case ueNumType_S10_3:
			UE_ASSERT(srcCount >= 3);
			UE_ASSERT(dstCount == 1);
			if (dstNormalized)
				*(u32*) dst =
					ueToEndianess(
						(((u32) (ueClamp(src[0], 0.0f, 1.0f) * 511.0f + 0.5f) & 0x3ff) <<  0) |
						(((u32) (ueClamp(src[1], 0.0f, 1.0f) * 511.0f + 0.5f) & 0x3ff) << 10) |
						(((u32) (ueClamp(src[2], 0.0f, 1.0f) * 511.0f + 0.5f) & 0x3ff) << 20), dstEndianess);
			else
				*(u32*) dst =
					ueToEndianess(
						(((u32) (src[0] + 0.5f) & 0x3ff) <<  0) |
						(((u32) (src[1] + 0.5f) & 0x3ff) << 10) |
						(((u32) (src[2] + 0.5f) & 0x3ff) << 20), dstEndianess);
			break;

		case ueNumType_U10_3:
			UE_ASSERT(srcCount >= 3);
			UE_ASSERT(dstCount == 1);
			if (dstNormalized)
				*(u32*) dst =
					ueToEndianess(
						(((u32) (ueClamp(src[0], 0.0f, 1.0f) * 1023.0f + 0.5f) & 0x3ff) <<  0) |
						(((u32) (ueClamp(src[1], 0.0f, 1.0f) * 1023.0f + 0.5f) & 0x3ff) << 10) |
						(((u32) (ueClamp(src[2], 0.0f, 1.0f) * 1023.0f + 0.5f) & 0x3ff) << 20), dstEndianess);
			else
				*(u32*) dst =
					ueToEndianess(
						(((u32) (src[0] + 0.5f) & 0x3ff) <<  0) |
						(((u32) (src[1] + 0.5f) & 0x3ff) << 10) |
						(((u32) (src[2] + 0.5f) & 0x3ff) << 20), dstEndianess);
			break;

		UE_INVALID_CASE(dstType)
    }
}

void ueNumType_ToS32(ueNumType srcType, u32 srcCount, ueBool srcNormalized, const void* src,
					 s32* dst, u32 dstCount, ueEndianess dstEndianess)
{
	UE_ASSERT(1 <= srcCount && srcCount <= 4);
	UE_ASSERT(1 <= dstCount && dstCount <= 4);

	const u32 count = ueMin(srcCount, dstCount);

	switch (srcType)
	{
		case ueNumType_U8:
			UE_ASSERT(!srcNormalized);
			for (u32 i = 0; i < count; i++)
				dst[i] = ((u8*) src)[i];
			break;
		UE_INVALID_CASE(srcType);
	}

	for (u32 i = srcCount; i < dstCount; i++)
		dst[i] = 0;
	for (u32 i = 0; i < dstCount; i++)
		dst[i] = ueToEndianess(dst[i], dstEndianess);
}

void ueNumType_FromS32(u32 srcCount, const s32* _src,
					   ueNumType dstType, u32 dstCount, ueBool dstNormalized, void* dst, ueEndianess dstEndianess)
{
	UE_ASSERT(1 <= srcCount && srcCount <= 4);
	UE_ASSERT(1 <= dstCount && dstCount <= 4);

	s32 src[4] = {0, 0, 0, 0};
	for (u32 i = 0; i < srcCount; i++)
		src[i] = _src[i];

	switch (dstType)
	{
		case ueNumType_U8:
			UE_ASSERT(!dstNormalized);
			for (u32 i = 0; i < dstCount; i++)
				((u8*) dst)[i] = (u8) src[i];
			break;
		UE_INVALID_CASE(dstType);
	}
}

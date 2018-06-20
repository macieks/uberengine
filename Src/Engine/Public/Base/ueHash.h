#ifndef UE_HASH_H
#define UE_HASH_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueBase.h"

// Crc functions

//! Calculates 32-bit based crc value of given bytes
u32 ueCalcCrc32(const void* data, ueSize size);
//! Calculates 32-bit based crc value of given NULL-terminated string
u32 ueCalcCrc32String(const char* s);

// Hashing functions

UE_INLINE u32 ueCalcHash(u8 v) { return v; }
UE_INLINE u32 ueCalcHash(s8 v) { return (u8) v; }
UE_INLINE u32 ueCalcHash(u16 v) { return v; }
UE_INLINE u32 ueCalcHash(s16 v) { return (u16) v; }
UE_INLINE u32 ueCalcHash(u32 v) { return v; }
UE_INLINE u32 ueCalcHash(s32 v) { return (u32) v; }
UE_INLINE u32 ueCalcHash(f32 v) { return ueF32ToU32(v); }
UE_INLINE u32 ueCalcHash(u64 v) { return (u32) ((v >> 32) + (v & U32_MAX)); }
UE_INLINE u32 ueCalcHash(s64 v) { return (u32) ((v >> 32) + (v & U32_MAX)); }
UE_INLINE u32 ueCalcHash(const char* s)
{
	if (!s)
		return 0;
#if 1
	u32 hash = 0;
	while (*s)
		hash = hash * 4 + *s++;
	return hash;
#else // 4-byte-at-a-time version
	const char* p = s;
	while (*p) p++;
	const u32 len = (u32) p - (u32) s;
	const u32 len4 = len & ~3;

	u32 hash = 0;
	u32 i = 0;
	while (i < len4)
	{
		hash <<= 4;
		hash += *(u32*) ((u8*) s + i);
		i += 4;
	}
	while (i < len)
	{
		hash <<= 4;
		hash += *((u8*) s + i);
		i++;
	}
	return hash;
#endif
}

UE_INLINE u32 ueCalcHash(const void* data, ueSize dataSize)
{
	u32 hash = 0;
	const u8* ptr = (u8*) data;
	for (ueSize i = 0; i < dataSize; i++)
		hash = hash * 4 + ptr[i];
	return hash;
}

UE_INLINE u32 ueCalcHashRange(const void* startAddr, const void* endAddr)
{
	return ueCalcHash(startAddr, (ueSize) endAddr - (ueSize) startAddr);
}

//! Calculates hash of memory between 2 class/struct members inclusive
#define ueCalcHashMemberRange(startMember, endMember) ueCalcHashRange(&(startMember), (const u8*) &(endMember) + sizeof(endMember))

template <class TYPE>
UE_INLINE u32 ueCalcHash(const TYPE* ptr)
{
	if (sizeof(TYPE*) == sizeof(u32))
		return (u32) ptr;

	UE_ASSERT(sizeof(TYPE*) == sizeof(u64));

	union u64_to2u32s
	{
		struct
		{
			u32 a, b;
		};
		u64 c;
	};
	u64_to2u32s u;
	u.c = (u64) ptr;
	return u.a ^ u.b;
}

template <class TYPE>
UE_INLINE u32 ueCalcHash(const TYPE& data)
{
	return ueCalcHash(&data, sizeof(TYPE));
}

// Default hash predicate

template <class TYPE>
struct ueCalcHashPredicate
{
	u32 operator () (TYPE value) const
	{
		return ueCalcHash(value);
	}
};

//! Hash container (map/set) statistics
struct ueHashContainerStats
{
	u32 m_numCollisions;			//!< Number of hash container (hashmap or hashset) collisions
	f32 m_collisionsPercentage;		//!< Percentage of hash container collisions
	u32 m_longestList;				//!< Length of the longest hash container list
};

// @}

#endif // UE_HASH_H

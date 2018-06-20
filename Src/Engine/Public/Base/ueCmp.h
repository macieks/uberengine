#ifndef UE_CMP_H
#define UE_CMP_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueBase.h"

//! Byte-wise compare
template <typename TYPE> struct ueMemCmpPredicate
{
	s32 operator () (const TYPE& a, const TYPE& b) const { return ueMemCmp(&a, &b, sizeof(TYPE)); }
};

//! Default byte-wise comparison
template <typename TYPE> struct ueCmpPredicate : ueMemCmpPredicate<TYPE> {};

//! Pointer comparison
template <typename TYPE> struct ueCmpPredicate<TYPE*>
{
	s32 operator () (const TYPE* a, const TYPE* b) const { return a == b ? 0 : (a < b ? -1 : 1); }
};

//! String comparison
template <> struct ueCmpPredicate<const char*>
{
	s32 operator () (const char* a, const char* b) const { return ueStrCmp(a, b); }
};

//! Compares any comparable types
template <typename TYPE> struct ueCmpPredicate_Comparable
{
	s32 operator () (const TYPE& a, const TYPE& b) const { return a == b ? 0 : (a < b ? -1 : 1); }
};

// Numbers

//! Compares s32 values
template <> struct ueCmpPredicate<s32> { s32 operator () (s32 a, s32 b) const { return a - b; } };
//! Compares u32 values
//template <> struct ueCmpPredicate<u32> { s32 operator () (u32 a, u32 b) const { return (s32) a - (s32) b; } };
//! Compares f32 values
template <> struct ueCmpPredicate<f32> : ueCmpPredicate_Comparable<f32> {};
//! Compares ueSize values
template <> struct ueCmpPredicate<ueSize> : ueCmpPredicate_Comparable<ueSize> {};

// Compare memory between 2 class/struct members inclusive
#define ueCmpMemberRange(startMember0, endMember0, startMember1, endMember1) ueMemCmp(&(startMember0), &(startMember1), (ueSize) &(endMember0) + sizeof(endMember0) - (ueSize) &(startMember0))

UE_INLINE s32 ueCmpPointers(const void* a, const void* b) { return a == b ? 0 : (a < b ? -1 : 1); }

// @}

#endif // UE_CMP_H

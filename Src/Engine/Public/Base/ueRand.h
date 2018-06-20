#ifndef UT_RAND_H
#define UT_RAND_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

//! Default random seed
#define UE_RAND_DEFAULT_SEED 12345678

//! Random number generator
struct ueRand;

//! Starts up random number manager
void	ueRandMgr_Startup(ueAllocator* stackAllocator, u32 maxRandoms = 1 << 4);
//! Shuts down random number manager
void	ueRandMgr_Shutdown();

//! Creates random number generator
ueRand*	ueRand_Create(u32 seed = UE_RAND_DEFAULT_SEED);
//! Destroys random number generator
void	ueRand_Destroy(ueRand* r);
//! Seeds random number generator
void	ueRand_Seed(ueRand* r, u32 seed);
//! Returns random 32 bit value
u32		ueRand_U32(ueRand* r);
//! Returns random number in range [0..maxValue] inclusive
u32		ueRand_U32(ueRand* r, u32 maxValue);
//! Returns random number in range [minValue..maxValue] inclusive
u32		ueRand_U32(ueRand* r, u32 minValue, u32 maxValue);
//! Returns random number in range [0..1]
f32		ueRand_F32(ueRand* r);
//! Returns random number in range [0..maxValue]
f32		ueRand_F32(ueRand* r, f32 maxValue);
//! Returns random number in range [minValue..maxValue]
f32		ueRand_F32(ueRand* r, f32 minValue, f32 maxValue);

// Global random number generator

//! Gets global random number generator
ueRand*			ueRand_GetGlobal();
//! Returns random 32 bit value (from global random generator)
UE_INLINE u32	ueRandG_U32() { return ueRand_U32(ueRand_GetGlobal()); }
//! Returns random number in range [0..maxValue] inclusive (from global random generator)
UE_INLINE u32	ueRandG_U32(u32 maxValue) { return ueRand_U32(ueRand_GetGlobal(), maxValue); }
//! Returns random number in range [minValue..maxValue] inclusive (from global random generator)
UE_INLINE u32	ueRandG_U32(u32 minValue, u32 maxValue) { return ueRand_U32(ueRand_GetGlobal(), minValue, maxValue); }
//! Returns random number in range [0..1] (from global random generator)
UE_INLINE f32	ueRandG_F32() { return ueRand_F32(ueRand_GetGlobal()); }
//! Returns random number in range [0..maxValue] (from global random generator)
UE_INLINE f32	ueRandG_F32(f32 maxValue) { return ueRand_F32(ueRand_GetGlobal(), maxValue); }
//! Returns random number in range [minValue..maxValue] (from global random generator)
UE_INLINE f32	ueRandG_F32(f32 minValue, f32 maxValue) { return ueRand_F32(ueRand_GetGlobal(), minValue, maxValue); }

// @}

#endif // UT_RAND_H

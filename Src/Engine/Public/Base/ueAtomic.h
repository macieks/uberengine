#ifndef UE_ATOMIC_H
#define UE_ATOMIC_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Atomic CAS (compare-and-swap) operation - if dst equals cmp, then dst is set to exch; returns UE_TRUE on success, UE_FALSE otherwise
UE_INLINE ueBool ueAtomic_CAS(volatile u32* dst, u32 cmp, u32 exch);

#if defined(UE_WIN32) || defined(UE_X360)

UE_INLINE ueBool ueAtomic_CAS(volatile u32* dst, u32 cmp, u32 exch)
{
	return InterlockedCompareExchange((volatile LONG*) dst, exch, cmp) != exch;
}

#elif defined(UE_LINUX)

UE_INLINE ueBool ueAtomic_CAS(volatile u32* dst, u32 cmp, u32 exch)
{
#if 0
    static ueMutex mutex;
	ueMutexLock lock(mutex); // TEMP: Testing on linux
    if (*dst == cmp)
    {
        *dst = exch;
        return UE_TRUE;
    }
	return UE_FALSE;
#else
	#include <libkern/OSAtomic.h>
	return OSAtomicCompareAndSwap32(cmp, exch, dst);
#endif
}

#else

UE_INLINE ueBool ueAtomic_CAS(volatile u32* dst, u32 cmp, u32 exch)
{
	if (*dst == cmp)
    {
        *dst = exch;
        return UE_TRUE;
    }
	return UE_FALSE;
}

#endif

// @}

#endif // UE_ATOMIC_H

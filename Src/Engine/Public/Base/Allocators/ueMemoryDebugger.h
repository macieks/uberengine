#ifndef UE_MEMORY_DEBUGGER_H
#define UE_MEMORY_DEBUGGER_H

#include "Base/ueBase.h"

#if defined(UE_ENABLE_MEMORY_DEBUGGING)

class ueAllocator;
struct ueMemoryDebugger;

static const u32 UE_DEFAULT_MAX_TRACKED_ALLOCS = 4096;
static const u32 UE_DEFAULT_CALLSTACK_DEPTH = 12;

//! Creates memory debugger
ueMemoryDebugger* ueMemoryDebugger_Create(ueAllocator* allocator, u32 maxTrackedAllocs = UE_DEFAULT_MAX_TRACKED_ALLOCS, u32 callstackDepth = UE_DEFAULT_CALLSTACK_DEPTH);
//! Destroys memory debugger
void ueMemoryDebugger_Destroy(ueMemoryDebugger* md);

//! Starts memory tracking
void ueMemoryDebugger_Begin(ueMemoryDebugger* md);
//! Ends memory tracking
void ueMemoryDebugger_End(ueMemoryDebugger* md);

//! Adds allocator to be tracked
void ueMemoryDebugger_AddAllocatorToTrack(ueMemoryDebugger* md, ueAllocator* allocator);
//! Removes allocator from tracking
void ueMemoryDebugger_RemoveAllocatorToTrack(ueMemoryDebugger* md, ueAllocator* allocator);

//! Gets number of unfreed allocation for a given allocator (use NULL for all allocators)
u32 ueMemoryDebugger_GetNumUnfreedAllocations(ueMemoryDebugger* md, ueAllocator* allocator = NULL, ueBool mutexLocked = UE_TRUE);
//! Dumps unfreed allocation for a given allocator (use NULL for all allocators); dumps up to maxItems leaks
void ueMemoryDebugger_DumpUnfreedAllocations(ueMemoryDebugger* md, ueAllocator* allocator = NULL, u32 maxItems = 20);

#endif // defined(UE_ENABLE_MEMORY_DEBUGGING)

#endif // UE_MEMORY_DEBUGGER_H

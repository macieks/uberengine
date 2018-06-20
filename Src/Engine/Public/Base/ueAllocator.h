#ifndef UE_ALLOCATOR_H
#define UE_ALLOCATOR_H

class ueAllocator;
struct ueMutex;

//! Flags indicating which stats are provided by the allocator
enum ueAllocatorStatsFlags
{
	ueAllocatorStatsFlags_HasUsed			= 1 << 0,	//!< Has current used bytes count
	ueAllocatorStatsFlags_HasUsedPeak		= 1 << 1,	//!< Has peak used bytes count
	ueAllocatorStatsFlags_HasLargestBlock	= 1 << 2,	//!< Has largest allocated block
	ueAllocatorStatsFlags_HasTotal			= 1 << 3	//!< Has total size
};

//! Allocator statistics; which stats are provided for a specific allocator is indicated by flags
struct ueAllocatorStats
{
	u32 m_flags;			//!< Flags indicating what data is contained in the stats; @see enum ueAllocatorStatsFlags

	ueSize m_used;			//!< Number of allocated bytes; available only if Flags_HasUsed is set
	ueSize m_usedPeak;		//!< Peal number of allocated bytes; available only if Flags_HasUsedPeak is set

	ueSize m_largestBlock;	//!< Largest free block; available only if Flags_HasLargestBlock is set
	ueSize m_total;			//!< Total number of bytes managed by this allocator; available only if Flags_HasTotal is set

	u32 m_numAllocs;		//!< Number of allocations
	u32 m_numAllocsPeak;	//!< Peak number of allocations

	ueAllocatorStats();
	void Reset();

	void AddAlloc(ueSize bytes);
	void RemoveAlloc(ueSize bytes);

	void UpdateAlloc(ueSize oldSize, ueSize newSize);
};

//! Allocator's reallocation function
typedef void* (*ueAllocatorFunc)(ueAllocator* allocator, void* memory, ueSize size, ueSize alignment, void* allocInfo);
//! Allocator's reallocation callback; handles alloc (when !memory && size), free (when memory && !size) and realloc (when memory && size)
typedef void (*ueAllocatorCallback)(ueAllocator* allocator, void* memory, ueSize size, ueSize alignment, void* result, void* userData);

//! Base allocator
class ueAllocator
{
public:
	ueAllocator(ueBool isThreadSafe = UE_DEFAULT_THREAD_SAFE);
	~ueAllocator();

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	//! Adds memory reallocation listener
	void AddListener(ueAllocatorCallback callback, void* userData);
	//! Removes memory reallocation listener
	void RemoveListener(ueAllocatorCallback callback, void* userData);

	//! Gets allocator name
	const char* GetName() const { return m_name; }
	//! Sets allocator name
	void SetName(const char* name);
#endif

	//! Gets allocator statistics
	void GetStats(ueAllocatorStats* stats);
	//! Tells whether there's any unfreed allocation
	ueBool HasAnyAllocation();

	//! Sets whether the allocator is to be thread safe; default bahviour is determined based on UE_DEFAULT_THREAD_SAFE macro
	void SetThreadSafe(ueBool isThreadSafe);

	//! Reallocates memory (also frees or allocates depending on params)
	void* Realloc(void* memory, ueSize size, ueSize alignment = UE_DEFAULT_ALIGNMENT, void* allocInfo = NULL);
	//! Allocates memory
	UE_INLINE void* Alloc(ueSize size, ueSize alignment = UE_DEFAULT_ALIGNMENT, void* allocInfo = NULL) { return Realloc(NULL, size, alignment, allocInfo); }
	//! Frees memory
	UE_INLINE void Free(void* data, void* allocInfo = NULL) { Realloc(data, 0, 0, allocInfo); }

#if defined(UE_TOOLS)
	//! Gets global (malloc based) allocator; for use by tools only
	static ueAllocator* GetGlobal();
#endif // defined(UE_TOOLS)

private:
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	void _DebugOnRealloc(void* memory, ueSize size, ueSize alignment, void* result);
#endif // defined(UE_ENABLE_MEMORY_DEBUGGING)

protected:
	ueAllocatorFunc m_reallocFunc;

	ueMutex* m_mutex;
	ueBool m_isThreadSafe;

#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	char m_name[16];

	u32 m_numListeners;
	struct Listener
	{
		ueAllocatorCallback m_callback;
		void* m_userData;
	};
	Listener m_listeners[4];
#endif

	ueAllocatorStats m_stats;
};

#define UE_DECLARE_ALLOC_FUNCS(type) \
private: \
	void* _Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo); \
	static void* _ReallocFunc(ueAllocator* allocator, void* memory, ueSize size, ueSize alignment, void* allocInfo) { return ((type*) allocator)->_Realloc(memory, size, alignment, allocInfo); }

#endif // UE_ALLOCATOR_H

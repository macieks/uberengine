#ifndef UE_PROFILER_H
#define UE_PROFILER_H

/**
 *	@addtogroup ue
 *	@{
 */

#if defined(UE_ENABLE_PROFILER)

//! Profiler flags
enum ueProfilerFlags
{
	ueProfilerFlags_Engine		= UE_POW2(0),	//!< Use engine's profiling
#if defined(UE_WIN32)
	ueProfilerFlags_PIX			= UE_POW2(1),	//!< Use PIX
	ueProfilerFlags_NvPerfHud	= UE_POW2(2),	//!< Use NvPerfHud profiling
#elif defined(UE_X360)
	ueProfilerFlags_PIX			= UE_POW2(1)	//!< Use PIX
#endif
};

struct ueProfilerCapture;

//! "Compressed" profiler data - use ueProfiler_BuildCapture to extract readable data
struct ueProfilerData
{
	void* m_buffer;		//!< Buffer
	u32 m_bufferSize;	//!< Buffer size
};

// Profiler

//! Starts up profiler
void		ueProfiler_Startup(ueAllocator* stackAllocator, u32 bufferSize = 1 << 18, u32 maxNamedScopes = 1 << 12, u32 captureBufferSize = 1 << 18);
//! Shuts down profiler
void		ueProfiler_Shutdown();

//! Resets profilier for the next frame; to be called by the user at the beginning of each new frame
const ueProfilerData*		ueProfiler_NextFrame();
//! Builds profiler capture (based on stats from the last frame) and returns it
const ueProfilerCapture*	ueProfiler_BuildCapture();
//! Builds profiler capture (based on given stats) and returns it
ueProfilerCapture*			ueProfiler_BuildCapture(void* memory, u32 memorySize, const ueProfilerData* data);

//! Returns scope id; if scope wasn't created before it will be created; for flags see ueProfilerFlags
u32			ueProfiler_GetScopeId(const char* name, u32 flags = U32_MAX);
//! Gets the name of the scope with given id
const char*	ueProfiler_GetScopeName(u32 id);
//! Pushes given scope onto the stack
void		ueProfiler_Push(u32 id);
//! Pushes dynamic (i.e. having dynamically created name, e.g. "rendering of Mesh_148") scope onto the stack
void		ueProfiler_PushDynamic(const char* name);
//! Pops given scope from the stack
void		ueProfiler_Pop();

//! Scope profiling helper
class ueScopeProfiler
{
public:
	//! Pushes scope onto the stack
	UE_INLINE ueScopeProfiler(u32 id) { ueProfiler_Push(id); }
	//! Pushes scope onto the stack
	UE_INLINE ueScopeProfiler(const char* name) { ueProfiler_PushDynamic(name); }
	//! Pops scope from the stack
	UE_INLINE ~ueScopeProfiler() { ueProfiler_Pop(); }
};

//! Creates profiled scope
#define UE_PROF_SCOPE(name) UE_PROF_SCOPE_EX(name, U32_MAX)

//! Creates profiled scope (with profiler flags - see ueProfilerFlags)
#define UE_PROF_SCOPE_EX(name, flags) \
	static const u32 UE_GEN_NAME(profilerScopeId) = ueProfiler_GetScopeId(name, flags); \
	ueScopeProfiler UE_GEN_NAME(scopeProfiler)(UE_GEN_NAME(profilerScopeId));

//! Creates profiled dynamic scope
#define UE_PROF_SCOPE_DYNAMIC(name) ueScopeProfiler UE_GEN_NAME(scopeProfilerDynamic)(name);

#else // defined(UE_ENABLE_PROFILER)

#define UE_PROF_SCOPE(name)
#define UE_PROF_SCOPE_EX(name, flags)
#define UE_PROF_SCOPE_DYNAMIC(name)

#endif // defined(UE_ENABLE_PROFILER)

// @}

#endif // UE_PROFILER_H

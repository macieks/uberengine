#ifndef GX_PROFILER_H
#define GX_PROFILER_H

#include "Base/ueBase.h"

struct ueResource;
struct ueProfilerCapture;
struct glCtx;

/**
 *	@addtogroup gx
 *	@{
 */

//! Callback for resource visualization
typedef void (*gxResourceVisualizationCallback)(ueResource* resource, void* userData);

//! Visual profiler startup parameters
struct gxProfilerStartupParams
{
	u32 m_maxAllocators;			//!< Max. number of allocators to track
	f32 m_inputPriority;			//!< Input priority for visual debugger

	gxProfilerStartupParams() :
		m_maxAllocators(16),
		m_inputPriority(GX_DEFAULT_PROFILER_INPUT_PRIORITY)
	{}
};

//! Starts up environment for visual profiler
void gxProfiler_StartupEnv(ueAllocator* stackAllocator);
//! Starts up visual profiler
void gxProfiler_Startup(gxProfilerStartupParams* params);
//! Shuts down visual profiler
void gxProfiler_Shutdown();

//! Updates visual profiler
void gxProfiler_Update(f32 dt);

//! Toggles visual profiler (rendering and update does nothing when disabled)
void gxProfiler_Enable(ueBool enable);
//! Tells whether visual profiler is enabled
ueBool gxProfiler_IsEnabled();

//! Toggles visual profiler input
void gxProfiler_EnableInput(ueBool enable);
//! Tells whether input is enabled for visual profiler
ueBool gxProfiler_IsInputEnabled();

// Allocator visualization

//! Adds allocator to visual profiler
void gxProfiler_AddAllocator(ueAllocator* allocator);
//! Removes allocator from visual profiler
void gxProfiler_RemoveAllocator(ueAllocator* allocator);

// Profiling data visualization

//! Sets profiler capture for the profiler
void gxProfiler_SetProfilerCapture(const ueProfilerCapture* profCapture);
//! Tells whether visual profiler is awaiting new profiler capture (based on configured frequency)
ueBool gxProfiler_IsWaitingForProfilerCapture();

// Resource visualization

//! Sets resource visualization callback for a given resource type
void gxProfiler_SetResourceVisualizationCallback(u32 resourceType, gxResourceVisualizationCallback callback, void* userData);

// @}

#endif // GX_PROFILER_H

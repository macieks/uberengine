#ifndef UE_PROFILER_CAPTURE_H
#define UE_PROFILER_CAPTURE_H

#include "Base/ueThreading.h"

//! Profiled node; all times are just 32-bit - not 64-bit - because they're relative, hence it's enough to store up to 4 secs there
struct ueProfilerNode
{
	const char* m_name;		//!< Profiler node name

	u32 m_enterNanoSecs;	//!< Enter / start time (offset from root node's enter time)
	u32 m_exclNanoSecs;		//!< Time excluding children
	u32 m_inclNanoSecs;		//!< Time including children

	ueProfilerNode* m_firstChild;	//!< Pointer to first child node
	ueProfilerNode* m_nextSibling;	//!< Pointer to next sibling node
};

//! Profiling tree of specific thread
struct ueProfilerTree
{
	ueThreadId m_threadId;		//!< Thread id
	ueProfilerNode* m_root;		//!< Root profiling node
	ueBool m_isValid;			//!< Indicates whether all "pushes" are matched by corresponding "pops"
};

//! Complete capture of a single frame
struct ueProfilerCapture
{
	ueTime m_startTime;			//!< Start time of the whole capture

	u32 m_numTrees;				//!< Number of profiler trees (also, number of profiled threads)
	ueProfilerTree* m_trees;	//!< An array of profiler trees - one for each thread
};

#endif // UE_PROFILER_CAPTURE_H

#ifndef UE_JOB_SYS_H
#define UE_JOB_SYS_H

#include "Base/ueAsync.h"

/**
 *	@addtogroup th
 *	@{
 */

/**
 *	@struct ueJobSys
 *	@brief FIFO queue based asynchronous job system
 */
struct ueJobSys;

//! Job work function callback invoked when job is about to be run
typedef void (*ueJob_WorkFunc)(ueAsync* async, void* userData);
//! Job done callback invoked when job is finished
typedef void (*ueJob_DoneFunc)(ueAsync* async, ueAsyncState state, void* userData);
//! Job destroy callback invoked when job is about to be destroyed
typedef void (*ueJob_DestroyFunc)(ueAsync* async, void* userData);

//! Job description
struct ueJobDesc
{
	ueJob_WorkFunc m_workFunc;		//!< Mandatory job work function

	ueJob_DoneFunc m_doneFunc;		//!< Optional callback invoked after job finishes
	ueBool m_enableAsyncCallback;	//!< Valid only when m_doneFunc is not NULL

	ueJob_DestroyFunc m_destroyFunc;//!< Optional function to be invoked on job destruction

	ueBool m_enableAutoRelease;		//!< Indicates whether the job shall be automatically destroyed when done; otherwise ueAsync_Destroy must be called by the user for a job that's not needed anymore

	void* m_userData;				//!< Custom user data

	ueJobDesc() :
		m_workFunc(NULL),
		m_doneFunc(NULL),
		m_enableAsyncCallback(UE_TRUE),
		m_enableAutoRelease(UE_FALSE),
		m_destroyFunc(NULL),
		m_userData(NULL)
	{}
};

//! Job system worker thread description
struct ueJobSysThreadDesc
{
	const char* m_name;				//!< Thread name
	u32 m_processor;				//!< Processor index
	f32 m_priority;					//!< Thread priority

	ueJobSysThreadDesc() :
		m_name("ue_job_sys"),
		m_processor(0),
		m_priority(0.5f)
	{}
};

//! Job system creation parameters
struct ueJobSysParams
{
	ueAllocator* m_stackAllocator;		//!< Prefarably stack allocator

	u32 m_maxJobs;						//!< Max. number of jobs

	u32 m_numThreads;					//!< Number of job system threads
	ueJobSysThreadDesc* m_threadDescs;	//!< Array of descriptions of all worked threads

	ueJobSysParams() :
		m_stackAllocator(NULL),
		m_maxJobs(16),
		m_numThreads(0),
		m_threadDescs(NULL)
	{}
};

//! Creates job system
ueJobSys*	ueJobSys_Create(ueJobSysParams* params);
//! Destroys job system
void		ueJobSys_Destroy(ueJobSys* sys);

//! Starts job system
void		ueJobSys_Start(ueJobSys* sys);
//! Pauses or unpauses job system
void		ueJobSys_Pause(ueJobSys* sys, ueBool enable);

//! Starts job on a given job system
ueAsync*	ueJobSys_StartJob(ueJobSys* sys, ueJobDesc* desc);
//! Starts job on a given job system
ueAsync*	ueJobSys_StartJob(ueJobSys* sys, ueJob_WorkFunc workFunc, void* userData = NULL);

//! Waits for all jobs to finish
void		ueJobSys_WaitAllJobs(ueJobSys* sys);
//! Destroys all jobs; when block is UE_TRUE this function blocks until all jobs are fully destroyed
void		ueJobSys_DestroyAllJobs(ueJobSys* sys, ueBool block = UE_TRUE);

//! Invokes synchronous "done" job callbacks
void		ueJobSys_InvokeJobCallbacks(ueJobSys* sys);

//! Sets data available via ueAsync_GetData() funtion for that async job
void		ueJob_SetAsyncData(ueAsync* async, void* data);

// Global job system

//! Starts up global job system
void		thGlobalJobSys_Startup(ueJobSysParams* params);
//! Shuts down global job system
void		thGlobalJobSys_Shutdown();
//! Starts job on a global job system
ueAsync*	thGlobalJobSys_StartJob(ueJobDesc* desc);
//! Starts job on a global job system
ueAsync*	thGlobalJobSys_StartJob(ueJob_WorkFunc workFunc, void* userData = NULL);

// @}

#endif // UE_JOB_SYS_H

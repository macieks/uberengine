#include "Base/ueJobSys.h"
#include "Base/ueAsync_Private.h"
#include "Base/ueThreading.h"
#include "Base/Containers/ueGenericPool.h"
#include "Base/Containers/ueList.h"

//#define UE_JOB_SYS_VERBOSE

struct ueJob : ueAsync, ueList<ueJob>::Node
{
	ueJobSys* m_jobSys;

	ueJob_WorkFunc m_workFunc;

	ueJob_DoneFunc m_doneFunc;
	ueBool m_enableAsyncCallback;
	ueBool m_doneFuncInvoked;

	ueJob_DestroyFunc m_userDestroyFunc;

	ueBool m_enableAutoRelease;

	volatile ueBool m_isWaiting;
	volatile ueBool m_isCancelled;

	ueEvent* m_event;

	void* m_asyncData;

	UE_INLINE ueJob() :
		m_isWaiting(UE_TRUE),
		m_isCancelled(UE_FALSE),
		m_doneFuncInvoked(UE_FALSE),
		m_asyncData(NULL),
		m_event(NULL)
	{}
};

struct ueJobWorker : ueList<ueJobWorker>::Node
{
	ueJobSys* m_sys;
	ueThreadId m_thread;
	ueJob* m_job;
	ueEvent* m_event;

#if defined(UE_JOB_SYS_VERBOSE)
	u32 m_index;
#endif

	ueJobWorker() :
		m_sys(NULL),
		m_thread(0),
		m_job(NULL),
		m_event(NULL)
	{}
};

struct ueJobSys
{
	ueAllocator* m_allocator;

	ueMutex* m_mutex;

	ueList<ueJob> m_doneJobs;
	ueList<ueJob> m_waitingJobs;

	ueGenericPool m_jobsPool;

	u32 m_numJobWorkers;
	ueJobWorker* m_jobWorkers;
	ueList<ueJobWorker> m_freeWorkers;
	ueList<ueJobWorker> m_usedWorkers;

	volatile ueBool m_quit;

	ueBool m_isPaused;
	ueBool m_isStarted;
};

void ueJob_Wait(ueJob* job, u32 millisecs);
void ueJob_Destroy(ueJob* job);

void ueJobSys_WorkerFunc(void* userData);

// ueJobSys

ueJobSys* ueJobSys_Create(ueJobSysParams* params)
{
	UE_ASSERT(params->m_stackAllocator);
	UE_ASSERT(params->m_numThreads > 0);
	UE_ASSERT(params->m_stackAllocator);
	UE_ASSERT(params->m_maxJobs > 0);

	// Allocate single memory block

	const ueSize jobsPoolSize = ueGenericPool::CalcMemReq(sizeof(ueJob), params->m_maxJobs);
	const ueSize memorySize = sizeof(ueJobSys) + params->m_numThreads * sizeof(ueJobWorker) + jobsPoolSize;
	u8* memory = (u8*) params->m_stackAllocator->Alloc(memorySize);
	if (!memory)
		return NULL;

	// Create job sys data

	ueJobSys* sys = new(memory) ueJobSys();
	memory += sizeof(ueJobSys);
	sys->m_allocator = params->m_stackAllocator;
	sys->m_quit = UE_FALSE;
	sys->m_isStarted = UE_FALSE;
	sys->m_isPaused = UE_FALSE;
	sys->m_mutex = ueMutex_Create();

	// Initialize threads

	sys->m_numJobWorkers = params->m_numThreads;
	sys->m_jobWorkers = (ueJobWorker*) memory;
	memory += sys->m_numJobWorkers * sizeof(ueJobWorker);

	for (u32 i = 0; i < sys->m_numJobWorkers; i++)
	{
		ueJobWorker* worker = new(sys->m_jobWorkers + i) ueJobWorker();
		worker->m_sys = sys;
		worker->m_event = ueEvent_Create();

#if defined(UE_JOB_SYS_VERBOSE)
		worker->m_index = i;
#endif

		ueThreadParams threadParams;
		threadParams.m_userData = &sys->m_jobWorkers[i];
		threadParams.m_priority = params->m_threadDescs[i].m_priority;
		threadParams.m_name = params->m_threadDescs[i].m_name;
		threadParams.m_processor = params->m_threadDescs[i].m_processor;
		threadParams.m_func = ueJobSys_WorkerFunc;
		worker->m_thread = ueThread_Create(&threadParams);
		UE_ASSERT(worker->m_thread);

		sys->m_freeWorkers.PushFront(worker);
	}

	// Initialize jobs

	sys->m_jobsPool.InitMem(memory, jobsPoolSize, sizeof(ueJob), params->m_maxJobs, 4);

	return sys;
}

void ueJobSys_Destroy(ueJobSys* sys)
{
	// Let all workers know to quit

#if defined(UE_JOB_SYS_VERBOSE)
	ueLogD("Signaling all %d workers (to quit)", sys->m_numJobWorkers);
#endif

	sys->m_quit = UE_TRUE;
	for (u32 i = 0; i < sys->m_numJobWorkers; i++)
		ueEvent_Signal(sys->m_jobWorkers[i].m_event);

	// Wait for all job workers to exit

#if defined(UE_JOB_SYS_VERBOSE)
	ueLogD("Waiting for all worker threads to quit");
#endif

	// Destroy job workers

	for (u32 i = 0; i < sys->m_numJobWorkers; i++)
	{
		ueJobWorker* worker = sys->m_jobWorkers + i;

		ueThread_Join(worker->m_thread);
		ueEvent_Destroy(worker->m_event);

		sys->m_freeWorkers.Remove(worker);
		worker->~ueJobWorker();
	}

#if defined(UE_JOB_SYS_VERBOSE)
	ueLogD("All worker threads quited");
#endif

	// Finish all jobs in progress

	ueJobSys_DestroyAllJobs(sys, UE_TRUE);
	ueMutex_Destroy(sys->m_mutex);

	// Deallocate job system memory

	ueDelete(sys, sys->m_allocator);
}

void ueJobSys_Start(ueJobSys* sys)
{
	UE_ASSERT(!sys->m_isStarted);
	sys->m_isStarted = UE_TRUE;

	for (u32 i = 0; i < sys->m_numJobWorkers; i++)
		ueThread_Start(sys->m_jobWorkers[i].m_thread);
}

void ueJobSys_Pause(ueJobSys* sys, ueBool pause)
{
	UE_ASSERT(sys->m_isStarted);
	sys->m_isPaused = pause;
}

ueAsync* ueJobSys_StartJob(ueJobSys* sys, ueJob_WorkFunc workFunc, void* userData)
{
	ueJobDesc desc;
	desc.m_workFunc = workFunc;
	desc.m_userData = userData;
	return ueJobSys_StartJob(sys, &desc);
}

void ueJob_SetAsyncData(ueAsync* async, void* data)
{
	ueJob* job = (ueJob*) async;
	job->m_asyncData = data;
}

void* ueJob_GetData(ueJob* job)
{
	return job->m_asyncData;
}

ueAsync* ueJobSys_StartJob(ueJobSys* sys, ueJobDesc* desc)
{
	UE_ASSERT(sys->m_isStarted);

	ueMutexLock lock(sys->m_mutex);

	// Create job

	ueJob* job = new(sys->m_jobsPool) ueJob();
	if (!job)
		return NULL;

	// Set up async

	job->m_getDataFunc = (ueAsync_GetDataFunc) ueJob_GetData;
	job->m_destroyFunc = (ueAsync_DestroyFunc) ueJob_Destroy;
	job->m_waitFunc = (ueAsync_WaitFunc) ueJob_Wait;

	// Set up job

	job->m_jobSys = sys;
	job->m_workFunc = desc->m_workFunc;
	job->m_doneFunc = desc->m_doneFunc;
	job->m_enableAsyncCallback = desc->m_enableAsyncCallback;
	job->m_enableAutoRelease = desc->m_enableAutoRelease;
	job->m_userDestroyFunc = desc->m_destroyFunc;
	job->m_userData = desc->m_userData;

	// Run the job immediately if there is free worker

	if (!sys->m_freeWorkers.IsEmpty())
	{
		ueJobWorker* worker = sys->m_freeWorkers.PopBack();
		sys->m_usedWorkers.PushFront(worker);

#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Giving job to worker %d", worker->m_index);
#endif

		job->m_state = ueAsyncState_InProgress;
		job->m_isWaiting = UE_FALSE;

		worker->m_job = job;
		ueEvent_Signal(worker->m_event);
	}
	else
	{
#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Putting job onto wait queue (no available workers, jobs left = %d)", sys->m_waitingJobs.Length());
#endif

		sys->m_waitingJobs.PushFront(job);
	}

	return job;
}

void ueJobSys_InvokeJobCallbacks(ueJobSys* sys)
{
	UE_ASSERT(sys->m_isStarted);

	ueMutexLock lock(sys->m_mutex);

	ueJob* job = sys->m_doneJobs.Front();
	while (job)
	{
		// Store next to allow the job to be released by the user while iterating
		ueJob* nextJob = job->Next();

		if (job->m_doneFunc && !job->m_doneFuncInvoked)
		{
			job->m_doneFuncInvoked = UE_TRUE;
			job->m_doneFunc(job, job->m_state, job->m_userData);
		}
		job = nextJob;
	}
}

void ueJobSys_WaitAllJobs(ueJobSys* sys)
{
	UE_ASSERT(sys->m_isStarted);

#if defined(UE_JOB_SYS_VERBOSE)
    {
        ueMutexLock lock(sys->m_mutex);
        ueLogD("Waiting for all pending jobs to finish (%d in progress, %d waiting)", sys->m_usedWorkers.Length(), sys->m_waitingJobs.Length());
    }
#endif

	while (1)
	{
		{
			ueMutexLock lock(sys->m_mutex);
			if (sys->m_waitingJobs.IsEmpty() && sys->m_usedWorkers.IsEmpty())
				return;
		}

		ueThread_Yield();
	}

#if defined(UE_JOB_SYS_VERBOSE)
	ueLogD("Waiting for all jobs finished");
#endif
}

void ueJobSys_DestroyAllJobs(ueJobSys* sys, ueBool block)
{
	UE_ASSERT(sys->m_isStarted);

	{
		ueMutexLock lock(sys->m_mutex);

#if defined(UE_JOB_SYS_VERBOSE)
        ueLogD("Releasing all jobs (waiting = %d, done = %d)", sys->m_waitingJobs.Length(), sys->m_doneJobs.Length());
#endif

		ueJob* job = NULL;
		while (job = sys->m_waitingJobs.Front())
		{
			sys->m_waitingJobs.Remove(job);
			ueDelete(job, sys->m_jobsPool);
		}

		while (job = sys->m_doneJobs.Front())
		{
			sys->m_doneJobs.Remove(job);
			ueDelete(job, sys->m_jobsPool);
		}
	}

#if defined(UE_JOB_SYS_VERBOSE)
	ueLogD("Released done and waiting jobs");
#endif

	if (block)
		ueJobSys_WaitAllJobs(sys);
}

// Job Worker

void ueJobSys_WorkerFunc(void* userData)
{
	ueJobWorker* worker = (ueJobWorker*) userData;
	ueJobSys* sys = worker->m_sys;

	while (1)
	{
#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Worker %d waiting for being signaled", worker->m_index);
#endif

		// Wait for an event (new job or quit request)

		ueEvent_Wait(worker->m_event);

#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Worker %d was signaled", worker->m_index);
#endif

		// Check whether to quit
		{
			ueMutexLock lock(sys->m_mutex);
			if (sys->m_quit && !worker->m_job) // Quit only if there is no job to be done
			{
#if defined(UE_JOB_SYS_VERBOSE)
				ueLogD("Worker %d quitting", worker->m_index);
#endif
				break;
			}
		}

#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Worker %d starting job", worker->m_index);
#endif

		// Execute the job

		ueJob* job = worker->m_job;
		job->m_workFunc(job, job->m_userData);

#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Worker %d finished job", worker->m_index);
#endif

		// Finalize the job and grab new one
		{
			if (job->m_state == ueAsyncState_InProgress) // Only set "succeeded" state if user didn't set any of the 2: "success" or "failure"
				job->m_state = ueAsyncState_Succeeded;

			ueEvent_Signal(job->m_event);

			if (job->m_doneFunc && job->m_enableAsyncCallback)
				job->m_doneFunc(job, job->m_state, job->m_userData);

			ueMutexLock lock(sys->m_mutex);

			worker->m_job = NULL;
			if ((job->m_enableAutoRelease && (!job->m_doneFunc || job->m_enableAsyncCallback)) ||
				job->m_isCancelled)
			{
				if (job->m_userDestroyFunc)
					job->m_userDestroyFunc(job, job->m_userData);
				ueDelete(job, sys->m_jobsPool);
			}
			else if (job->m_doneFunc)
				sys->m_doneJobs.PushFront(job);

			// No jobs waiting? - put our worker into waiting state

			if (sys->m_isPaused || sys->m_quit || sys->m_waitingJobs.IsEmpty())
			{
#if defined(UE_JOB_SYS_VERBOSE)
				ueLogD("Worker %d going to sleep", worker->m_index);
#endif
				sys->m_usedWorkers.Remove(worker);
				sys->m_freeWorkers.PushFront(worker);
				continue;
			}

#if defined(UE_JOB_SYS_VERBOSE)
		ueLogD("Worker %d getting new job", worker->m_index);
#endif
			// Grab new job if available

			worker->m_job = sys->m_waitingJobs.PopBack(); // This must succeed
			UE_ASSERT(worker->m_job);
			ueEvent_Signal(worker->m_event); // Let worker pass through Wait() at the top of this while loop
		}
	}
}

void ueJob_Destroy(ueJob* job)
{
	ueJobSys* sys = job->m_jobSys;
	ueMutexLock lock(sys->m_mutex);

	switch (job->m_state)
	{
		case ueAsyncState_InProgress:
			if (job->m_isWaiting)
			{
				if (job->m_userDestroyFunc)
					job->m_userDestroyFunc(job, job->m_userData);
				sys->m_waitingJobs.Remove(job);
				ueDelete(job, sys->m_jobsPool);
				return;
			}
			// Defer destruction until it's released by worker thread - this way we don't block
			job->m_isCancelled = UE_TRUE;
			break;

		case ueAsyncState_Succeeded:
		case ueAsyncState_Failed:
			if (job->m_userDestroyFunc)
				job->m_userDestroyFunc(job, job->m_userData);
			sys->m_doneJobs.Remove(job);
			ueDelete(job, sys->m_jobsPool);
			break;
	}
}

void ueJob_Wait(ueJob* job, u32 millisecs)
{
	ueEvent_Wait(job->m_event, millisecs);
}

// Global job system

static ueJobSys* s_globalJobSys = NULL;

void thGlobalJobSys_Startup(ueJobSysParams* params)
{
	UE_ASSERT(!s_globalJobSys);

	s_globalJobSys = ueJobSys_Create(params);
	UE_ASSERT(s_globalJobSys);
	ueJobSys_Start(s_globalJobSys);
}

void thGlobalJobSys_Shutdown()
{
	UE_ASSERT(s_globalJobSys);
	ueJobSys_Destroy(s_globalJobSys);
	s_globalJobSys = NULL;
}

ueAsync* thGlobalJobSys_StartJob(ueJobDesc* desc)
{
	UE_ASSERT(s_globalJobSys);
	return ueJobSys_StartJob(s_globalJobSys, desc);
}

ueAsync* thGlobalJobSys_StartJob(ueJob_WorkFunc workFunc, void* userData)
{
	UE_ASSERT(s_globalJobSys);
	return ueJobSys_StartJob(s_globalJobSys, workFunc, userData);
}
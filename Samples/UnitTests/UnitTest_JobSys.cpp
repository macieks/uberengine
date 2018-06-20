#include "Base/ueJobSys.h"

#define MAX_PROCESSORS 16
#define NUM_THREADS 16
#define MAX_JOBS 1000
#define JOB_SLEEP_TIME 10

void MyWorkFunc(ueAsync* async, void* userData)
{
//	ueLogD("Doing job %u", (u32) userData);
	ueThread_Sleep(JOB_SLEEP_TIME);
}

void MyDoneFunc(ueAsync* async, ueAsyncState state, void* userData)
{
}

void UnitTest_JobSys(ueAllocator* allocator)
{
	// Init thread workers

	ueJobSysThreadDesc workers[NUM_THREADS];
	const u32 numProcessors = ueThread_GetNumProcessors();
	for (u32 i = 0; i < NUM_THREADS; i++)
	{
		workers[i].m_processor = i % numProcessors;
		workers[i].m_priority = 0.6f;
	}

	ueJobSysParams params;
	params.m_stackAllocator = allocator;
	params.m_maxJobs = MAX_JOBS;
	params.m_numThreads = NUM_THREADS;
	params.m_threadDescs = workers;

	ueJobSys* jobSys = ueJobSys_Create(&params);
	ueJobSys_Start(jobSys);

	// Start jobs and wait for their completion

	ueLogI("Starting %d jobs (each sleeping for %u ms) on %u threads on %u physical processors", (u32) MAX_JOBS, (u32) JOB_SLEEP_TIME, NUM_THREADS, numProcessors);
	ueLogI("Min. time on 1 thread is: %.6f secs", (f32) (MAX_JOBS * JOB_SLEEP_TIME) * 0.001f);
	ueLogI("Min. time on %u threads is: %.6f secs", NUM_THREADS, (f32) (MAX_JOBS * JOB_SLEEP_TIME) * 0.001f / (f32) NUM_THREADS);

	ueTimer timer;
	for (u32 i = 0; i < MAX_JOBS; i++)
	{
		ueJobDesc desc;
		desc.m_enableAutoRelease = UE_TRUE;
		desc.m_workFunc = MyWorkFunc;
		desc.m_doneFunc = MyDoneFunc;
		desc.m_userData = (void*) (ueSize) i;

		ueJobSys_StartJob(jobSys, &desc);
	}
	ueJobSys_WaitAllJobs(jobSys);

	ueLogI("Finished: took %.6f secs", timer.GetSecs());

	// Shut down

	ueJobSys_Destroy(jobSys);

	ueLogI("Job system destroyed");
}

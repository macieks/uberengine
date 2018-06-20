#include "Base/ueThreading.h"

#include <process.h>

// Local thread
__declspec(thread) ueThreadId s_thread = 0;

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName( DWORD dwThreadID, char* threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*) &info);
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}

void ueThread_Exit(s32 exitCode)
{
	ExitThread(exitCode);
}

ueThreadId ueThread_Create(ueThreadParams* params)
{
	// Create suspended thread

	DWORD win32Id;
	ueThreadId id = (ueThreadId) CreateThread(NULL, params->m_stackSize, (LPTHREAD_START_ROUTINE) params->m_func, params->m_userData, CREATE_SUSPENDED, &win32Id);
	if (!id)
	{
		ueLogE("CreateThread failed, error = 0x%x", GetLastError());
		return 0;
	}

	// Set thread processor

	if (params->m_processor != -1)
#if defined(UE_WIN32)
		SetThreadAffinityMask((HANDLE) id, (DWORD) (1 << params->m_processor));
#elif defined(UE_X360)
		XSetThreadProcessor((HANDLE) id, params->m_processor);
#endif

	// Set thread priority

	ueThread_SetPriority(id, params->m_priority);

	// Set thread name

	if (params->m_name)
		SetThreadName(win32Id, const_cast<char*>(params->m_name));

	return id;
}

void ueThread_Join(ueThreadId id)
{
	WaitForSingleObject((HANDLE) id, INFINITE);
	CloseHandle((HANDLE) id);
}

ueBool ueThread_Start(ueThreadId id)
{
	const DWORD result = ResumeThread((HANDLE) id);
	if (result == U32_MAX)
	{
		ueLogE("ResumeThread failed, error = 0x%x", GetLastError());
		return UE_FALSE;
	}
	return UE_TRUE;
}

void ueThread_SetPriority(ueThreadId id, f32 priority)
{
	static const u32 win32Priorities[] =
	{
		THREAD_PRIORITY_IDLE,
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_TIME_CRITICAL,
	};
	static const f32 numWin32PrioritiesF = (f32) UE_ARRAY_SIZE(win32Priorities);

	const s32 priorityIndex = (s32) (priority * numWin32PrioritiesF / (numWin32PrioritiesF + 1.0f));

	if (!SetThreadPriority((HANDLE) id, win32Priorities[priorityIndex]))
		ueLogE("SetThreadPriority failed, error = 0x%x", GetLastError());
}

void ueThread_Sleep(u32 milliseconds)
{
	::Sleep((DWORD) milliseconds);
}

void ueThread_Yield()
{
	::Sleep(0);
}

ueThreadId ueThread_GetCurrent()
{
	return s_thread;
}

ueBool ueThread_IsMain()
{
	return !s_thread;
}

u32 ueThread_GetNumProcessors()
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
}

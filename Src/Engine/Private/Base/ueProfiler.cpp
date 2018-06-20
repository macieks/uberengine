#include "Base/ueBase.h"

#if defined(UE_ENABLE_PROFILER)

#include "Base/ueProfiler_Private.h"
#include "Base/ueThreading.h"

ueProfilerMgrData* g_profilerMgrData = NULL;

void ueProfiler_Startup(ueAllocator* allocator, u32 bufferSize, u32 maxNamedScopes, u32 captureBufferSize)
{
	UE_ASSERT(!g_profilerMgrData);

	u8* memory = (u8*) allocator->Alloc(sizeof(ueProfilerMgrData) + bufferSize * 2 + captureBufferSize);
	UE_ASSERT(memory);

	g_profilerMgrData = new(memory) ueProfilerMgrData();
	memory += sizeof(ueProfilerMgrData);

	g_profilerMgrData->m_allocator = allocator;
	g_profilerMgrData->m_scopesMutex = ueMutex_Create();

	g_profilerMgrData->m_buffer.m_data = memory;
	memory += bufferSize;
	g_profilerMgrData->m_buffer.m_capacity = bufferSize;
	g_profilerMgrData->m_buffer.m_size = 0;

	g_profilerMgrData->m_lastFrame.m_buffer = memory;
	memory += bufferSize;

	g_profilerMgrData->m_captureSize = captureBufferSize;
	g_profilerMgrData->m_capture = (ueProfilerCapture*) memory;
	g_profilerMgrData->m_captureUpToDate = UE_FALSE;

	g_profilerMgrData->m_numScopes = 0;
	UE_ASSERT_FUNC(g_profilerMgrData->m_scopes.Init(allocator, maxNamedScopes, maxNamedScopes));
	UE_ASSERT_FUNC(g_profilerMgrData->m_scopesById.Init(allocator, maxNamedScopes, maxNamedScopes));
}

void ueProfiler_Shutdown()
{
	UE_ASSERT(g_profilerMgrData);
	g_profilerMgrData->m_scopesById.Deinit();
	g_profilerMgrData->m_scopes.Deinit();
	ueMutex_Destroy(g_profilerMgrData->m_scopesMutex);

	ueAllocator* allocator = g_profilerMgrData->m_allocator;
	g_profilerMgrData->~ueProfilerMgrData();
	allocator->Free(g_profilerMgrData);
	g_profilerMgrData = NULL;
}

const ueProfilerData* ueProfiler_NextFrame()
{
	UE_ASSERT(g_profilerMgrData);
	UE_ASSERT(ueThread_IsMain());

	// Copy whole buffer to "last frame" buffer
	// Note: This is thread safe even though other threads may be writing to profiler buffer at the same time

	g_profilerMgrData->m_lastFrame.m_bufferSize = g_profilerMgrData->m_buffer.m_size;
	ueMemCpy(g_profilerMgrData->m_lastFrame.m_buffer, g_profilerMgrData->m_buffer.m_data, g_profilerMgrData->m_buffer.m_size);

	// Reset profiling

	g_profilerMgrData->m_buffer.m_size = 0;
	g_profilerMgrData->m_captureUpToDate = UE_FALSE;

	return &g_profilerMgrData->m_lastFrame;
}

const ueProfilerCapture* ueProfiler_BuildCapture()
{
	UE_ASSERT(g_profilerMgrData);
	UE_ASSERT(ueThread_IsMain());
	if (g_profilerMgrData->m_captureUpToDate)
		return g_profilerMgrData->m_capture;
	if (g_profilerMgrData->m_lastFrame.m_bufferSize == 0)
		return NULL;
	ueProfilerCapture* newCapture = ueProfiler_BuildCapture(g_profilerMgrData->m_capture, g_profilerMgrData->m_captureSize, &g_profilerMgrData->m_lastFrame);
	return newCapture;
}

u32 ueProfiler_GetScopeId(const char* name, u32 flags)
{
	UE_ASSERT(g_profilerMgrData);
	ueMutexLock lock(g_profilerMgrData->m_scopesMutex);

	ueProfilerMgrData::Scope* scopePtr = g_profilerMgrData->m_scopes.Find(name);
	if (scopePtr)
	{
		const u16 id = scopePtr->m_id;
		return id;
	}

	ueProfilerMgrData::Scope scope;
	scope.m_flags = flags;
	scope.m_id = g_profilerMgrData->m_numScopes++;

	g_profilerMgrData->m_scopes.Insert(name, scope);
	g_profilerMgrData->m_scopesById.Insert(scope.m_id, name);
	
	return scope.m_id;
}

const char* ueProfiler_GetScopeName(u32 id)
{
	UE_ASSERT(g_profilerMgrData);
	ueMutexLock lock(g_profilerMgrData->m_scopesMutex);
	const char** namePtr = g_profilerMgrData->m_scopesById.Find(id);
	return namePtr ? *namePtr : NULL;
}

void ueProfiler_Push(u32 id)
{
	UE_ASSERT(g_profilerMgrData);
	ueProfilerOperation_Push* op = (ueProfilerOperation_Push*) g_profilerMgrData->m_buffer.Alloc(sizeof(ueProfilerOperation_Push));
	if (!op) return;
	op->m_ticks = ueClock_GetCurrent();
	op->m_type = ueProfilerOperation::Type_Push;
	op->m_threadId = ueThread_GetCurrent();
	op->m_id = id;
}

void ueProfiler_PushDynamic(const char* name)
{
	UE_ASSERT(g_profilerMgrData);
	const u32 nameSize = ueStrLen(name) + 1;
	ueProfilerOperation_PushDynamic* op = (ueProfilerOperation_PushDynamic*) g_profilerMgrData->m_buffer.Alloc(sizeof(ueProfilerOperation_PushDynamic) + nameSize);
	if (!op) return;
	op->m_ticks = ueClock_GetCurrent();
	op->m_type = ueProfilerOperation::Type_PushDynamic;
	op->m_threadId = ueThread_GetCurrent();
	op->m_nameSize = nameSize;
	ueMemCpy(op + 1, name, nameSize);
}

void ueProfiler_Pop()
{
	UE_ASSERT(g_profilerMgrData);
	ueProfilerOperation_Pop* op = (ueProfilerOperation_Pop*) g_profilerMgrData->m_buffer.Alloc(sizeof(ueProfilerOperation_Pop));
	if (!op) return;
	op->m_ticks = ueClock_GetCurrent();
	op->m_type = ueProfilerOperation::Type_Pop;
	op->m_threadId = ueThread_GetCurrent();
}

#endif // defined(UE_ENABLE_PROFILER)
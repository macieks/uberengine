#include "Base/ueThreading.h"

ueAllocatorStats::ueAllocatorStats() :
	m_flags(0)
{
	Reset();
}

void ueAllocatorStats::Reset()
{
	m_used = 0;
	m_usedPeak = 0;
	m_largestBlock = 0;
	m_total = 0;
	m_numAllocs = 0;
	m_numAllocsPeak = 0;
}

void ueAllocatorStats::UpdateAlloc(ueSize oldSize, ueSize newSize)
{
	if (oldSize < newSize)
	{
		m_used -= oldSize;
		m_used += newSize;
		m_usedPeak = ueMax(m_usedPeak, m_used);
	}
	else
	{
		UE_ASSERT(m_used >= oldSize);
		m_used -= oldSize;
		m_used += newSize;
	}
}

void ueAllocatorStats::AddAlloc(ueSize bytes)
{
	m_numAllocs++;
	m_numAllocsPeak = (u32) ueMax(m_numAllocsPeak, m_numAllocs);
	UpdateAlloc(0, bytes);
}

void ueAllocatorStats::RemoveAlloc(ueSize bytes)
{
	UE_ASSERT(m_numAllocs > 0);
	m_numAllocs--;
	UpdateAlloc(bytes, 0);
}

ueAllocator::ueAllocator(ueBool isThreadSafe) :
	m_reallocFunc(NULL),
	m_isThreadSafe(isThreadSafe),
	m_mutex(NULL)
{
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	ueStrCpyS(m_name, "<unnamed>");
	m_numListeners = 0;
#endif

	if (m_isThreadSafe)
		m_mutex = ueMutex_Create();
}

ueAllocator::~ueAllocator()
{
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	UE_ASSERT(m_numListeners == 0);
#endif

	if (m_mutex)
		ueMutex_Destroy(m_mutex);
}

#if defined(UE_ENABLE_MEMORY_DEBUGGING)

void ueAllocator::SetName(const char* name)
{
	ueStrCpyS(m_name, name);
}

#endif

void ueAllocator::GetStats(ueAllocatorStats* stats)
{
	if (m_isThreadSafe) ueMutex_Lock(m_mutex);
	ueMemCpy(stats, &m_stats, sizeof(ueAllocatorStats));
	if (m_isThreadSafe) ueMutex_Unlock(m_mutex);
}

ueBool ueAllocator::HasAnyAllocation()
{
	return m_stats.m_numAllocs > 0;
}

void* ueAllocator::Realloc(void* memory, ueSize size, ueSize alignment, void* allocInfo)
{
	if (m_isThreadSafe) ueMutex_Lock(m_mutex);

	void* result = m_reallocFunc(this, memory, size, alignment, allocInfo);
#if defined(UE_ENABLE_MEMORY_DEBUGGING)
	_DebugOnRealloc(memory, size, alignment, result);
#endif

	if (m_isThreadSafe) ueMutex_Unlock(m_mutex);
	return result;
}

#if defined(UE_ENABLE_MEMORY_DEBUGGING)

void ueAllocator::AddListener(ueAllocatorCallback callback, void* userData)
{
	UE_ASSERT(m_numListeners < UE_ARRAY_SIZE(m_listeners));
	m_listeners[m_numListeners].m_callback = callback;
	m_listeners[m_numListeners].m_userData = userData;
	m_numListeners++;
}

void ueAllocator::RemoveListener(ueAllocatorCallback callback, void* userData)
{
	for (u32 i = 0; m_numListeners; i++)
		if (m_listeners[i].m_callback == callback && m_listeners[i].m_userData == userData)
		{
			m_listeners[i] = m_listeners[--m_numListeners];
			return;
		}
	UE_ASSERT_MSG(0, "Listener not found");
}

void ueAllocator::_DebugOnRealloc(void* memory, ueSize size, ueSize alignment, void* result)
{
	// Notify listeners
	for (u32 i = 0; i < m_numListeners; i++)
		m_listeners[i].m_callback(this, memory, size, alignment, result, m_listeners[i].m_userData);
}

void ueAllocator::SetThreadSafe(ueBool isThreadSafe)
{
	if (m_isThreadSafe == isThreadSafe)
		return;
	m_isThreadSafe = isThreadSafe;
	if (m_isThreadSafe)
		m_mutex = ueMutex_Create();
	else
	{
		ueMutex_Destroy(m_mutex);
		m_mutex = NULL;
	}
}

#endif

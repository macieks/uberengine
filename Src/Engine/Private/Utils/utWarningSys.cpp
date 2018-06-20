#include "Base/ueBase.h"
#include "Base/ueThreading.h"
#include "IO/ioFile.h"

#if defined(UE_ENABLE_WARNINGS)

struct utWarningSysData
{
	ueAllocator* m_allocator;

	void* m_buffer;
	ueSize m_size;
	ueSize m_capacity;

	ueSize* m_warningStartOffsets;
	u32 m_numWarnings;
	u32 m_maxWarnings;

	ueMutex* m_mutex;

	utWarningSysData() : m_allocator(NULL) {}
};

static utWarningSysData* s_data = NULL;

void utWarningSys_Startup(ueAllocator* allocator, ueSize bufferSize, u32 maxWarnings)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) utWarningSysData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;
	s_data->m_mutex = ueMutex_Create();

	s_data->m_buffer = allocator->Alloc(bufferSize);
	UE_ASSERT(s_data->m_buffer);
	s_data->m_capacity = bufferSize;
	s_data->m_size = 0;

	s_data->m_warningStartOffsets = (ueSize*) allocator->Alloc(bufferSize);
	UE_ASSERT(s_data->m_warningStartOffsets);
	s_data->m_maxWarnings = maxWarnings;
	s_data->m_numWarnings = 0;
}

void utWarningSys_Shutdown()
{
	UE_ASSERT(s_data);
	s_data->m_allocator->Free(s_data->m_warningStartOffsets);
	s_data->m_allocator->Free(s_data->m_buffer);
	ueMutex_Destroy(s_data->m_mutex);
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void utWarningSys_Reset()
{
	UE_ASSERT(s_data);
	ueMutexLock(s_data->m_mutex);
	s_data->m_numWarnings = 0;
	s_data->m_size = 0;
}

void utWarningSys_AddWarningV(const char* file, u32 line, const char* function, const char* condition, const char* msg, va_list args)
{
	UE_ASSERT(s_data);

	ueMutexLock(s_data->m_mutex);

	// Not enough space?

	if (s_data->m_numWarnings == s_data->m_maxWarnings)z
		return;

	// Format warning

	char buffer[1024];
	u32 n = ueStrFormatS(buffer, "%s:%d %s: ", ioPath_ExtractFileName(file), line, condition);
	n += ueStrFormat((char*) buffer + n, UE_ARRAY_SIZE(buffer) - n, msg);

	// Not enough space in buffer?

	if (s_data->m_size + n + 1 >= s_data->m_capacity)
		return;

	// Add entry

	ueMemCpy((u8*) s_data->m_buffer + s_data->m_size, buffer, n + 1);
	s_data->m_warningStartOffsets[s_data->m_numWarnings++] = s_data->m_size;
	s_data->m_size += n + 1;
}

u32 utWarningSys_GetNumWarnings()
{
	UE_ASSERT(s_data);
	ueMutexLock(s_data->m_mutex);
	return s_data->m_numWarnings;
}

const char* utWarningSys_GetWarning(u32 index)
{
	UE_ASSERT(s_data);
	ueMutexLock(s_data->m_mutex);
	UE_ASSERT(index < s_data->m_numWarnings);
	return (char*) s_data->m_buffer + s_data->m_warningStartOffsets[index];
}

#endif // defined(UE_ENABLE_WARNINGS)
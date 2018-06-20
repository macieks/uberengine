#include "IO/ioFileSys_Private.h"

// File
// ---------------------

struct ioFile_Memory : ioFile
{
	ueSize m_offset;
	ueSize m_size;
	void* m_buffer;
};

// File system
// ---------------------

struct ioFileSys_Memory : ioFileSys
{
};

static ioFileSys_Memory s_fs;

ioFile* ioFile_OpenInMemory(void* buffer, ueSize size)
{
	UE_ASSERT(s_fs.m_allocator);
	ioFile_Memory* f = new(s_fs.m_allocator) ioFile_Memory();
	if (!f)
		return NULL;
	f->m_buffer = buffer;
	f->m_size = size;
	f->m_offset = 0;
	f->m_fs = &s_fs;
	return f;
}

void ioFile_Memory_Close(ioFile_Memory* f)
{
	ueDelete(f, s_fs.m_allocator);
}

ueSize ioFile_Memory_Read(ioFile_Memory* f, void* data, ueSize size, ueSize count, ueAsync** async)
{
	UE_ASSERT(!async);
	if (size <= 0 || count <= 0)
		return 0;
	const ueSize actualCount = ueMin((f->m_size - f->m_offset) / size, count);
	if (actualCount)
		ueMemCpy(data, (u8*) f->m_buffer + f->m_offset, size * actualCount);
	f->m_offset += size * actualCount;
	return actualCount;
}

ueSize ioFile_Memory_Write(ioFile_Memory* f, const void* data, ueSize size, ueSize count)
{
	if (size <= 0 || count <= 0)
		return 0;
	const ueSize actualCount = ueMin((f->m_size - f->m_offset) / size, count);
	if (actualCount)
		ueMemCpy((u8*) f->m_buffer + f->m_offset, data, size * actualCount);
	f->m_offset += size * actualCount;
	return actualCount;
}

ueBool ioFile_Memory_Seek(ioFile_Memory* f, ioFileSeekType seekType, ueSize offset)
{
	switch (seekType)
	{
		case ioFileSeekType_Set:	f->m_offset = offset; break;
		case ioFileSeekType_Offset:	f->m_offset += offset; break;
		case ioFileSeekType_End:	f->m_offset = f->m_size; break;
	}

	if (f->m_offset < 0)
	{
		f->m_offset = 0;
		return UE_FALSE;
	}
	else if (f->m_offset > f->m_size)
	{
		f->m_offset = f->m_size;
		return UE_FALSE;
	}

    return UE_TRUE;
}

ueSize ioFile_Memory_GetOffset(ioFile_Memory* f)
{
    return f->m_offset;
}

void ioFileSys_InitMemory(ioFileSysParams* params)
{
	s_fs.m_allocator = params->m_allocator;

	// Initialize file system functions

	s_fs.m_close = (ioFile_CloseFunc) ioFile_Memory_Close;
	s_fs.m_read = (ioFile_ReadFunc) ioFile_Memory_Read;
	s_fs.m_write = (ioFile_WriteFunc) ioFile_Memory_Write;
	s_fs.m_seek = (ioFile_SeekFunc) ioFile_Memory_Seek;
	s_fs.m_getOffset = (ioFile_GetOffsetFunc) ioFile_Memory_GetOffset;
}
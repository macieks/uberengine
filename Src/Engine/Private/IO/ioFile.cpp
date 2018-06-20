#include "Base/ueAsync_Private.h"
#include "IO/ioFileSys_Private.h"
#include "Base/ueJobSys.h"
#include "Base/Containers/ueVector.h"

//! Mounted file system
struct ioFileSysEntry
{
	ioFileSys* m_fs;
	u32 m_priority;		//!< File system priority used when accessing files or directories

	ioFileSysEntry() : m_fs(NULL) {}
};

struct ioFileSysMgrData
{
	ueAllocator* m_allocator;

	ioFileSys* m_default;
	ioFileSys* m_sourceFS;
	ioFileSys* m_dataFS;

	ueVector<ioFileSysEntry> m_fileSystems;	//!< Mounted file systems; TODO: Refactor storage into path-tree for faster lookup (imagine hundreds of Zip file systems being mounted)
};

static ioFileSysMgrData* s_data;

// File system management functions

void ioFileSys_Startup(ueAllocator* allocator, u32 maxFileSystems)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) ioFileSysMgrData;
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_default = NULL;
	s_data->m_dataFS = NULL;
	s_data->m_sourceFS = NULL;

	UE_ASSERT_FUNC(s_data->m_fileSystems.Init(allocator, maxFileSystems));
}

void ioFileSys_Shutdown()
{
	UE_ASSERT(s_data);

	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
		ioFileSys_Destroy(s_data->m_fileSystems[i]);

	s_data->m_fileSystems.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void ioFileSys_Mount(ioFileSys* fs, const char* location, u32 priority)
{
	u32 indexToInsert = 0;
	for (; indexToInsert < s_data->m_fileSystems.Size(); indexToInsert++)
		if (priority >= s_data->m_fileSystems[indexToInsert].m_priority)
			break;

	ioFileSysEntry* entry = s_data->m_fileSystems.InsertAt(indexToInsert);
	entry->m_fs = fs;
	entry->m_priority = priority;
	entry->m_fs->m_mountPathLength = ueStrLen(location);
	ueMemCpy(entry->m_fs->m_mountPath, location, entry->m_fs->m_mountPathLength + 1);
}

void ioFileSys_Unmount(ioFileSys* fs)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
		if (s_data->m_fileSystems[i].m_fs == fs)
		{
			if (fs == s_data->m_default)
				s_data->m_default = NULL;
			s_data->m_fileSystems.RemoveAt(i); // Preserve order
			return;
		}
}

void ioFileSys_SetDefault(ioFileSys* fs)
{
	s_data->m_default = fs;
}

ioFileSys* ioFileSys_GetDefault()
{
	return s_data->m_default;
}

void ioFileSys_SetSourceFileSys(ioFileSys* fs)
{
	s_data->m_sourceFS = fs;
}

ioFileSys* ioFileSys_GetSourceFileSys()
{
	return s_data->m_sourceFS;
}

void ioFileSys_SetDataFileSys(ioFileSys* fs)
{
	s_data->m_dataFS = fs;
}

ioFileSys* ioFileSys_GetDataFileSys()
{
	return s_data->m_dataFS;
}

void ioFileSys_SetJobSystem(ioFileSys* fs, ueJobSys* jobSys)
{
	fs->m_jobSys = jobSys;
}

// File system-full functions

void ioFileSys_Destroy(ioFileSys* fs)
{
	ioFileSys_Unmount(fs);
	fs->m_destroyFileSystem(fs);
}

void ioFileSys_Enumerate(ioFileSys* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData)
{
	UE_ASSERT(fs->m_enumerate);
	return fs->m_enumerate(fs, path, pattern, cb, enumFlags, userData);
}

ioFile* ioFile_Open(ioFileSys* fs, const char* path, u32 openFlags)
{
	return fs->m_open(fs, path, openFlags);
}

void ioFile_Close(ioFile* f)
{
	f->m_fs->m_close(f);
}

ueSize ioFile_Read(ioFile* f, void* data, ueSize size, ueSize count, ueAsync** async)
{
	return f->m_fs->m_read(f, data, size, count, async);
}

ueSize ioFile_Write(ioFile* f, const void* data, ueSize size, ueSize count)
{
	return f->m_fs->m_write(f, data, size, count);
}

ueBool ioFile_Seek(ioFile* f, ioFileSeekType seekType, ueSize offset)
{
	return f->m_fs->m_seek(f, seekType, offset);
}

ueSize ioFile_GetOffset(ioFile* f)
{
	return f->m_fs->m_getOffset(f);
}

ueBool ioFile_GetAttrs(ioFileSys* fs, const char* path, ioFileAttrs& attrs)
{
	UE_ASSERT(fs->m_getAttrs);
	return fs->m_getAttrs(fs, path, attrs);
}

ueBool ioFile_Exists(ioFileSys* fs, const char* path)
{
	UE_ASSERT(fs->m_existsFile);
	return fs->m_existsFile(fs, path);
}

ueBool ioFile_Delete(ioFileSys* fs, const char* path)
{
	if (!fs->m_deleteFile)
		return UE_FALSE;
	return fs->m_deleteFile(fs, path);
}

ueBool ioFile_Copy(ioFileSys* fs, const char* srcPath, const char* dstPath, ueBool failIfExists)
{
	UE_ASSERT(fs->m_copyFile);
	return fs->m_copyFile(fs, srcPath, dstPath, failIfExists);
}

ueBool ioFile_Save(const char* path, const void* data, ueSize dataSize)
{
	ioFile* f = ioFile_Open(path, ioFileOpenFlags_Write | ioFileOpenFlags_Create);
	if (!f)
		return UE_FALSE;
	const ueBool writeSuccessful = ioFile_Write(f, data, dataSize, 1) == 1;
	ioFile_Close(f);
	return writeSuccessful;
}

// ioDir

ueBool ioDir_Exists(ioFileSys* fs, const char* path)
{
	UE_ASSERT(fs->m_existsDir);
	return fs->m_existsDir(fs, path);
}

ueBool ioDir_Create(ioFileSys* fs, const char* path, ueBool failIfExists)
{
	UE_ASSERT(fs->m_createDir);
	return fs->m_createDir(fs, path, failIfExists);
}

ueBool ioDir_Delete(ioFileSys* fs, const char* path, ueBool failIfNotEmpty)
{
	UE_ASSERT(fs->m_deleteDir);
	return fs->m_deleteDir(fs, path, failIfNotEmpty);
}

ueBool ioDir_Copy(ioFileSys* fs, const char* srcPath, const char* dstPath)
{
	UE_ASSERT(fs->m_copyDir);
	return fs->m_copyDir(fs, srcPath, dstPath);
}

// File system helpers

ueBool ioFile_GetSize(ioFileSys* fs, const char* path, ueSize& size)
{
	ioFileAttrs attrs;
	attrs.m_flags = ioFileAttrFlags_Size;
	if (!ioFile_GetAttrs(fs, path, attrs))
		return UE_FALSE;
	size = attrs.m_size;
	return UE_TRUE;
}

ueBool ioDir_CreateChain(ioFileSys* fs, const char* path, ueBool pathContainsFileName)
{
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

ueBool ioFile_Load(ioFileSys* fs, const char* path, void*& buffer, ueSize& size, ueSize offset, ueAllocator* allocator, ueBool capWithEOF)
{
	const ueBool bufferSupplied = buffer != NULL;

	// Open file

	ioFilePtr f;
	if (!f.Open(fs, path, ioFileOpenFlags_Read))
		return UE_FALSE;

	// Get file size

	if (size == 0)
		size = f.GetSize();

	// Alloc buffer if not supplied

	if (!buffer && !(buffer = allocator->Alloc(size + (capWithEOF ? 1 : 0))))
		goto Failure;

	// Read requested file portion

	if (!f.Seek(ioFileSeekType_Set, offset))
		goto Failure;

	if (f.Read(buffer, size, 1) != 1)
		goto Failure;

	// Optionally cap buffer with EOF character

	if (capWithEOF)
		((char*) buffer)[size] = 0;

	return UE_TRUE;

Failure:
	if (!bufferSupplied && buffer)
	{
		allocator->Free(buffer);
		buffer = NULL;
	}
	return UE_FALSE;
}

// File system-less functions

ueBool ioFile_GetSize(const char* path, ueSize& size)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioFile_GetSize(entry.m_fs, path, size)) return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioFile_Load(const char* path, void*& buffer, ueSize& size, ueSize offset, ueAllocator* allocator, ueBool capWithEOF)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioFile_Load(entry.m_fs, path, buffer, size, offset, allocator, capWithEOF)) return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioFile_GetAttrs(const char* path, ioFileAttrs& attrs)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioFile_GetAttrs(entry.m_fs, path, attrs)) return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioFile_Exists(const char* path)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioFile_Exists(entry.m_fs, path)) return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioFile_Delete(const char* path)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioFile_Delete(entry.m_fs, path)) return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioDir_Exists(const char* path)
{
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (ioDir_Exists(entry.m_fs, path)) return UE_TRUE;
	}
	return UE_FALSE;
}

ioFile* ioFile_Open(const char* path, u32 openFlags)
{
	ioFile* file = NULL;
	for (u32 i = 0; i < s_data->m_fileSystems.Size(); i++)
	{
		ioFileSysEntry& entry = s_data->m_fileSystems[i];
		if (ueStrNCmp(entry.m_fs->m_mountPath, path, entry.m_fs->m_mountPathLength)) continue;
		if (file = ioFile_Open(entry.m_fs, path, openFlags)) break;
	}
	return file;
}

// Async file operations

struct ioFile_GenericAsyncDesc
{
	ueAllocator* m_allocator;

	ioFile* m_file;
	void* m_data;
	ueSize m_size;
	ueSize m_count;

	ueSize m_numProcessedElements;
};

void ioFile_GenericAsyncRead_WorkFunc(ueAsync* async, void* userData)
{
	ioFile_GenericAsyncDesc* desc = (ioFile_GenericAsyncDesc*) userData;
	desc->m_numProcessedElements = ioFile_Read(desc->m_file, desc->m_data, desc->m_size, desc->m_count);
	ueJob_SetAsyncData(async, &desc->m_numProcessedElements);
	ueAsync_SetDone(async, UE_TRUE);
}

void ioFile_GenericAsyncRead_DestroyFunc(ueAsync* async, void* userData)
{
	ioFile_GenericAsyncDesc* desc = (ioFile_GenericAsyncDesc*) userData;
	desc->m_allocator->Free(desc);
}

ueAsync* ioFile_CreateGenericAsyncOperation(ueAllocator* allocator, ueJobSys* jobSys, ioFile* file, void* data, ueSize size, ueSize count)
{
	ioFileSys* fs = file->m_fs;
	UE_ASSERT_MSG(jobSys, "This file system requires job system to be assigned to it for asynchronous operations - see ioFileSys_SetJobSystem()");

	// Create async task description

	ioFile_GenericAsyncDesc* asyncDesc = new(allocator) ioFile_GenericAsyncDesc;
	if (!asyncDesc)
		return NULL;
	asyncDesc->m_allocator = allocator;
	asyncDesc->m_file = file;
	asyncDesc->m_data = data;
	asyncDesc->m_size = size;
	asyncDesc->m_count = count;
	asyncDesc->m_numProcessedElements = 0;

	// Create job description

	ueJobDesc jobDesc;
	jobDesc.m_destroyFunc = ioFile_GenericAsyncRead_DestroyFunc;
	jobDesc.m_workFunc = ioFile_GenericAsyncRead_WorkFunc;
	jobDesc.m_userData = asyncDesc;

	// Start the job

	ueAsync* async = ueJobSys_StartJob(jobSys, &jobDesc);
	if (!async)
	{
		allocator->Free(asyncDesc);
		return NULL;
	}
	return async;
}

// ioFilePtr

ioFilePtr::ioFilePtr() :
	m_file(NULL)
{}

ioFilePtr::~ioFilePtr()
{
	Close();
}

ueBool ioFilePtr::Open(const char* path, u32 flags)
{
	Close();
	m_file = ioFile_Open(path, flags);
	return m_file != NULL;
}

ueBool ioFilePtr::Open(ioFileSys* fs, const char* path, u32 flags)
{
	Close();
	m_file = ioFile_Open(fs, path, flags);
	return m_file != NULL;
}

void ioFilePtr::Close()
{
	if (m_file)
	{
		ioFile_Close(m_file);
		m_file = NULL;
	}
}

ueSize ioFilePtr::Read(void* data, ueSize size, ueSize count, ueAsync** async)
{
	UE_ASSERT(m_file);
	return ioFile_Read(m_file, data, size, count, async);
}

ueSize ioFilePtr::Write(const void* data, ueSize size, ueSize count)
{
	UE_ASSERT(m_file);
	return ioFile_Write(m_file, data, size, count);
}

ueBool ioFilePtr::Seek(ioFileSeekType seekType, ueSize offset)
{
	UE_ASSERT(m_file);
	return ioFile_Seek(m_file, seekType, offset);
}

ueSize ioFilePtr::GetOffset()
{
	UE_ASSERT(m_file);
	return ioFile_GetOffset(m_file);
}

ueSize ioFilePtr::GetSize()
{
	UE_ASSERT(m_file);
	const ueSize curr = ioFile_GetOffset(m_file);
	if (curr == IO_INVALID_OFFSET) return IO_INVALID_OFFSET;
	if (!ioFile_Seek(m_file, ioFileSeekType_End, 0)) return IO_INVALID_OFFSET;
	const ueSize size = ioFile_GetOffset(m_file);
	if (size == IO_INVALID_OFFSET) return IO_INVALID_OFFSET;
	if (!ioFile_Seek(m_file, ioFileSeekType_Set, curr)) return IO_INVALID_OFFSET;
	return size;
}

// uePath

ueBool ioPath_ExtractFileNameWithoutExtension(const char* path, char* buffer, u32 bufferSize)
{
	const char* fullFileName = ioPath_ExtractFileName(path);
	if (!fullFileName)
		return UE_FALSE;

	const char* extension = ioPath_ExtractFileExtension(fullFileName);
	if (!extension)
	{
		ueStrCpy(buffer, bufferSize, path);
		return UE_TRUE;
	}

	ueStrNCpy(buffer, bufferSize, fullFileName, (u32) ((ueSize) extension - (ueSize) fullFileName - 1));
	return UE_TRUE;
}
/*
ueBool ioFileSystem::RemoveExtension(ioFilePath& fileName)
{
	const ueChar* extension = ExtractFileExtension(*fileName);
	if (!extension)
		return true;

	fileName.TrimEnd( fileName.Length() - (ueInt) ((ueSize) extension - (ueSize) &fileName[0]) + 1 );
	return true;
}
*/
const char* ioPath_ExtractFileName(const char* path)
{
	const char* ptr = path;
	while (*path)
	{
		if (ioPath_IsSplitter(*path)) ptr = path + 1;
		path++;
	}
	return ptr;
}

const char* ioPath_ExtractFileExtension(const char* path)
{
	const char* ext = NULL;
	while (*path)
	{
		if (*path == '.') ext = path + 1;
		path++;
	}
	return ext;
}

ueBool ioPath_ExtractDirectoryName(const char* path, char* buffer, u32 bufferSize)
{
	const char* lastSplitterPtr = NULL;
	const char* ptr = path;
	while (*ptr)
	{
		if (ioPath_IsSplitter(*ptr)) lastSplitterPtr = ptr;
		ptr++;
	}
	if (!lastSplitterPtr)
		return UE_FALSE;
	ueStrNCpy(buffer, bufferSize, path, (u32) ((ueSize) lastSplitterPtr - (ueSize) path));
	return true;
}
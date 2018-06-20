#include "IO/ioFileSys_Private.h"
#include "Base/ueAsync_Private.h"
#include "Base/Containers/ueGenericPool.h"
#include "Base/ueJobSys.h"

#if defined(UE_WIN32)

	#define MAKE_FULL_PATH(fullPath, path) \
		uePath fullPath; \
		ueStrFormatS(fullPath, "%s%s", fs->m_root, path);

#elif defined(UE_X360)

	#define MAKE_FULL_PATH(fullPath, path) \
		uePath fullPath; \
		ueStrFormatS(fullPath, "%s", path); \
		for (u32 i = 0; fullPath[i]; i++) \
			fullPath[i] = (fullPath[i] == '/') ? '\\' : fullPath[i];

#endif

struct ioFile_Native : ioFile
{
	HANDLE m_handle;
	ueBool m_isAsync;
};

struct ioFile_Native_Async : ueAsync
{
	ioFile_Native* m_file;

	OVERLAPPED m_overlapped;

	ueSize m_elementSize;
	ueSize m_numProcessedElements;
};

struct ioFileSys_Native : ioFileSys
{
	ueGenericPool m_filesPool;
	uePath m_root;
	u32 m_rootLength;

	ueGenericPool m_asyncsPool;
};

void ioFileSys_Native_Destroy(ioFileSys_Native* fs)
{
	if (fs->m_jobSys)
		ueJobSys_Destroy(fs->m_jobSys);
	fs->m_asyncsPool.Deinit();
	fs->m_filesPool.Deinit();
	ueDelete(fs, fs->m_allocator);
}

void ioFileSys_Native_EnumerateRec(ioFileSys_Native* fs, char* path, u32 pathLength, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, u32 level, void* userData)
{
	HANDLE handle;
	DWORD result;
	WIN32_FIND_DATAA tagData;

	ueStrCpy(path + pathLength, UE_MAX_PATH - pathLength, pattern);
	handle = FindFirstFile(path, &tagData);
	result = U32_MAX;
	while (handle != INVALID_HANDLE_VALUE && result)
	{
		const u32 nameLength = ueStrLen(tagData.cFileName);

		// Directory
		if (tagData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ueStrCmp(tagData.cFileName, ".") && ueStrCmp(tagData.cFileName, "..") &&
				!(tagData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
			{
				ueMemCpy(path + pathLength, tagData.cFileName, nameLength + 1);
				path[pathLength + nameLength] = '/';
				path[pathLength + nameLength + 1] = 0;

				if (enumFlags & ioEnumFlags_Dirs)
				{
					ioEnumInfo info;
					info.m_isDirectory = UE_TRUE;
					info.m_name = tagData.cFileName;
					info.m_path = path + fs->m_rootLength;
					info.m_level = level;

					cb(&info, userData);
				}

				if (enumFlags & ioEnumFlags_Recursive)
					ioFileSys_Native_EnumerateRec(fs, path, pathLength + nameLength + 1, pattern, cb, enumFlags, level + 1, userData);
			}
		}

		// File
		else if (enumFlags & ioEnumFlags_Files)
		{
			ueMemCpy(path + pathLength, tagData.cFileName, nameLength + 1);

			ioEnumInfo info;
			info.m_isDirectory = UE_FALSE;
			info.m_name = tagData.cFileName;
			info.m_path = path + fs->m_rootLength;
			info.m_level = level;

			cb(&info, userData);
		}

		result = FindNextFile(handle, &tagData);
	}

	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);
}

void ioFileSys_Native_Enumerate(ioFileSys_Native* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData)
{
	MAKE_FULL_PATH(fullPath, path);
	ioFileSys_Native_EnumerateRec(fs, fullPath, ueStrLen(fullPath), pattern, cb, enumFlags, 0, userData);
}

ioFile* ioFile_Native_Open(ioFileSys_Native* fs, const char* path, u32 openFlags)
{
	MAKE_FULL_PATH(fullPath, path);

	DWORD desiredAccess = 0;
	DWORD creationDisposition = 0;
	DWORD flagsAndAttributes = 0;

	if (openFlags & ioFileOpenFlags_Read)
		desiredAccess |= GENERIC_READ;
	if (openFlags & ioFileOpenFlags_Write)
		desiredAccess |= GENERIC_WRITE;
	if (openFlags & ioFileOpenFlags_Async)
		desiredAccess |= FILE_FLAG_OVERLAPPED;

	flagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

	if (openFlags & ioFileOpenFlags_Read)
		creationDisposition |= OPEN_EXISTING;
	if (openFlags & ioFileOpenFlags_Write)
		creationDisposition |= (openFlags & ioFileOpenFlags_Create) ? CREATE_ALWAYS : OPEN_EXISTING;

	HANDLE handle = CreateFile(fullPath, desiredAccess, 0, NULL, creationDisposition, flagsAndAttributes, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return NULL;

	ioFile_Native* f = new(fs->m_filesPool) ioFile_Native();
	UE_ASSERT(f);
	f->m_handle = handle;
	f->m_fs = fs;
	f->m_isAsync = (openFlags & ioFileOpenFlags_Async) != 0;
	return f;
}

void ioFile_Native_Close(ioFile_Native* f)
{
	UE_ASSERT(f->m_handle != INVALID_HANDLE_VALUE);
	CloseHandle(f->m_handle);
	((ioFileSys_Native*) f->m_fs)->m_filesPool.Free(f);
}

void* ioFile_Native_Async_GetDataFunc(ioFile_Native_Async* async)
{
	return &async->m_numProcessedElements;
}

void ioFile_Native_Async_WaitFunc(ioFile_Native_Async* async, u32 millisecs)
{
	DWORD numBytesTransfered;
	if (GetOverlappedResult(async->m_file->m_handle, &async->m_overlapped, &numBytesTransfered, TRUE))
	{
		async->m_numProcessedElements = numBytesTransfered / async->m_elementSize;
		async->m_state = ueAsyncState_Succeeded;
	}
	else if (GetLastError() != ERROR_IO_PENDING)
		async->m_state = ueAsyncState_Failed;
}

void ioFile_Native_Async_PollFunc(ioFile_Native_Async* async)
{
	DWORD numBytesTransfered;
	if (GetOverlappedResult(async->m_file->m_handle, &async->m_overlapped, &numBytesTransfered, FALSE))
	{
		async->m_numProcessedElements = numBytesTransfered / async->m_elementSize;
		async->m_state = ueAsyncState_Succeeded;
	}
	else if (GetLastError() != ERROR_IO_PENDING)
		async->m_state = ueAsyncState_Failed;
}

void ioFile_Native_Async_DestroyFunc(ioFile_Native_Async* async)
{
	if (async->m_state == ueAsyncState_InProgress)
#if defined(UE_TARGET_WIN_XP)
		CancelIo(async->m_file->m_handle);
#else
		CancelIoEx(async->m_file->m_handle, &async->m_overlapped);
#endif

	ioFileSys_Native* fs = (ioFileSys_Native*) async->m_file->m_fs;
	fs->m_asyncsPool.Free(async);
}

ueSize ioFile_Native_Read(ioFile_Native* f, void* data, ueSize size, ueSize count, ueAsync** async)
{
	UE_ASSERT(size > 0);

	// Asynchronous read

	if (async)
	{
		// Use generic async code path if job system is set

		if (f->m_fs->m_jobSys)
		{
			*async = ioFile_CreateGenericAsyncOperation(f->m_fs->m_allocator, f->m_fs->m_jobSys, f, data, size, count);
			return 0;
		}

		// Use Win32 specific async IO routines

		UE_ASSERT_MSG(f->m_isAsync, "File wasn't open for async access.");

		ioFileSys_Native* fs = (ioFileSys_Native*) f->m_fs;
		ioFile_Native_Async* a = new(fs->m_asyncsPool) ioFile_Native_Async();
		if (!a)
			return 0; // Failure

		ueMemZeroS(a->m_overlapped);
		a->m_numProcessedElements = 0;

		DWORD numBytesTransfered;
		if (ReadFile(f->m_handle, data, (DWORD) (size * count), &numBytesTransfered, &a->m_overlapped))
		{
			a->m_numProcessedElements = numBytesTransfered / size;
			a->m_state = ueAsyncState_Succeeded;
		}
		else if (GetLastError() != ERROR_IO_PENDING)
		{
			fs->m_asyncsPool.Free(a);
			return 0;
		}

		a->m_file = f;
		a->m_elementSize = size;
		a->m_getDataFunc = (ueAsync_GetDataFunc) ioFile_Native_Async_GetDataFunc;
		a->m_pollFunc = (ueAsync_PollFunc) ioFile_Native_Async_PollFunc;
		a->m_waitFunc = (ueAsync_WaitFunc) ioFile_Native_Async_WaitFunc;
		a->m_destroyFunc = (ueAsync_DestroyFunc) ioFile_Native_Async_DestroyFunc;

		*async = a;
		return 0;
	}

	// Synchronous read

	if (!count)
		return 0;

	DWORD numBytesRead;
	if (!ReadFile(f->m_handle, data, (DWORD) (size * count), &numBytesRead, NULL))
		return 0;

	return numBytesRead / size;
}

ueSize ioFile_Native_Write(ioFile_Native* f, const void* data, ueSize size, ueSize count)
{
	UE_ASSERT(size > 0);
	if (!count)
		return 0;

	DWORD numBytesWritten;
	if (!WriteFile(f->m_handle, data, (DWORD) (size * count), &numBytesWritten, NULL))
		return 0;

	return numBytesWritten / size;
}

ueBool ioFile_Native_Seek(ioFile_Native* f, ioFileSeekType seekType, ueSize offset)
{
	DWORD win32SeekMode = 0;
	switch (seekType)
	{
		case ioFileSeekType_Set:	win32SeekMode = FILE_BEGIN; break;
		case ioFileSeekType_Offset:	win32SeekMode = FILE_CURRENT; break;
		case ioFileSeekType_End:	win32SeekMode = FILE_END; break;
	}

	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = SetFilePointer(f->m_handle, li.LowPart, &li.HighPart, win32SeekMode);
	return INVALID_SET_FILE_POINTER != li.LowPart;
}

ueSize ioFile_Native_GetOffset(ioFile_Native* f)
{
	LARGE_INTEGER li;
	li.QuadPart = 0;
	li.LowPart = SetFilePointer(f->m_handle, 0, &li.HighPart, FILE_CURRENT);
	return INVALID_SET_FILE_POINTER != li.LowPart ? (ueSize) li.QuadPart : -1;
}

ueBool ioFile_Native_GetAttrs(ioFileSys_Native* fs, const char* path, ioFileAttrs& attrs)
{
	MAKE_FULL_PATH(fullPath, path);

	WIN32_FILE_ATTRIBUTE_DATA win32Attrs;
	if (!GetFileAttributesEx(fullPath, GetFileExInfoStandard, &win32Attrs))
		return UE_FALSE;

	ueDateTime::ToDateTime(attrs.m_creationDate, win32Attrs.ftCreationTime);
	ueDateTime::ToDateTime(attrs.m_modificationDate, win32Attrs.ftLastWriteTime);
	ueDateTime::ToDateTime(attrs.m_accessDate, win32Attrs.ftLastAccessTime);

	if (win32Attrs.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE))
	{
		attrs.m_size = 0;
		attrs.m_isDirectory = UE_TRUE;
	}
	else
	{
		attrs.m_size = win32Attrs.nFileSizeLow | (ueSize) ((u64) win32Attrs.nFileSizeHigh << (u64) 32);
		attrs.m_isDirectory = UE_FALSE;
	}

	return UE_TRUE;
}

ueBool ioFile_Native_Exists(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);

	WIN32_FILE_ATTRIBUTE_DATA win32Attrs;
	if (GetFileAttributesEx(fullPath, GetFileExInfoStandard, &win32Attrs) == 0)
		return UE_FALSE;

	return (win32Attrs.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE)) == 0;
}

ueBool ioFile_Native_Delete(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);
	return ::DeleteFile(fullPath) != 0;
}

ueBool ioFile_Native_Copy(ioFileSys_Native* fs, const char* srcPath, const char* dstPath, ueBool failIfExists)
{
	MAKE_FULL_PATH(fullSrcPath, srcPath);
	MAKE_FULL_PATH(fullDstPath, dstPath);
	return CopyFile(fullSrcPath, fullDstPath, failIfExists ? TRUE : FALSE) != 0;
}

ueBool ioDir_Native_Exists(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);

	WIN32_FILE_ATTRIBUTE_DATA win32Attrs;
	if (GetFileAttributesEx(fullPath, GetFileExInfoStandard, &win32Attrs) == 0)
		return UE_FALSE;

	return (win32Attrs.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE)) != 0;
}

ueBool ioDir_Native_Create(ioFileSys_Native* fs, const char* path, ueBool failIfExists)
{
	MAKE_FULL_PATH(fullPath, path);

	WIN32_FILE_ATTRIBUTE_DATA win32Attrs;
	if (GetFileAttributesEx(fullPath, GetFileExInfoStandard, &win32Attrs) != 0)
		return !failIfExists;

	return ::CreateDirectory(fullPath, NULL) != 0;
}

ueBool ioDir_Native_Delete(ioFileSys_Native* fs, const char* path, ueBool failIfNotEmpty = true)
{
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

ioFileSys* ioFileSys_CreateNative(ioNativeFileSysParams* params)
{
	ioFileSys_Native* fs = new(params->m_allocator) ioFileSys_Native();
	if (!fs) return NULL;

	fs->m_allocator = params->m_allocator;
	fs->m_rootLength = ueStrLen(params->m_root);
	ueMemCpy(fs->m_root, params->m_root, fs->m_rootLength + 1);

	// Initialize file system functions

	fs->m_destroyFileSystem = (ioFileSys_DestroyFunc) ioFileSys_Native_Destroy;
	fs->m_enumerate = (ioFileSys_EnumerateFunc) ioFileSys_Native_Enumerate;

	fs->m_open = (ioFile_OpenFunc) ioFile_Native_Open;
	fs->m_close = (ioFile_CloseFunc) ioFile_Native_Close;
	fs->m_read = (ioFile_ReadFunc) ioFile_Native_Read;
	fs->m_write = (ioFile_WriteFunc) ioFile_Native_Write;
	fs->m_seek = (ioFile_SeekFunc) ioFile_Native_Seek;
	fs->m_getOffset = (ioFile_GetOffsetFunc) ioFile_Native_GetOffset;
	fs->m_getAttrs = (ioFile_GetAttrsFunc) ioFile_Native_GetAttrs;
	fs->m_existsFile = (ioFile_ExistsFunc) ioFile_Native_Exists;
	fs->m_deleteFile = (ioFile_DeleteFunc) ioFile_Native_Delete;
	fs->m_copyFile = (ioFile_CopyFunc) ioFile_Native_Copy;

	fs->m_existsDir = (ioDir_ExistsFunc) ioDir_Native_Exists;
	fs->m_createDir = (ioDir_CreateFunc) ioDir_Native_Create;
	fs->m_deleteDir = (ioDir_DeleteFunc) ioDir_Native_Delete;

	// Initialize file handles pool

	UE_ASSERT_FUNC(fs->m_filesPool.Init(params->m_allocator, sizeof(ioFile_Native), params->m_maxFilesOpen));

	// Initialize async handles pool

	if (params->m_maxJobs > 0)
		UE_ASSERT_FUNC(fs->m_asyncsPool.Init(params->m_allocator, sizeof(ioFile_Native_Async), params->m_maxJobs));

	return fs;
}
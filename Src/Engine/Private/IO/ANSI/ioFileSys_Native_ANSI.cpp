#include "IO/ioFileSys_Private.h"
#include "Containers/ueGenericPool.h"
#include "Base/ueJobSys.h"

#if defined(UE_WIN32)
    #include <direct.h>
	#define __S_IFDIR _S_IFDIR
#endif

#if defined(UE_LINUX) || defined(UE_MARMALADE)
	#include <dirent.h>
#endif

#if !defined(__S_IFDIR)
	#define __S_IFDIR _S_IFDIR
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAKE_FULL_PATH(fullPath, path) \
    uePath fullPath; \
    ueStrFormatS(fullPath, "%s%s", fs->m_root, path);

// File
// ---------------------

struct ioFile_Native : ioFile
{
	FILE* m_handle;
};

// File system
// ---------------------

struct ioFileSys_Native : ioFileSys
{
	ueGenericPool m_filesPool;
	uePath m_root;
	u32 m_rootLength;
};

void ioFileSys_Native_Destroy(ioFileSys_Native* fs)
{
	if (fs->m_jobSys)
		ueJobSys_Destroy(fs->m_jobSys);
	fs->m_filesPool.Deinit();
	ueDelete(fs, fs->m_allocator);
}

void ioFileSys_Native_EnumerateRec(ioFileSys_Native* fs, char* path, u32 pathLength, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, u32 level, void* userData)
{
#if defined(UE_LINUX) || defined(UE_MARMALADE)

	MAKE_FULL_PATH(fullPath, path);

	DIR* d = opendir(fullPath);
	if (!d)
		return;

	while (dirent* de = readdir(d))
	{
		ioEnumInfo info;
		info.m_isDirectory = (de->d_type & DT_DIR) ? UE_TRUE : UE_FALSE;
		info.m_name = de->d_name;
		info.m_path = path + fs->m_rootLength;
		info.m_level = level;

		cb(&info, userData);
	}

	closedir(d);

#else

	#error Unsupported Platform

#endif
}

void ioFileSys_Native_Enumerate(ioFileSys_Native* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData)
{
	MAKE_FULL_PATH(fullPath, path);
	ioFileSys_Native_EnumerateRec(fs, fullPath, ueStrLen(fullPath), pattern, cb, enumFlags, 0, userData);
}

ioFile* ioFile_Native_Open(ioFileSys_Native* fs, const char* path, u32 openFlags)
{
	MAKE_FULL_PATH(fullPath, path);

	const ueBool isRead = (openFlags & ioFileOpenFlags_Read) ? UE_TRUE : UE_FALSE;
	const ueBool isWrite = (openFlags & ioFileOpenFlags_Write) ? UE_TRUE : UE_FALSE;

    const char* ansiMode = NULL;
    if (isRead && !isWrite)
		ansiMode = "rb";
	else if (!isRead && isWrite)
        ansiMode = "wb";
	else
	{
		UE_NOT_IMPLEMENTED();
		return NULL;
	}

    FILE* handle = fopen(fullPath, ansiMode);
    if (!handle)
        return NULL;

	ioFile_Native* f = new(fs->m_filesPool) ioFile_Native();
	UE_ASSERT(f);
	f->m_handle = handle;
	f->m_fs = fs;
	return f;
}

void ioFile_Native_Close(ioFile_Native* f)
{
	UE_ASSERT(f->m_handle);
	fclose(f->m_handle);
	((ioFileSys_Native*) f->m_fs)->m_filesPool.Free(f);
}

ueSize ioFile_Native_Read(ioFile_Native* f, void* data, ueSize size, ueSize count, ueAsync** async)
{
	UE_ASSERT(size > 0);
	if (async)
	{
		*async = ioFile_CreateGenericAsyncOperation(f->m_fs->m_allocator, f->m_fs->m_jobSys, f, data, size, count);
		return 0;
	}
	return fread(data, size, count, f->m_handle);
}

ueSize ioFile_Native_Write(ioFile_Native* f, const void* data, ueSize size, ueSize count)
{
	UE_ASSERT(size > 0);
	return fwrite(data, size, count, f->m_handle);
}

ueBool ioFile_Native_Seek(ioFile_Native* f, ioFileSeekType seekType, ueSize offset)
{
	s32 ansiSeekMode = 0;
	switch (seekType)
	{
		case ioFileSeekType_Set:	ansiSeekMode = SEEK_SET; break;
		case ioFileSeekType_Offset:	ansiSeekMode = SEEK_CUR; break;
		case ioFileSeekType_End:	ansiSeekMode = SEEK_END; break;
	}

    return fseek(f->m_handle, offset, ansiSeekMode) == 0;
}

ueSize ioFile_Native_GetOffset(ioFile_Native* f)
{
    return ftell(f->m_handle);
}

ueBool ioFile_Native_GetAttrs(ioFileSys_Native* fs, const char* path, ioFileAttrs& attrs)
{
	MAKE_FULL_PATH(fullPath, path);

    struct stat st;
	if (stat(fullPath, &st) != 0) return UE_FALSE;

	if (attrs.m_flags & ioFileAttrFlags_IsDirectory)
		attrs.m_isDirectory = (st.st_mode & __S_IFDIR) != 0;

	if (attrs.m_flags & ioFileAttrFlags_CreationDate)
		ueDateTime::ToDateTime(attrs.m_creationDate, st.st_ctime);
	if (attrs.m_flags & ioFileAttrFlags_ModificationDate)
		ueDateTime::ToDateTime(attrs.m_modificationDate, st.st_mtime);
	if (attrs.m_flags & ioFileAttrFlags_AccessDate)
		ueDateTime::ToDateTime(attrs.m_accessDate, st.st_atime);

	if (!(st.st_mode & __S_IFDIR) && (attrs.m_flags & ioFileAttrFlags_Size))
		attrs.m_size = st.st_size;

	return UE_TRUE;
}

ueBool ioFile_Native_Exists(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);

	FILE* f = fopen(fullPath, "rb");
	if (!f) return UE_FALSE;
    fclose(f);

	return UE_TRUE;
}

ueBool ioFile_Native_Delete(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);
    return remove(fullPath) == 0;
}

ueBool ioFile_Native_Copy(ioFileSys_Native* fs, const char* srcPath, const char* dstPath, ueBool failIfExists)
{
	MAKE_FULL_PATH(fullSrcPath, srcPath);
	MAKE_FULL_PATH(fullDstPath, dstPath);
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

ueBool ioDir_Native_Exists(ioFileSys_Native* fs, const char* path)
{
	MAKE_FULL_PATH(fullPath, path);
#if defined(UE_MARMALADE) && 0
	return s3eFileCheckExists(fullPath) ? UE_TRUE : UE_FALSE;
#else
	struct stat st;
	if (stat(fullPath, &st) != 0) return UE_FALSE;
	return (st.st_mode & __S_IFDIR) != 0;
#endif
}

ueBool ioDir_Native_Create(ioFileSys_Native* fs, const char* path, ueBool failIfExists)
{
	MAKE_FULL_PATH(fullPath, path);
#if defined(UE_WIN32)
	const int result = mkdir(fullPath);
#else
    const int result = mkdir(fullPath, 0);
#endif
	if (result == 0)
		return UE_TRUE;
	return errno == EEXIST;
}

ueBool ioDir_Native_Delete(ioFileSys_Native* fs, const char* path, ueBool failIfNotEmpty)
{
    MAKE_FULL_PATH(fullPath, path);
	const int result = rmdir(fullPath);
	if (result == 0) return UE_TRUE;
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

	fs->m_filesPool.Init(params->m_allocator, sizeof(ioFile_Native), params->m_maxFilesOpen);

	// Initialize job system
/*
	if (params->m_maxJobs > 0)
	{
		ueJobSysThreadDesc jobSysThread;
		jobSysThread.m_name = "ue_ansi_fs_worker";
		jobSysThread.m_processor = 0;

		ueJobSysParams jobSysParams;
		jobSysParams.m_allocator = params->m_allocator;
		jobSysParams.m_jobExtraDataSize = sizeof(ueAsync);
		jobSysParams.m_maxJobs = params->m_maxJobs;
		jobSysParams.m_threadDescs = &jobSysThread;
		jobSysParams.m_numThreads = 1;

		fs->m_jobSys = ueJobSys_Create(jobSysParams);
		UE_ASSERT(fs->m_jobSys);
	}
*/
	return fs;
}
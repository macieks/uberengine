#include "IO/ioFileSys_Private.h"
#include "Base/Containers/ueGenericPool.h"

// ZZIP
// ----------------

#include <zzip/zzip.h>

typedef struct zzip_dir		ZZIP_DIR;
typedef struct zzip_file	ZZIP_FILE;

const char* ioFileSys_GetZzipErrorDescription(zzip_error_t zzipError) 
{
	switch (zzipError)
	{
		case ZZIP_NO_ERROR: return NULL;
#define CASE(value) case value: return #value;
		CASE(ZZIP_OUTOFMEM)
		CASE(ZZIP_DIR_OPEN)
		CASE(ZZIP_DIR_STAT) 
		CASE(ZZIP_DIR_SEEK)
		CASE(ZZIP_DIR_READ)
		CASE(ZZIP_UNSUPP_COMPR)
		CASE(ZZIP_CORRUPTED)
#undef CASE
		default:
			return "<Unknown Error>";
	}
}

void ioFileSys_CheckZzipError(zzip_error_t zzipError, const char* path, const char* operation)
{
	const char* errorDesc = ioFileSys_GetZzipErrorDescription(zzipError);
	if (!errorDesc)
		return;
	ueLogE("Error with '%s' file during %s: %s", path, operation, errorDesc);
}

// Zip file
// ----------------

struct ioFile_Zip : ioFile
{
	ZZIP_FILE* m_file;
};

ueSize ioFile_Zip_Read(ioFile_Zip* f, void* data, ueSize size, u32 count, ueAsync** async)
{
	if (async)
	{
		*async = ioFile_CreateGenericAsyncOperation(f->m_fs->m_allocator, f->m_fs->m_jobSys, f, data, size, count);
		return 0;
	}
	return (ueSize) zzip_fread(data, (zzip_size_t) size, count, f->m_file);
}

ueBool ioFile_Zip_Seek(ioFile_Zip* f, ioFileSeekType seekType, ueSize offset)
{
	int ansiSeekMode = 0;
	switch (seekType)
	{
		case ioFileSeekType_Set:	ansiSeekMode = SEEK_SET; break;
		case ioFileSeekType_Offset:	ansiSeekMode = SEEK_CUR; break;
		case ioFileSeekType_End:	ansiSeekMode = SEEK_END; break;
	}
	return zzip_seek(f->m_file, (zzip_off_t) offset, ansiSeekMode) != -1;
}

ueSize ioFile_Zip_GetOffset(ioFile_Zip* f)
{
	return zzip_tell(f->m_file);
}

// Zip file system
// ----------------

struct ioFileSys_Zip : ioFileSys
{
	ZZIP_DIR* m_dir;
	ueGenericPool m_handlesPool;
};

void ioFileSys_Zip_Destroy(ioFileSys_Zip* fs)
{
	UE_ASSERT(fs->m_handlesPool.Size() == 0);
	fs->m_handlesPool.Deinit();
	zzip_dir_close(fs->m_dir);
	fs->m_allocator->Free(fs);
}

ioFile* ioFile_Zip_Open(ioFileSys_Zip* fs, const char* path, u32 mode)
{
	UE_ASSERT(mode & ioFileOpenFlags_Read);
	UE_ASSERT(!(mode & ioFileOpenFlags_Write));

	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return NULL;
	path += fs->m_mountPathLength;

	ZZIP_FILE* zzipFile = zzip_file_open(fs->m_dir, path, ZZIP_CASELESS);
	if (!zzipFile)
		return NULL;

	ioFile_Zip* f = new(fs->m_handlesPool) ioFile_Zip;
	UE_ASSERT(f);
	f->m_fs = fs;
	f->m_file = zzipFile;
	return f;
}

void ioFile_Zip_Close(ioFile_Zip* f)
{
	zzip_file_close(f->m_file);
	((ioFileSys_Zip*) f->m_fs)->m_handlesPool.Free(f);
}

ueBool ioFile_Zip_Exists(ioFileSys_Zip* fs, const char* path)
{
	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	ZZIP_STAT zstat;
	const int res = zzip_dir_stat(fs->m_dir, path, &zstat, ZZIP_CASELESS);
	return (res == ZZIP_NO_ERROR) && zstat.st_size > 0;
}

ueBool ioFile_Zip_GetAttrs(ioFileSys_Zip* fs, const char* path, ioFileAttrs& attrs)
{
	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	ZZIP_STAT zstat;
	const int res = zzip_dir_stat(fs->m_dir, path, &zstat, ZZIP_CASELESS);
	if (res == ZZIP_NO_ERROR)
	{
		if (attrs.m_flags & ioFileAttrFlags_Size)
			attrs.m_size = zstat.st_size;
		if (attrs.m_flags & ioFileAttrFlags_IsDirectory)
			attrs.m_isDirectory = zstat.st_size != 0;
		return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ioDir_Zip_Exists(ioFileSys_Zip* fs, const char* path)
{
	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	ZZIP_STAT zstat;
	const int res = zzip_dir_stat(fs->m_dir, path, &zstat, ZZIP_CASELESS);
	return (res == ZZIP_NO_ERROR) && zstat.st_size == 0;
}

void ioFileSys_Zip_Enumerate(ioFileSys_Zip* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData)
{
	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return;
	path += fs->m_mountPathLength;

	const u32 pathLength = ueStrLen(path);

	uePath fullPath;
	ueMemCpy(fullPath, fs->m_mountPath, fs->m_mountPathLength);

    while (ZZIP_DIRENT* d = zzip_readdir(fs->m_dir))
    {
		if (pathLength && ueStrNCmp(path, d->d_name, pathLength))
			continue;

		ueStrCpy((char*) fullPath + fs->m_mountPathLength, UE_ARRAY_SIZE(fullPath) - fs->m_mountPathLength, d->d_name);

		ioEnumInfo info;
		info.m_isDirectory = d->st_size == 0;
		info.m_name = d->d_name;
		info.m_path = fullPath;
		info.m_level = 0;

		if (((enumFlags & ioEnumFlags_Dirs) && info.m_isDirectory) ||
			((enumFlags & ioEnumFlags_Files) && !info.m_isDirectory))
			cb(&info, userData);
    }
    zzip_rewinddir(fs->m_dir);
}

ioFileSys* ioFileSys_CreateZip(ioArchiveFileSysParams* params)
{
	zzip_error_t zzipError = ZZIP_NO_ERROR;
	ZZIP_DIR* dir = zzip_dir_open(params->m_path, &zzipError);
	if (!dir)
	{
		ioFileSys_CheckZzipError(zzipError, params->m_path, "opening archive");
		return NULL;
	}

	ioFileSys_Zip* fs = new(params->m_allocator) ioFileSys_Zip();
	UE_ASSERT(fs);
	fs->m_allocator = params->m_allocator;
	fs->m_dir = dir;

	UE_ASSERT_FUNC(fs->m_handlesPool.Init(fs->m_allocator, sizeof(ioFile_Zip), params->m_maxFilesOpen));

	// Initialize file system functions

	fs->m_destroyFileSystem = (ioFileSys_DestroyFunc) ioFileSys_Zip_Destroy;
	fs->m_enumerate = (ioFileSys_EnumerateFunc) ioFileSys_Zip_Enumerate;

	fs->m_open = (ioFile_OpenFunc) ioFile_Zip_Open;
	fs->m_close = (ioFile_CloseFunc) ioFile_Zip_Close;
	fs->m_read = (ioFile_ReadFunc) ioFile_Zip_Read;
	fs->m_write = NULL;
	fs->m_seek = (ioFile_SeekFunc) ioFile_Zip_Seek;
	fs->m_getOffset = (ioFile_GetOffsetFunc) ioFile_Zip_GetOffset;
	fs->m_getAttrs = (ioFile_GetAttrsFunc) ioFile_Zip_GetAttrs;
	fs->m_existsFile = (ioFile_ExistsFunc) ioFile_Zip_Exists;
	fs->m_deleteFile = NULL;
	fs->m_copyFile = NULL;

	fs->m_existsDir = (ioDir_ExistsFunc) ioDir_Zip_Exists;
	fs->m_createDir = NULL;
	fs->m_deleteDir = NULL;

	return fs;
}
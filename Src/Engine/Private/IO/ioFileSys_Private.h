#ifndef IO_FILE_SYS_PRIVATE_H
#define IO_FILE_SYS_PRIVATE_H

#include "IO/ioFile.h"

typedef void (*ioFileSys_DestroyFunc)(ioFileSys* fs);
typedef void (*ioFileSys_EnumerateFunc)(ioFileSys* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData);

typedef ioFile* (*ioFile_OpenFunc)(ioFileSys* fs, const char* path, u32 openFlags);
typedef void (*ioFile_CloseFunc)(ioFile* f);
typedef ueSize (*ioFile_ReadFunc)(ioFile* f, void* data, ueSize size, ueSize count, ueAsync** async);
typedef ueSize (*ioFile_WriteFunc)(ioFile* f, const void* data, ueSize size, ueSize count);
typedef ueBool (*ioFile_SeekFunc)(ioFile* f, ioFileSeekType seekType, ueSize offset);
typedef ueSize (*ioFile_GetOffsetFunc)(ioFile* f);
typedef ueBool (*ioFile_GetAttrsFunc)(ioFileSys* fs, const char* path, ioFileAttrs& attrs);
typedef ueBool (*ioFile_ExistsFunc)(ioFileSys* fs, const char* path);
typedef ueBool (*ioFile_DeleteFunc)(ioFileSys* fs, const char* path);
typedef ueBool (*ioFile_CopyFunc)(ioFileSys* fs, const char* srcPath, const char* dstPath, ueBool failIfExists);

typedef ueBool (*ioDir_ExistsFunc)(ioFileSys* fs, const char* path);
typedef ueBool (*ioDir_CreateFunc)(ioFileSys* fs, const char* path, ueBool failIfExists);
typedef ueBool (*ioDir_DeleteFunc)(ioFileSys* fs, const char* path, ueBool failIfNotEmpty);
typedef ueBool (*ioDir_CopyFunc)(ioFileSys* fs, const char* srcPath, const char* dstPath);

struct ioFileSys
{
	ioFileSys_DestroyFunc m_destroyFileSystem;
	ioFileSys_EnumerateFunc m_enumerate;

	ioFile_OpenFunc m_open;
	ioFile_CloseFunc m_close;
	ioFile_ReadFunc m_read;
	ioFile_WriteFunc m_write;
	ioFile_SeekFunc m_seek;
	ioFile_GetOffsetFunc m_getOffset;
	ioFile_GetAttrsFunc m_getAttrs;
	ioFile_ExistsFunc m_existsFile;
	ioFile_DeleteFunc m_deleteFile;
	ioFile_CopyFunc m_copyFile;

	ioDir_ExistsFunc m_existsDir;
	ioDir_CreateFunc m_createDir;
	ioDir_DeleteFunc m_deleteDir;
	ioDir_CopyFunc m_copyDir;

	ueAllocator* m_allocator;	//!< Allocator used by the file system

	ueJobSys* m_jobSys;			//!< Optional job system for asynchronous operations

	uePath m_mountPath;			//!< Mount point
	u32 m_mountPathLength;		//!< Mount point path length

	UE_INLINE ioFileSys() { ueMemZero(this, sizeof(ioFileSys)); }
};

struct ioFile
{
	ioFileSys* m_fs;

	UE_INLINE ioFile() : m_fs(NULL) {}
};

ueAsync* ioFile_CreateGenericAsyncOperation(ueAllocator* allocator, ueJobSys* jobSys, ioFile* file, void* data, ueSize size, ueSize count);

#endif // IO_FILE_SYS_PRIVATE_H

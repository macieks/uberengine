#include "IO/ioFileSys_Private.h"
#include "IO/ioFileSys_AFS_Types.h"
#include "IO/ioPackage.h"

/**
 *	Run-time side Archive File System (AFS).
 *
 *	Features:
 *	- loads all management data into single memory block (nice!)
 *	- each file corresponds with single ioPackageSegment
 *	- internally uses ioPackage class for serialization
 */
struct ioFileSys_AFS : ioFileSys
{
	ioPackage* m_package;
	const ioDirInfo_AFS* m_rootDir;
};

void ioFileSys_AFS_Destroy(ioFileSys_AFS* fs)
{
	ioPackage_Destroy(fs->m_package);
	fs->m_allocator->Free(fs);
}

const ioFileInfo_AFS* ioFileSys_AFS_GetFile(ioFileSys_AFS* fs, const char* path)
{
	const char* pathPtr = path;
	const ioDirInfo_AFS* dirInfo = fs->m_rootDir;
	while (*pathPtr)
	{
		ueBool isLastPart = UE_FALSE;

		s32 firstSeparatorIndex = ueStrFirstIndexOf(pathPtr, '/');
		if (firstSeparatorIndex == -1)
		{
			firstSeparatorIndex = ueStrFirstIndexOf(pathPtr, '\\');
			if (firstSeparatorIndex == -1)
			{
				isLastPart = UE_TRUE;
				firstSeparatorIndex = ueStrLen(pathPtr);
			}
		}

		if (isLastPart)
		{
			for (u32 i = 0; i < dirInfo->m_numFiles; i++)
				if (!ueStrNICmp(pathPtr, dirInfo->m_files[i].m_name, firstSeparatorIndex))
					return &dirInfo->m_files[i];
			return NULL;
		}

		u32 foundDirIndex = U32_MAX;
		for (u32 i = 0; i < dirInfo->m_numSubDirs; i++)
			if (!ueStrNICmp(pathPtr, dirInfo->m_subDirs[i].m_name, firstSeparatorIndex))
			{
				foundDirIndex = i;
				break;
			}
		if (foundDirIndex == U32_MAX)
			return NULL;

		pathPtr += firstSeparatorIndex + 1;
		dirInfo = &dirInfo->m_subDirs[foundDirIndex];
	}
	return NULL;
}

const ioDirInfo_AFS* ioFileSys_AFS_GetDir(ioFileSys_AFS* fs, const char* path)
{
	if (!path[0])
		return fs->m_rootDir;
	const char* pathPtr = path;
	const ioDirInfo_AFS* dirInfo = fs->m_rootDir;
	while (*pathPtr)
	{
		ueBool isLastPart = UE_FALSE;

		s32 firstSeparatorIndex = ueStrFirstIndexOf(pathPtr, '/');
		if (firstSeparatorIndex == -1)
		{
			firstSeparatorIndex = ueStrFirstIndexOf(pathPtr, '\\');
			if (firstSeparatorIndex == -1)
			{
				isLastPart = UE_TRUE;
				firstSeparatorIndex = ueStrLen(pathPtr);
			}
		}

		u32 foundDirIndex = U32_MAX;
		for (u32 i = 0; i < dirInfo->m_numSubDirs; i++)
			if (!ueStrNICmp(pathPtr, dirInfo->m_subDirs[i].m_name, firstSeparatorIndex))
			{
				foundDirIndex = i;
				break;
			}
		if (foundDirIndex == U32_MAX)
			return NULL;

		if (isLastPart)
			return &dirInfo->m_subDirs[foundDirIndex];

		pathPtr += firstSeparatorIndex + 1;
		dirInfo = &dirInfo->m_subDirs[foundDirIndex];
	}
	return NULL;
}

ioFile* ioFile_AFS_Open(ioFileSys_AFS* fs, const char* path, u32 openFlags)
{
	if (openFlags & (ioFileOpenFlags_Write | ioFileOpenFlags_Create))
		return NULL;

	if (ueStrNICmp(fs->m_mountPath, path, fs->m_mountPathLength)) return NULL;
	path += fs->m_mountPathLength;

	const ioFileInfo_AFS* fileInfo = ioFileSys_AFS_GetFile(fs, path);
	return fileInfo ? ioPackage_OpenSegmentAsFile(fs->m_allocator, fs->m_package, fileInfo->m_segmentIndex) : NULL;
}

ueBool ioFile_AFS_Exists(ioFileSys_AFS* fs, const char* path)
{
	if (ueStrNICmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	return ioFileSys_AFS_GetFile(fs, path) != NULL;
}

ueBool ioDir_AFS_Exists(ioFileSys_AFS* fs, const char* path)
{
	if (ueStrNICmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	return ioFileSys_AFS_GetDir(fs, path) != NULL;
}

ueBool ioFile_AFS_GetAttrs(ioFileSys_AFS* fs, const char* path, ioFileAttrs& attrs)
{
	if (ueStrNICmp(fs->m_mountPath, path, fs->m_mountPathLength)) return UE_FALSE;
	path += fs->m_mountPathLength;

	ueMemSet(&attrs, 0, sizeof(ioFileAttrs));

	const ioFileInfo_AFS* file = ioFileSys_AFS_GetFile(fs, path);
	if (file)
	{
		attrs.m_flags = ioFileAttrFlags_IsDirectory | ioFileAttrFlags_Size;
		attrs.m_isDirectory = UE_FALSE;
		attrs.m_size = file->m_size;
		return UE_TRUE;
	}

	const ioDirInfo_AFS* dir = ioFileSys_AFS_GetDir(fs, path);
	if (dir)
	{
		attrs.m_flags = ioFileAttrFlags_IsDirectory;
		attrs.m_isDirectory = UE_TRUE;
		return UE_TRUE;
	}

	return UE_FALSE;
}

void ioFileSys_AFS_EnumerateRec(ioFileSys_AFS* fs, const ioDirInfo_AFS* dir, char* path, u32 pathLength, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, u32 level, void* userData)
{
	for (u32 i = 0; i < dir->m_numSubDirs; i++)
	{
		const ioDirInfo_AFS* subDir = &dir->m_subDirs[i];

		const u32 nameLength = ueStrLen(subDir->m_name);
		ueMemCpy(path + pathLength, subDir->m_name, nameLength + 1);
		path[pathLength + nameLength] = '/';
		path[pathLength + nameLength + 1] = 0;

		if ((enumFlags & ioEnumFlags_Dirs) && ueWildcardMatches(subDir->m_name, pattern))
		{
			ioEnumInfo info;
			info.m_isDirectory = UE_TRUE;
			info.m_name = subDir->m_name;
			info.m_path = path - fs->m_mountPathLength;
			info.m_level = level;

			cb(&info, userData);
		}

		if (enumFlags & ioEnumFlags_Recursive)
			ioFileSys_AFS_EnumerateRec(fs, subDir, path, pathLength + nameLength + 1, pattern, cb, enumFlags, level + 1, userData);
	}

	if (enumFlags & ioEnumFlags_Files)
		for (u32 i = 0; i < dir->m_numFiles; i++)
		{
			const ioFileInfo_AFS* file = &dir->m_files[i];

			if (!ueWildcardMatches(file->m_name, pattern))
				continue;

			const u32 nameLength = ueStrLen(file->m_name);
			ueMemCpy(path + pathLength, file->m_name, nameLength + 1);

			ioEnumInfo info;
			info.m_isDirectory = UE_FALSE;
			info.m_name = file->m_name;
			info.m_path = path - fs->m_mountPathLength;
			info.m_level = level;

			cb(&info, userData);
		}
}

void ioFileSys_AFS_Enumerate(ioFileSys_AFS* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags, void* userData)
{
	uePath _path;
	ueStrCpyS(_path, path);

	if (ueStrNCmp(fs->m_mountPath, path, fs->m_mountPathLength)) return;
	path += fs->m_mountPathLength;

	const ioDirInfo_AFS* dir = ioFileSys_AFS_GetDir(fs, path);
	if (!dir) return;
	ioFileSys_AFS_EnumerateRec(fs, dir, (char*) _path + fs->m_mountPathLength, ueStrLen((char*) _path + fs->m_mountPathLength), pattern, cb, enumFlags, 0, userData);
}

ioFileSys* ioFileSys_CreateAFS(ioArchiveFileSysParams* params)
{
	// Load package

	ioPackageLoadParams packageParams;
	packageParams.m_allocator = params->m_allocator;
	packageParams.m_file = params->m_file;
	packageParams.m_path = params->m_path;
	ioPackage* package = ioPackage_Load(&packageParams);
	if (!package)
		return NULL;

	// Load first (management) segment

	if (!ioPackage_LoadSegment(package, 0))
	{
		ioPackage_Destroy(package);
		return NULL;
	}

	// Create AFS

	ioFileSys_AFS* fs = new(params->m_allocator) ioFileSys_AFS();
	UE_ASSERT(fs);
	fs->m_allocator = params->m_allocator;
	fs->m_package = package;
	ioPackage_GetSegmentData(fs->m_package, 0, (void**) &fs->m_rootDir);

	fs->m_destroyFileSystem = (ioFileSys_DestroyFunc) ioFileSys_AFS_Destroy;
	fs->m_enumerate = (ioFileSys_EnumerateFunc) ioFileSys_AFS_Enumerate;

	fs->m_open = (ioFile_OpenFunc) ioFile_AFS_Open;
	fs->m_getAttrs = (ioFile_GetAttrsFunc) ioFile_AFS_GetAttrs;
	fs->m_existsFile = (ioFile_ExistsFunc) ioFile_AFS_Exists;
	// Read, seek and get-offset functions are redirected from ioFile_PackageSegment_*

	fs->m_existsDir = (ioDir_ExistsFunc) ioDir_AFS_Exists;

	return fs;
}

const ioDirInfo_AFS* ioFileSys_AFS_GetRootDir(ioFileSys* fs)
{
	return ((ioFileSys_AFS*) fs)->m_rootDir;
}
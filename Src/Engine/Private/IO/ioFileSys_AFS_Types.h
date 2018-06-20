#ifndef IO_FILE_SYS_AFS_TYPES_H
#define IO_FILE_SYS_AFS_TYPES_H

#include "Base/ueBase.h"

struct ioFileInfo_AFS
{
	const char* m_name;

	u32 m_size;
	u32 m_segmentIndex;
};

struct ioDirInfo_AFS
{
	const char* m_name;

	u32 m_numSubDirs;
	ioDirInfo_AFS* m_subDirs;

	u32 m_numFiles;
	ioFileInfo_AFS* m_files;
};

#endif // IO_FILE_SYS_AFS_TYPES_H
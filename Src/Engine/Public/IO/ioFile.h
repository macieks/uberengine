#ifndef IO_FILE_SYS_H
#define IO_FILE_SYS_H

#include "Base/ueAsync.h"

struct ueJobSys;

/**
 *	@addtogroup io
 *	@{
 */

/**
 *	@struct ioFile
 *	@brief File handle
 */
struct ioFile;

/**
 *	@struct ioFileSys
 *	@brief File system
 */
struct ioFileSys;

//! Returned by ioFile_GetOffset() on failure
#define IO_INVALID_OFFSET ((ueSize) -1)

//! File open flags
enum ioFileOpenFlags
{
	ioFileOpenFlags_Read		= UE_POW2(0),	//!< Enable file reads
	ioFileOpenFlags_Write		= UE_POW2(1),	//!< Enable file writes
	ioFileOpenFlags_Create		= UE_POW2(2),	//!< Create file if doesn't exist
	ioFileOpenFlags_NoBuffer	= UE_POW2(3),	//!< Disables file buffering
	ioFileOpenFlags_Async		= UE_POW2(4)	//!< Allows for asynchronous operations
};

//! File attribute flags
enum ioFileAttrFlags
{
	ioFileAttrFlags_IsDirectory		= UE_POW2(0),	//!< "is directory" true/false attribute
	ioFileAttrFlags_Size			= UE_POW2(1),	//!< File size
	ioFileAttrFlags_CreationDate	= UE_POW2(2),	//!< Creation date
	ioFileAttrFlags_ModificationDate= UE_POW2(3),	//!< Modification date
	ioFileAttrFlags_AccessDate		= UE_POW2(4),	//!< Access date

	ioFileAttrFlags_AllDates		= ioFileAttrFlags_CreationDate |
									ioFileAttrFlags_ModificationDate |
									ioFileAttrFlags_AccessDate
};

//! File attributes
struct ioFileAttrs
{
	u32 m_flags;					//!< Indicates which attributes to retrieve; see ioFileAttrFlags
	ueBool m_isDirectory;			//!< Indicates whether it is directory
	ueSize m_size;					//!< File size (valid only for files)
	ueDateTime m_creationDate;		//!< File (or directory) creation date
	ueDateTime m_modificationDate;	//!< File (or directory) modification date
	ueDateTime m_accessDate;		//!< File (or directory) access date
};

//! File seek type
enum ioFileSeekType
{
	ioFileSeekType_Set = 0,			//!< Seeks to specific offset
	ioFileSeekType_Offset,			//!< Seeks by specific offset
	ioFileSeekType_End				//!< Seeks to file end
};

//! Auto file pointer; assures file is closed on destruction
class ioFilePtr
{
public:
	ioFilePtr();
	//! Closes the file if still open
	~ioFilePtr();
	//! Opens file
	ueBool Open(const char* path, u32 flags);
	//! Opens file withing given file system
	ueBool Open(ioFileSys* fs, const char* path, u32 flags);
	//! Closes file
	void Close();
	//! Reads from file; passing in pointer to async will start asynchronous read - in that case on success async is set to non-NULL value and return value can be ignored
	ueSize Read(void* data, ueSize size, ueSize count = 1, ueAsync** async = NULL);
	//! Writes to file
	ueSize Write(const void* data, ueSize size, ueSize count = 1);
	//! Seeks file
	ueBool Seek(ioFileSeekType seekType, ueSize offset);
	//! Gets file offset; returns -1 on failure
	ueSize GetOffset();
	//! Gets file size; returns -1 on failure
	ueSize GetSize();

private:
	ioFile* m_file; //!< Handled file
};

//! File / directory enumeration information
struct ioEnumInfo
{
	const char* m_name;		//!< Just the name (of the file with extension or directory)
	const char* m_path;		//!< Full path
	ueBool m_isDirectory;	//!< Indicates whether it's a directory
	u32 m_level;			//!< Current depth level
};

//! Enumeration callback to be provided by the user
typedef ueBool (*ioEnumerateCallback)(const ioEnumInfo* info, void* userData);

//! File system enumeration flags
enum ioEnumFlags
{
	ioEnumFlags_Recursive	= UE_POW2(0),	//!< Recurse directories recursively
	ioEnumFlags_Files		= UE_POW2(1),	//!< Enumerate files
	ioEnumFlags_Dirs		= UE_POW2(2),	//!< Enumerate directories

	ioEnumFlags_Default	= ioEnumFlags_Files | ioEnumFlags_Dirs
};

//! Base file system creation parameters
struct ioFileSysParams
{
	ueAllocator* m_allocator;	//!< Allocator to be used
	u32 m_maxFilesOpen;			//!< Max. number of simulatenously opened files
	u32 m_maxJobs;				//!< Max. number of simulatenous asynchronous file operations

	ioFileSysParams() :
		m_allocator(NULL),
		m_maxFilesOpen(16),
		m_maxJobs(0)
	{}
};

//! Native file system creation parameters
struct ioNativeFileSysParams : ioFileSysParams
{
	const char* m_root; //!< Root file system path within OS file system

	ioNativeFileSysParams() : m_root("") {}
};

//! Archive file system creation parameters
struct ioArchiveFileSysParams : ioFileSysParams
{
	// Archive file either as a handle to file or a path (one of these must be set)

	ioFile* m_file;			//!< File handle or NULL if m_name is set
	const char* m_path;		//!< File path or NULL if m_file is set

	ioArchiveFileSysParams() : m_file(NULL), m_path(NULL) {}
};

// File System

//! Starts up file system manager
void ioFileSys_Startup(ueAllocator* allocator, u32 maxFileSystems = 16);
//! Shuts down file system manager
void ioFileSys_Shutdown();

//! Creates native file system
ioFileSys* ioFileSys_CreateNative(ioNativeFileSysParams* params);
//! Creates Zip file system
ioFileSys* ioFileSys_CreateZip(ioArchiveFileSysParams* params);
//! Creates AFS file system
ioFileSys* ioFileSys_CreateAFS(ioArchiveFileSysParams* params);
//! Initializes file system used for in-memory files
void ioFileSys_InitMemory(ioFileSysParams* params);
//! Destroys file system
void ioFileSys_Destroy(ioFileSys* fs);
//! Registers file system under given location
void ioFileSys_Mount(ioFileSys* fs, const char* location, u32 priority);
//! Unregisters file system
void ioFileSys_Unmount(ioFileSys* fs);
//! Enumerates files and directories
void ioFileSys_Enumerate(ioFileSys* fs, const char* path, const char* pattern, ioEnumerateCallback cb, u32 enumFlags = ioEnumFlags_Default, void* userData = NULL);
//! Sets job queue to be used by the file system for asynchronous operations; some file systems (e.g. native Win32) don't require job system, but if set it will be used
void ioFileSys_SetJobSystem(ioFileSys* fs, ueJobSys* jobSys);

//! Sets default file system
void ioFileSys_SetDefault(ioFileSys* fs);
//! Gets default file system
ioFileSys* ioFileSys_GetDefault();
//! Sets source file system
void ioFileSys_SetSourceFileSys(ioFileSys* fs);
//! Gets source file system
ioFileSys* ioFileSys_GetSourceFileSys();
//! Sets data file system
void ioFileSys_SetDataFileSys(ioFileSys* fs);
//! Gets data file system
ioFileSys* ioFileSys_GetDataFileSys();

// File

//! Opens file; see ioFileOpenFlags for open flags
ioFile* ioFile_Open(ioFileSys* fs, const char* path, u32 openFlags);
//! Opens file; see ioFileOpenFlags for open flags
ioFile* ioFile_Open(const char* path, u32 openFlags);
//! Opens file in memory
ioFile* ioFile_OpenInMemory(void* buffer, ueSize size);
//! Closes file
void ioFile_Close(ioFile* f);
/**
 *	Reads elements from file.
 *
 *	For sync reads returns number of successfully read elements.
 *
 *	For asynchronous reads:
 *	(1) returned result can be ignored
 *	(2) on success async handle is set to non-NULL
 *	(3) when done, ueAsync_GetData() returns pointer to ueSize containing number of successfully read elements
 */
ueSize ioFile_Read(ioFile* f, void* data, ueSize size, ueSize count = 1, ueAsync** async = NULL);
//! Writes elements to file; returns number of successfully written elements
ueSize ioFile_Write(ioFile* f, const void* data, ueSize size, ueSize count = 1);
//! Seeks within a file; return UE_TRUE on success and UE_FALSE otherwise
ueBool ioFile_Seek(ioFile* f, ioFileSeekType seekType, ueSize offset);
//! Returns current file offset; returns IO_INVALID_OFFSET if failed
ueSize ioFile_GetOffset(ioFile* f);

//! Gets file attributes
ueBool ioFile_GetAttrs(ioFileSys* fs, const char* path, ioFileAttrs& attrs);
//! Gets file attributes
ueBool ioFile_GetAttrs(const char* path, ioFileAttrs& attrs);
//! Tests whether file exists
ueBool ioFile_Exists(ioFileSys* fs, const char* path);
//! Tests whether file exists
ueBool ioFile_Exists(const char* path);
//! Deletes file
ueBool ioFile_Delete(ioFileSys* fs, const char* path);
//! Deletes file
ueBool ioFile_Delete(const char* path);
//! Copies file
ueBool ioFile_Copy(ioFileSys* fs, const char* srcPath, const char* dstPath, ueBool failIfExists);

//! Gets file size
ueBool ioFile_GetSize(ioFileSys* fs, const char* path, ueSize& size);
//! Gets file size
ueBool ioFile_GetSize(const char* path, ueSize& size);
//! Loads part of or whole file; optionally appends data with EOF character
ueBool ioFile_Load(ioFileSys* fs, const char* path, void*& buffer, ueSize& size, ueSize offset = 0, ueAllocator* allocator = NULL, ueBool capWithEOF = UE_FALSE);
//! Loads part of or whole file; optionally appends data with EOF character
ueBool ioFile_Load(const char* path, void*& buffer, ueSize& size, ueSize offset = 0, ueAllocator* allocator = NULL, ueBool capWithEOF = UE_FALSE);
//! Saves (and overwrites) the whole file
ueBool ioFile_Save(const char* path, const void* data, ueSize dataSize);

// Directory

//! Tests whether directory exists
ueBool ioDir_Exists(ioFileSys* fs, const char* path);
//! Tests whether directory exists
ueBool ioDir_Exists(const char* path);
//! Creates directory
ueBool ioDir_Create(ioFileSys* fs, const char* path, ueBool failIfExists);
//! Deletes directory
ueBool ioDir_Delete(ioFileSys* fs, const char* path, ueBool failIfNotEmpty);
//! Copies directory
ueBool ioDir_Copy(ioFileSys* fs, const char* srcPath, const char* dstPath);
//! Creates directory chain; if pathContainsFileName is set to true, the last path segment is skipped
ueBool ioDir_CreateChain(ioFileSys* fs, const char* path, ueBool pathContainsFileName = UE_TRUE);

// Path Utilities

//! Extracts file name from the path (returns pointer somwhere into path)
const char*		ioPath_ExtractFileName(const char* path);
//! Extracts file name withour extension from the path
ueBool			ioPath_ExtractFileNameWithoutExtension(const char* path, char* buffer, u32 bufferSize);
//! Extracts file name extension from the path (returns pointer somwhere into path)
const char*		ioPath_ExtractFileExtension(const char* path);
//! Removes extension from the path
ueBool			ioPath_RemoveExtension(char* path);
//! Extracts directory name from the path
ueBool			ioPath_ExtractDirectoryName(const char* path, char* buffer, u32 bufferSize);
//! Splits path into parts (splitting occurs at each path splitter)
ueBool			ioPath_SplitPath(const char* path, char** parts, u32& numParts);
//! Tells whether given character is a valid path splitter
UE_INLINE ueBool ioPath_IsSplitter(char c) { return c == '\\' || c == '/'; }

// @}

#endif // IO_FILE_SYS_H

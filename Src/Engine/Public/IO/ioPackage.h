#ifndef IO_PACKAGE_H
#define IO_PACKAGE_H

#include "Base/ueBase.h"

struct ioFile;

/**
 *	@addtogroup io
 *	@{
 */

/**
 *	@struct ioPackage
 *	@brief In-place loadable data package featuring automatic pointer fixup support and compression.
 *			Note: you should never read from the same package from 2 or more different threads.
 */
struct ioPackage;

//! Available package allocation types
enum ioPackageAllocType
{
	ioPackageAllocType_PackageData = 0,		//!< Package data (e.g. string pool, segment & entries headers)
	ioPackageAllocType_SegmentData,			//!< Segment data
	ioPackageAllocType_SegmentInitData		//!< Segment initialization data
};

//! Package memory allocation callback
typedef void* (*ioPackageAllocCallback)(ioPackage* package, u32 segmentIndex, ioPackageAllocType allocType, ueSize size, ueSize alignment);
//! Package memory free callback
typedef void (*ioPackageFreeCallback)(ioPackage* package, u32 segmentIndex, ioPackageAllocType allocType, void* data, ueSize size);

//! Package load parameters
struct ioPackageLoadParams
{
	// Custom allocation functions or allocator (one of these must be valid)

	ioPackageAllocCallback m_allocCallback;	//!< Memory allocation callbacks (can be NULL if m_allocator is set)
	ioPackageFreeCallback m_freeCallback;	//!< Memory free callbacks (can be NULL if m_allocator is set)

	ueAllocator* m_allocator;				//!< Optional allocator (to be used instead for memory allocation callbacks)

	const char* m_path;						//!< Path of the file to load package from
	ioFile* m_file;							//!< File to load package from

	ioPackageLoadParams() :
		m_allocCallback(NULL),
		m_freeCallback(NULL),
		m_allocator(NULL),
		m_path(NULL),
		m_file(NULL)
	{}
};

//! Loads package (header only)
ioPackage* ioPackage_Load(ioPackageLoadParams* params);
//! Destroys package
void ioPackage_Destroy(ioPackage* package);
//! Tells whether we're done loading anything from the package (which means package will no longer need file)
void ioPackage_DoneLoading(ioPackage* package);

//! Gets number of segments
u32 ioPackage_GetNumSegments(ioPackage* package);
//! Gets symbol of the segment at given index
u32 ioPackage_GetSegmentSymbol(ioPackage* package, u32 index);
//! Gets data of the segment at given index
void ioPackage_GetSegmentData(ioPackage* package, u32 index, void** data, u32* size = NULL);

//! Opens segment as file
ioFile* ioPackage_OpenSegmentAsFile(ueAllocator* allocator, ioPackage* package, u32 segmentIndex);
//! Loads all segments
ueBool ioPackage_LoadAllSegments(ioPackage* package);
//! Loads segment at given index
ueBool ioPackage_LoadSegment(ioPackage* package, u32 index);
//! Acquires data of segment at given index (meaning the owner of segment data memory is now the user)
void* ioPackage_AcquireSegmentData(ioPackage* package, u32 index);
//! Unloads segment at given index
void ioPackage_UnloadSegment(ioPackage* package, u32 index);
//! Unloads data of all segments
void ioPackage_UnloadAllSegments(ioPackage* package);

//! Gets entry data
const void* ioPackage_GetEntryData(ioPackage* package, u32 segmentIndex, u32 entryIndex);

//! Sets package user data
void ioPackage_SetUserData(ioPackage* package, void* userData);
//! Gets package user data
void* ioPackage_GetUserData(ioPackage* package);

// @}

#endif // IO_PACKAGE_H

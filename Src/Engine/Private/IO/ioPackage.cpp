#include "Base/ueResource.h"
#include "IO/ioPackage.h"
#include "IO/ioPackage_Format.h"
#include "IO/ioFileSys_Private.h"
#include "Utils/utCompression.h"

struct ioPackage
{
	u32 m_commonDataSize;
	void* m_commonData;

	u32 m_stringPoolSize;
	char* m_stringPool;

	u32 m_numSegments;
	ioPackageSegment* m_segments;

	u32 m_decompressionBufferSize;
	void* m_decompressionBuffer;

	void* m_userData;

	ioPackageLoadParams m_loadParams;

	ueBool m_ownsFile;

	ioPackage() :
		m_commonData(NULL),
		m_commonDataSize(0),
		m_stringPool(NULL),
		m_stringPoolSize(0),
		m_numSegments(0),
		m_segments(NULL),
		m_decompressionBufferSize(0),
		m_decompressionBuffer(NULL),
		m_userData(NULL),
		m_ownsFile(UE_FALSE)
	{}
};

// Helpers

UE_INLINE void* ioPackage_Alloc(ioPackage* p, u32 segmentIndex, ioPackageAllocType allocType, ueSize size, ueSize alignment)
{
	return p->m_loadParams.m_allocCallback ? p->m_loadParams.m_allocCallback(p, segmentIndex, allocType, size, alignment) : p->m_loadParams.m_allocator->Alloc(size, alignment);
}

UE_INLINE void ioPackage_Free(ioPackage* p, u32 segmentIndex, ioPackageAllocType allocType, void* data, ueSize size)
{
	return p->m_loadParams.m_allocCallback ? p->m_loadParams.m_freeCallback(p, segmentIndex, allocType, data, size) : p->m_loadParams.m_allocator->Free(data);
}

void ioPackage_DoneLoading(ioPackage* p)
{
	if (p->m_ownsFile && p->m_loadParams.m_file)
		ioFile_Close(p->m_loadParams.m_file);

	p->m_loadParams.m_file = NULL;
}

ioPackage* ioPackage_Load(ioPackageLoadParams* params)
{
	// Get file

	const ueBool ownsFile = params->m_path != NULL;
	ioFile* file = NULL;
	if (params->m_path)
	{
		file = ioFile_Open(params->m_path, ioFileOpenFlags_Read);
		if (!file)
		{
			ueLogE("Failed to load package (path = '%s'), reason: failed to open file for reading", params->m_path);
			return NULL;
		}
	}
	else
	{
		UE_ASSERT(params->m_file);
		file = params->m_file;
	}

	// Load header

	ioPackageFile_Header header;
	if (!ioFile_Read(file, &header, sizeof(header)))
	{
		ueLogE("Failed to load package (path = '%s'), reason: failed to read header (%u bytes)", params->m_path, sizeof(header));
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	if (header.m_magicId != ioPackageFile_Header::MAGIC_ID)
	{
		ueLogE("Failed to load package (path = '%s'), reason: invalid magic id, got %u, expected %u", params->m_path, header.m_magicId, ioPackageFile_Header::MAGIC_ID);
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	header.m_endianess = header.m_endianess;
	if (header.m_endianess != UE_ENDIANESS)
	{
		ueLogE("Failed to load package (path = '%s'), reason: invalid endianess", params->m_path);
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	if (header.m_version != ioPackageFile_Header::VERSION)
	{
		ueLogE("Failed to load package (path = '%s'), reason: invalid version, got %u, expected %u", params->m_path, header.m_version, ioPackageFile_Header::VERSION);
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	const ueSize packageSize = sizeof(ioPackage) + header.m_commonDataSize + header.m_decompressionBufferSize;

	ioPackage tempPackage;
	tempPackage.m_loadParams = *params;
	u8* curr = (u8*) ioPackage_Alloc(&tempPackage, NULL, ioPackageAllocType_PackageData, packageSize, UE_DEFAULT_ALIGNMENT);
	if (!curr)
	{
		ueLogE("Failed to load package (path = '%s'), reason: failed to allocate %u bytes", params->m_path, packageSize);
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	// Create package

	ioPackage* p = new(curr) ioPackage();
	curr += sizeof(ioPackage);

	p->m_loadParams = *params;
	p->m_loadParams.m_file = file;
	p->m_ownsFile = ownsFile;

	p->m_commonDataSize = header.m_commonDataSize;
	p->m_stringPoolSize = header.m_stringPoolSize;
	p->m_numSegments = header.m_numSegments;
	p->m_decompressionBufferSize = header.m_decompressionBufferSize;

	// Get common data

	p->m_commonData = p->m_commonDataSize ? curr : NULL;

	if (!ioFile_Read(p->m_loadParams.m_file, p->m_commonData, p->m_commonDataSize))
	{
		ueLogE("Failed to load package (path = '%s'), reason: failed to read common data (%u bytes)", params->m_path, p->m_commonDataSize);

		ioPackage_Free(p, NULL, ioPackageAllocType_PackageData, p->m_commonData, packageSize);
		if (ownsFile)
			ioFile_Close(file);
		return UE_FALSE;
	}

	// Get string pool

	p->m_stringPool = p->m_stringPoolSize ? (char*) p->m_commonData : NULL;
	curr += p->m_stringPoolSize;

	// Get segments and entries

	p->m_segments = (ioPackageSegment*) curr;
	curr += p->m_numSegments * sizeof(ioPackageSegment);

	for (u32 i = 0; i < p->m_numSegments; i++)
	{
		ioPackageSegment* segment = &p->m_segments[i];

		segment->m_entries = (ioPackageEntry*) curr;
		curr += segment->m_numEntries * sizeof(ioPackageEntry);

		segment->m_compressedChunkOffsets = (u32*) curr;
		curr += segment->m_numChunks * sizeof(u32);

		segment->m_compressedChunkSizes = (u32*) curr;
		curr += segment->m_numChunks * sizeof(u32);
	}

	// Set decompression buffer

	p->m_decompressionBuffer = p->m_decompressionBufferSize ? curr : NULL;

	return p;
}

void ioPackage_Destroy(ioPackage* p)
{
	if (p->m_ownsFile && p->m_loadParams.m_file)
		ioFile_Close(p->m_loadParams.m_file);

	for (u32 i = 0; i < p->m_numSegments; i++)
	{
		ioPackageSegment* segment = &p->m_segments[i];
		if (segment->m_data)
			ioPackage_Free(p, i, ioPackageAllocType_SegmentData, segment->m_data, segment->m_size);
	}

	const ueSize packageSize = sizeof(ioPackage) + p->m_commonDataSize + p->m_decompressionBufferSize;
	ioPackage_Free(p, NULL, ioPackageAllocType_PackageData, p, packageSize);
}

// Segments

ueBool ioPackage_LoadAllSegments(ioPackage* p)
{
	for (u32 i = 0; i < p->m_numSegments; i++)
		if (!ioPackage_LoadSegment(p, i))
			return UE_FALSE;
	return UE_TRUE;
}

ueBool ioPackage_LoadSegment(ioPackage* p, u32 index)
{
	UE_ASSERT(p->m_loadParams.m_file);
	UE_ASSERT(index < p->m_numSegments);

	ioPackageSegment* segment = &p->m_segments[index];
	if (segment->m_data)
		return UE_TRUE; // Already loaded

	// Seek to segment's start

	if (!ioFile_Seek(p->m_loadParams.m_file, ioFileSeekType_Set, segment->m_fileOffset))
	{
		ueLogE("Failed to load package segment (path = '%s'; segment index = %u), reason: failed to seek to %u", p->m_loadParams.m_path, index, segment->m_fileOffset);
		return UE_FALSE;
	}

	// Allocate memory

	segment->m_data = ioPackage_Alloc(p, index, ioPackageAllocType_SegmentData, segment->m_size, segment->m_alignment);
	if (!segment->m_data)
	{
		ueLogE("Failed to load package segment (path = '%s'; segment index = %u), reason: failed to allocate %u bytes", p->m_loadParams.m_path, index, segment->m_size);
		return UE_FALSE;
	}

	// Load and decompress the data

	if (segment->m_compressionLib == UT_NONE_COMPRESSION_SYMBOL)
	{
		if (ioFile_Read(p->m_loadParams.m_file, segment->m_data, segment->m_size, 1) != 1)
		{
			ueLogE("Failed to load package segment (path = '%s'; segment index = %u), reason: failed to read %u bytes (uncompressed data)", p->m_loadParams.m_path, index, segment->m_size);
			ioPackage_Free(p, index, ioPackageAllocType_SegmentData, segment->m_data, segment->m_alignment);
			segment->m_data = NULL;
			return UE_FALSE;
		}
	}
	else
		for (u32 i = 0; i < segment->m_numChunks; i++)
		{
			const u32 chunkSize = ueMin(segment->m_chunkSize, segment->m_size - segment->m_chunkSize * i);

			if (ioFile_Read(p->m_loadParams.m_file, p->m_decompressionBuffer, segment->m_compressedChunkSizes[i], 1) != 1)
			{
				ueLogE("Failed to load package segment (path = '%s'; segment index = %u), reason: failed to read compressed chunk (index = %u) of %u bytes", p->m_loadParams.m_path, index, i, segment->m_compressedChunkSizes[i]);
				ioPackage_Free(p, index, ioPackageAllocType_SegmentData, segment->m_data, segment->m_alignment);
				segment->m_data = NULL;
				return UE_FALSE;
			}
			UE_ASSERT_FUNC(utCompression_Decompress(segment->m_compressionLib, (u8*) segment->m_data + segment->m_chunkSize * i, chunkSize, p->m_decompressionBuffer, segment->m_compressedChunkSizes[i]));
		}

	// Read & apply patches

	const u32 patchesSize = (segment->m_numPtrPatches + segment->m_numStrPatches) * sizeof(ioPackageFile_Patch) + segment->m_sizeResPatches;
	if (patchesSize > 0)
	{
		if (ioFile_Read(p->m_loadParams.m_file, p->m_decompressionBuffer, patchesSize, 1) != 1)
		{
			ueLogE("Failed to load package segment (path = '%s'; segment index = %u), reason: failed to read patches of %u bytes", p->m_loadParams.m_path, index, patchesSize);
			ioPackage_Free(p, index, ioPackageAllocType_SegmentData, segment->m_data, segment->m_alignment);
			segment->m_data = NULL;
			return UE_FALSE;
		}

		ioPackageFile_Patch* ptrPatches = (ioPackageFile_Patch*) p->m_decompressionBuffer;
		ioPackageFile_Patch* strPatches = ptrPatches + segment->m_numPtrPatches;
		ioPackageFile_ResourcePatch* resPatches = (ioPackageFile_ResourcePatch*) (strPatches + segment->m_numStrPatches);

		// Pointer patches

		for (u32 i = 0; i < segment->m_numPtrPatches; i++)
		{
			const ioPackageFile_Patch* patch = &ptrPatches[i];

			const u32 srcOffset = patch->m_srcOffset;
			const u32 dstOffset = *((u32*) ((u8*) segment->m_data + srcOffset));

			*((void**) ((u8*) segment->m_data + srcOffset)) = (void*) ((u8*) segment->m_data + dstOffset);
		}

		// String patches

		for (u32 i = 0; i < segment->m_numStrPatches; i++)
		{
			const ioPackageFile_Patch* patch = &strPatches[i];

			const u32 srcOffset = patch->m_srcOffset;
			const u32 dstOffset = *((u32*) ((u8*) segment->m_data + srcOffset));
			char* dstString = p->m_stringPool + dstOffset;

			*((char**) ((u8*) segment->m_data + srcOffset)) = dstString;
		}

		// Resource patches

		const ioPackageFile_ResourcePatch* resPatch = resPatches;
		while ((ueSize) resPatch - (ueSize) resPatches < segment->m_sizeResPatches)
		{
			const u32 srcOffset = resPatch->m_srcOffset;

			ueResource** resHandle = (ueResource**) ((u8*) segment->m_data + srcOffset);
			const char* resName = resPatch->GetName();
			if (resName && resName[0])
				*resHandle = ueResourceMgr_Get(resPatch->m_typeId, resName, UE_TRUE);
			else
				*resHandle = NULL;

			resPatch = resPatch->Next();
		}
	}

	return UE_TRUE;
}

void* ioPackage_AcquireSegmentData(ioPackage* p, u32 index)
{
	UE_ASSERT(index < p->m_numSegments);

	ioPackageSegment* segment = &p->m_segments[index];
	UE_ASSERT(segment->m_data);

	void* data = segment->m_data;
	segment->m_data = NULL;
	return data;
}

void ioPackage_UnloadSegment(ioPackage* p, u32 index)
{
	UE_ASSERT(index < p->m_numSegments);

	ioPackageSegment* segment = &p->m_segments[index];
	if (segment->m_data)
	{
		ioPackage_Free(p, index, ioPackageAllocType_SegmentData, segment->m_data, segment->m_alignment);
		segment->m_data = NULL;
	}
}

void ioPackage_UnloadAllSegments(ioPackage* p)
{
	for (u32 i = 0; i < p->m_numSegments; i++)
		ioPackage_UnloadSegment(p, i);
}

u32 ioPackage_GetNumSegments(ioPackage* p)
{
	return p->m_numSegments;
}

u32 ioPackage_GetSegmentSymbol(ioPackage* p, u32 index)
{
	UE_ASSERT(index < p->m_numSegments);
	return p->m_segments[index].m_symbol;
}

void ioPackage_GetSegmentData(ioPackage* p, u32 index, void** data, u32* size)
{
	UE_ASSERT(index < p->m_numSegments);
	const ioPackageSegment& segment = p->m_segments[index];
	if (data)
		*data = segment.m_data;
	if (size)
		*size = segment.m_size;
}

const void* ioPackage_GetEntryData(ioPackage* p, u32 segmentIndex, u32 entryIndex)
{
	UE_ASSERT(segmentIndex < p->m_numSegments);
	const ioPackageSegment* segment = &p->m_segments[segmentIndex];
	UE_ASSERT(entryIndex < segment->m_numEntries);
	return (const u8*) segment->m_data + segment->m_entries[entryIndex].m_offset;
}

void ioPackage_SetUserData(ioPackage* package, void* userData)
{
	package->m_userData = userData;
}

void* ioPackage_GetUserData(ioPackage* package)
{
	return package->m_userData;
}

// Package segment as a file

struct ioFile_PackageSegment : ioFile
{
	ueAllocator* m_allocator;

	ioPackage* m_package;
	const ioPackageSegment* m_segment;

	u32 m_bufferedChunkIndex;
	u32 m_bufferSize;
	void* m_buffer;

	u32 m_compressedBufferSize;
	void* m_compressedBuffer;

	u32 m_offset;
};

struct ioFileSys_PackageSegment : ioFileSys
{
	ioFileSys_PackageSegment();
};

static ioFileSys_PackageSegment static_ioFileSys_PackageSegment;

ioFile* ioPackage_OpenSegmentAsFile(ueAllocator* allocator, ioPackage* package, u32 segmentIndex)
{
	UE_ASSERT(segmentIndex < package->m_numSegments);
	const ioPackageSegment* segment = &package->m_segments[segmentIndex];

	ioFile_PackageSegment* file = NULL;
	if (segment->m_compressionLib == UT_NONE_COMPRESSION_SYMBOL)
	{
		file = new(allocator) ioFile_PackageSegment();
		if (!file)
			return NULL;
		file->m_compressedBufferSize = 0;
		file->m_compressedBuffer = NULL;
		file->m_bufferSize = 0;
		file->m_buffer = NULL;
	}
	else
	{
		u32 compressedBufferSize = segment->m_compressedChunkSizes[0];
		for (u32 i = 1; i < segment->m_numChunks; i++)
			compressedBufferSize = ueMax(compressedBufferSize, segment->m_compressedChunkSizes[i]);

		const u32 maxChunkSize = segment->m_numChunks >= 2 ? segment->m_chunkSize : segment->m_size;

		u8* memory = (u8*) allocator->Alloc(sizeof(ioFile_PackageSegment) + maxChunkSize + compressedBufferSize);
		if (!memory)
			return NULL;

		file = new(memory) ioFile_PackageSegment();
		memory += sizeof(ioFile_PackageSegment);

		file->m_compressedBufferSize = compressedBufferSize;
		file->m_compressedBuffer = memory;
		memory += compressedBufferSize;

		file->m_bufferSize = maxChunkSize;
		file->m_buffer = memory;
	}

	file->m_allocator = allocator;

	file->m_package = package;
	file->m_segment = segment;

	file->m_fs = &static_ioFileSys_PackageSegment;

	file->m_offset = 0;
	file->m_bufferedChunkIndex = U32_MAX;

	return file;
}

void ioFile_PackageSegment_Close(ioFile_PackageSegment* r)
{
	r->m_allocator->Free(r);
}

ueSize ioFile_PackageSegment_Read(ioFile_PackageSegment* r, void* data, ueSize size, ueSize count, ueAsync** async)
{
	ioFile* packageFile = r->m_package->m_loadParams.m_file;

	if (async)
	{
		UE_ASSERT_MSGP(packageFile->m_fs->m_jobSys, "Parent file system (mount pou32 = '%s') is required to have job system attached - see ioFileSys_SetJobSystem()", packageFile->m_fs->m_mountPath);
		*async = ioFile_CreateGenericAsyncOperation(packageFile->m_fs->m_allocator, packageFile->m_fs->m_jobSys, r, data, size, count);
		return 0;
	}

	u32 numElementsRead = 0;

	// Uncompressed (unbuffered) case

	if (!r->m_compressedBufferSize)
	{
		// Don't read past segment within package

		if (r->m_offset == r->m_segment->m_size)
			return 0;
		UE_ASSERT(r->m_offset <= r->m_segment->m_size);

		// Clamp elements count

		const ueSize sizeLeft = r->m_segment->m_size - r->m_offset;
		count = ueMin(count, sizeLeft / size);

		// Seek and read

		if (!ioFile_Seek(packageFile, ioFileSeekType_Set, r->m_segment->m_fileOffset + r->m_offset))
			return 0;
		const ueSize numElementsRead = ioFile_Read(packageFile, data, size, count);
		r->m_offset += numElementsRead * size;
		return numElementsRead;
	}

	// Compressed (buffered) case

	const u32 startOffset = r->m_offset;
	const u32 startChunkIndex = r->m_offset / r->m_segment->m_chunkSize;

	const u32 endOffset = r->m_offset + size * count;
	const u32 endChunkIndex = ueMin(r->m_segment->m_numChunks, (endOffset + r->m_segment->m_chunkSize - 1) / r->m_segment->m_chunkSize);

	const u32 sizeToRead = (u32) size * count;

	u32 dstOffset = 0;
	u8* dst = (u8*) data;

	// Process all chunks that "touch" data to be read

	for (u32 chunkIndex = startChunkIndex; chunkIndex < endChunkIndex; chunkIndex++)
	{
		const u32 chunkOffset = chunkIndex * r->m_segment->m_chunkSize;
		const u32 chunkSize = ueMin(r->m_segment->m_chunkSize, r->m_segment->m_size - chunkOffset);

		// Figure out where and how much to copy

		const u32 copyOffset = ueMax(chunkOffset, startOffset) - chunkOffset;
		const u32 copySize = ueMin(sizeToRead - dstOffset, chunkSize - copyOffset);
		const ueBool copyWholeChunk = chunkSize == copySize;

		// Load & decompress chunk if needed

		if (r->m_bufferedChunkIndex != chunkIndex)
		{
			if (!ioFile_Seek(packageFile, ioFileSeekType_Set, r->m_segment->m_fileOffset + r->m_segment->m_compressedChunkOffsets[chunkIndex]))
				break;
			const u32 compressedChunkSize = r->m_segment->m_compressedChunkSizes[chunkIndex];
			if (ioFile_Read(packageFile, r->m_compressedBuffer, compressedChunkSize, 1) != 1)
				break;
			if (!utCompression_Decompress(r->m_segment->m_compressionLib, copyWholeChunk ? dst : r->m_buffer, chunkSize, r->m_compressedBuffer, compressedChunkSize))
				break;
			if (!copyWholeChunk)
				r->m_bufferedChunkIndex = chunkIndex;
		}

		// Copy data from chunk u32o dst buffer

		if (!copyWholeChunk)
			ueMemCpy(dst, (u8*) r->m_buffer + copyOffset, (ueSize) copySize);
		dst += copySize;
		dstOffset += copySize;
	}

	numElementsRead = dstOffset / size;
	r->m_offset += numElementsRead * size;

	return numElementsRead;
}

ueBool ioFile_PackageSegment_Seek(ioFile_PackageSegment* r, ioFileSeekType seekType, ueSize offset)
{
	switch (seekType)
	{
		case ioFileSeekType_Set:
			if (offset >= r->m_segment->m_size) return UE_FALSE;
			r->m_offset = (u32) offset;
			return UE_TRUE;

		case ioFileSeekType_End:
			r->m_offset = r->m_segment->m_size;
			return UE_TRUE;

		case ioFileSeekType_Offset:
		{
			const ueSize newOffset = (ueSize) r->m_offset + offset;
			if (newOffset < 0 || newOffset >= r->m_segment->m_size) return UE_FALSE;
			r->m_offset = (u32) newOffset;
			return UE_TRUE;
		}
	}
	return UE_FALSE;
}

ueSize ioFile_PackageSegment_GetOffset(ioFile_PackageSegment* r)
{
	return (ueSize) r->m_offset;
}

ioFileSys_PackageSegment::ioFileSys_PackageSegment()
{
	m_close = (ioFile_CloseFunc) ioFile_PackageSegment_Close;
	m_read = (ioFile_ReadFunc) ioFile_PackageSegment_Read;
	m_seek = (ioFile_SeekFunc) ioFile_PackageSegment_Seek;
	m_getOffset = (ioFile_GetOffsetFunc) ioFile_PackageSegment_GetOffset;
}

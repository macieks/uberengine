#ifndef IO_PACKAGE_FORMAT_H
#define IO_PACKAGE_FORMAT_H

struct ioPackageFile_Header
{
	static const u32 MAGIC_ID = 0xABCD1234;
	static const u32 VERSION = 1;

	u32 m_magicId;
	u32 m_endianess;
	u32 m_version;
	u32 m_commonDataSize;
	u32 m_stringPoolSize;
	u32 m_numSegments;
	u32 m_decompressionBufferSize;
};

struct ioPackageFile_Patch
{
	u32 m_srcOffset;
};

struct ioPackageFile_ResourcePatch
{
	u32 m_srcOffset;
	u32 m_typeId;

	const char* GetName() const { return (char*) (this + 1); }

	const ioPackageFile_ResourcePatch* Next() const
	{
		const char* name = GetName();
		const u32 nameLength = ueStrLen(name);
		return (ioPackageFile_ResourcePatch*) (name + nameLength + 1);
	}
};

//! Entry within package segment
struct ioPackageEntry
{
	u32 m_symbol;
	u32 m_userFlags;
	u32 m_alignment;

	u32 m_offset;
	u32 m_size;
};

//! Segment within package
struct ioPackageSegment
{
	u32 m_symbol;
	u32 m_userFlags;
	u32 m_alignment;

	u32 m_fileOffset;

	u32 m_compressionLib;
	u32 m_compressedSize;

	u32 m_chunkSize;
	u32 m_numChunks;
	u32* m_compressedChunkOffsets;
	u32* m_compressedChunkSizes;

	u32 m_size;
	void* m_data;

	u32 m_numEntries;
	ioPackageEntry* m_entries;

	u32 m_numPtrPatches;
	u32 m_numStrPatches;
	u32 m_sizeResPatches;
};

#endif // IO_PACKAGE_FORMAT_H

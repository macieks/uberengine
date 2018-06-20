#include "IO/ioPackageWriter.h"
#include "IO/ioPackage.h"
#include "IO/ioPackage_Format.h"

// ioPackageConfig

void ioPackageConfig::SetForPlatform(uePlatform platform)
{
	m_endianess = uePlatformEndianess(platform);

	switch (platform)
	{
		case uePlatform_WII:
		case uePlatform_Linux:
		case uePlatform_Mac:
		case uePlatform_Marmalade:
			m_ptrSize = 4;
			m_defaultPtrAlignment = 4;
			m_boolSize = 1;
			m_alignVecsTo16Bytes = false;
			m_defaultAlignment = 4;
			break;
		case uePlatform_Win32:
			m_ptrSize = 4;
			m_defaultPtrAlignment = 4;
			m_boolSize = 1;
			m_alignVecsTo16Bytes = false;
			m_defaultAlignment = 4;
			break;
		case uePlatform_PS3:
			m_ptrSize = 4;
			m_defaultPtrAlignment = 16; // No need to think if there's any vector4s
			m_boolSize = 1;
			m_alignVecsTo16Bytes = false;
			m_defaultAlignment = 4;
			break;
		case uePlatform_X360:
			m_ptrSize = 4;
			m_defaultPtrAlignment = 16; // No need to think if there's any vector4s
			m_boolSize = 4; // 4-byte BOOL better than 1-byte bool on X360
			m_alignVecsTo16Bytes = false;
			m_defaultAlignment = 4;
			break;
		UE_INVALID_CASE(platform)
	}
}

// ioPackageWriter::Segment

void ioPackageWriter::Segment::Compress()
{
	if (!m_chunkSize)
		m_chunkSize = (u32) m_data.size();
	vector_u8_compress_chunked(m_compressionLib, m_data, m_compressedData, m_chunkSize, m_chunkOffsets, m_chunkSizes);
}

bool ioPackageWriter::Segment::IsDataCompressed() const
{
	return m_compressionLib != UT_NONE_COMPRESSION_SYMBOL;
}

u32 ioPackageWriter::Segment::CalcDecompressionBufferSize() const
{
	u32 size = 0;

	if (m_compressionLib != UT_NONE_COMPRESSION_SYMBOL)
		for (u32 i = 0; i < m_chunkSizes.size(); i++)
			size = ueMax(size, m_chunkSizes[i]);

	return (u32) size;
}

u32 ioPackageWriter::Segment::GetFileSize() const
{
	return (u32) ((m_compressionLib == UT_NONE_COMPRESSION_SYMBOL) ? m_data.size() : m_compressedData.size());
}

u32 ioPackageWriter::Segment::CalcResPatchesSize() const
{
	u32 size = 0;
	for (u32 i = 0; i < m_resPatches.size(); i++)
	{
		const ResourcePatch& patch = m_resPatches[i];
		size += sizeof(u32); // Src offset
		size += sizeof(u32); // Type id
		size += (u32) patch.m_name.length() + 1;
	}
	return size;
}

ioPackageWriter::ioPackageWriter()
{}

void ioPackageWriter::Reset(const ioPackageConfig& config)
{
	m_config = config;
	m_segments.clear();
}

void ioPackageWriter::ResetForPlatform(uePlatform platform)
{
	ioPackageConfig writerConfig;
	writerConfig.SetForPlatform(platform);
	Reset(writerConfig);
}

// Saving

#define WRITE(buffer, size) { if (f.Write(buffer, size, 1) != 1) return UE_FALSE; }
#define WRITE_STRUCT(s) WRITE(&s, sizeof(s))
#define WRITE_VEC(v) { if (v.size() > 0) WRITE(&v[0], v.size() * sizeof(v[0])) }
#define WRITE_VEC_SUB(v, start, count) { WRITE(&v[start], count * sizeof(v[0])) }

ueBool ioPackageWriter::SaveToFile(const char* path)
{
	ioFilePtr f;
	if (!f.Open(path, ioFileOpenFlags_Write | ioFileOpenFlags_Create))
		return UE_FALSE;

	if (!m_config.m_silentMode)
		fprintf(stdout, "Building string pool...\n");

	// Prepare string pool

	std::vector<u32> stringOffsets(m_stringPool.size());
	std::vector<char> strings;
	for (std::map<std::string, u32>::iterator i = m_stringPool.begin(); i != m_stringPool.end(); i++)
	{
		const std::string& s = i->first;

		const u32 oldLength = (u32) strings.size();
		strings.resize(oldLength + (u32) s.length() + 1);
		memcpy(&strings[oldLength], s.c_str(), s.length() + 1);

		stringOffsets[i->second] = oldLength;
	}

	// Align string pool size so that the segments start at aligned address

	while (strings.size() % m_config.m_defaultAlignment)
		strings.push_back('X');

	// Set up pointers

	for (u32 i = 0; i < m_segments.size(); i++)
	{
		Segment& srcSegment = m_segments[i];

		UE_ASSERT(srcSegment.m_isDone);

		for (u32 j = 0; j < srcSegment.m_patches.size(); j++)
		{
			const PtrPatch& srcPatch = srcSegment.m_patches[j];
			const PtrInfo& srcPtr = srcSegment.m_ptrs[srcPatch.m_ptr.m_index];

			*((u32*) &srcSegment.m_data[srcPatch.m_srcOffset]) = srcPtr.m_dstOffset;
		}

		for (u32 j = 0; j < srcSegment.m_strPatches.size(); j++)
		{
			const StrPatch& srcPatch = srcSegment.m_strPatches[j];

			*((u32*) &srcSegment.m_data[srcPatch.m_srcOffset]) = stringOffsets[srcPatch.m_strIndex];
		}
	}

	if (!m_config.m_silentMode)
		fprintf(stdout, "Compressing...\n");

	// Compress all data (determine if it's worth to compress it)

	for (u32 i = 0; i < m_segments.size(); i++)
	{
		if (!m_config.m_silentMode)
			fprintf(stdout, "  Segment %d [%u bytes]...\n", i, (u32) m_segments[i].m_data.size());
		m_segments[i].Compress();
	}

	// Calculate decompression buffer size

	u32 decompressionBufferSize = 0;
	for (u32 i = 0; i < m_segments.size(); i++)
	{
		const Segment& srcSegment = m_segments[i];

		decompressionBufferSize = ueMax(decompressionBufferSize, srcSegment.CalcDecompressionBufferSize());

		const u32 patchesSize =
			(u32) srcSegment.m_patches.size() * sizeof(ioPackageFile_Patch) +
			(u32) srcSegment.m_strPatches.size() * sizeof(ioPackageFile_Patch) +
			srcSegment.CalcResPatchesSize();
		decompressionBufferSize = ueMax(decompressionBufferSize, patchesSize);
	}

	// Calculate common data size

	u32 commonDataSize =
		(u32) strings.size() +
		(u32) m_segments.size() * sizeof(ioPackageSegment);
	for (u32 i = 0; i < m_segments.size(); i++)
	{
		const Segment& srcSegment = m_segments[i];
		commonDataSize += (u32) srcSegment.m_entries.size() * sizeof(ioPackageEntry);
		commonDataSize += (u32) srcSegment.m_chunkSizes.size() * sizeof(u32) * 2;
	}

	if (!m_config.m_silentMode)
		fprintf(stdout, "Writing...\n");

	// Write header

	ioPackageFile_Header header;
	header.m_magicId = ueToEndianess(ioPackageFile_Header::MAGIC_ID, UE_ENDIANESS_LITTLE);
	header.m_endianess = m_config.m_endianess;
	header.m_version = ueToEndianess(ioPackageFile_Header::VERSION, m_config.m_endianess);
	header.m_commonDataSize = ueToEndianess(commonDataSize, m_config.m_endianess);
	header.m_stringPoolSize = ueToEndianess((u32) strings.size(), m_config.m_endianess);
	header.m_numSegments = ueToEndianess((u32) m_segments.size(), m_config.m_endianess);
	header.m_decompressionBufferSize = ueToEndianess(decompressionBufferSize, m_config.m_endianess);
	WRITE_STRUCT(header);

	// Write string pool

	WRITE_VEC(strings);

	// Calculate starting data size

	u32 dataOffset = sizeof(ioPackageFile_Header) + commonDataSize;

	// Write segments (just the info)

	for (u32 i = 0; i < m_segments.size(); i++)
	{
		const Segment& srcSegment = m_segments[i];

		ioPackageSegment dstSegment;

		dstSegment.m_symbol = ueToEndianess(srcSegment.m_symbol, m_config.m_endianess);
		dstSegment.m_userFlags = ueToEndianess(srcSegment.m_userFlags, m_config.m_endianess);
		dstSegment.m_alignment = ueToEndianess(srcSegment.m_alignment, m_config.m_endianess);

		dstSegment.m_fileOffset = ueToEndianess(dataOffset, m_config.m_endianess);

		dstSegment.m_compressionLib = ueToEndianess(srcSegment.m_compressionLib, m_config.m_endianess);
		dstSegment.m_compressedSize = ueToEndianess((u32) srcSegment.m_compressedData.size(), m_config.m_endianess);
		dstSegment.m_chunkSize = ueToEndianess(srcSegment.m_chunkSize, m_config.m_endianess);

		dstSegment.m_numChunks = ueToEndianess((u32) srcSegment.m_chunkSizes.size(), m_config.m_endianess);
		dstSegment.m_compressedChunkOffsets = NULL;
		dstSegment.m_compressedChunkSizes = NULL;

		dstSegment.m_size = ueToEndianess((u32) srcSegment.m_data.size(), m_config.m_endianess);
		dstSegment.m_data = NULL;

		dstSegment.m_numEntries = ueToEndianess((u32) srcSegment.m_entries.size(), m_config.m_endianess);
		dstSegment.m_entries = NULL;

		dstSegment.m_numPtrPatches = ueToEndianess((u32) srcSegment.m_patches.size(), m_config.m_endianess);
		dstSegment.m_numStrPatches = ueToEndianess((u32) srcSegment.m_strPatches.size(), m_config.m_endianess);
		dstSegment.m_sizeResPatches = ueToEndianess((u32) srcSegment.CalcResPatchesSize(), m_config.m_endianess);

		//ueLogD("Segment %u start is %u", i, dstSegment.m_fileOffset);

		WRITE_STRUCT(dstSegment);

		// Update data offset
		dataOffset +=
			srcSegment.GetFileSize() +
			srcSegment.m_patches.size() * sizeof(ioPackageFile_Patch) +
			srcSegment.m_strPatches.size() * sizeof(ioPackageFile_Patch) +
			srcSegment.CalcResPatchesSize();
	}

	// Write entry infos and chunk sizes and offsets

	for (u32 i = 0; i < m_segments.size(); i++)
	{
		const Segment& srcSegment = m_segments[i];

		for (u32 j = 0; j < srcSegment.m_entries.size(); j++)
		{
			const Entry& srcEntry = srcSegment.m_entries[j];

			ioPackageEntry dstEntry;

			dstEntry.m_symbol = ueToEndianess(srcEntry.m_symbol, m_config.m_endianess);
			dstEntry.m_userFlags = ueToEndianess(srcEntry.m_userFlags, m_config.m_endianess);
			dstEntry.m_alignment = ueToEndianess(srcEntry.m_alignment, m_config.m_endianess);

			dstEntry.m_offset = ueToEndianess(srcEntry.m_offset, m_config.m_endianess);
			dstEntry.m_size = ueToEndianess(srcEntry.m_size, m_config.m_endianess);

			WRITE_STRUCT(dstEntry);
		}

		for (u32 j = 0; j < srcSegment.m_chunkOffsets.size(); j++)
		{
			const u32 offset = ueToEndianess(srcSegment.m_chunkOffsets[j], m_config.m_endianess);
			WRITE_STRUCT(offset);
		}
		for (u32 j = 0; j < srcSegment.m_chunkSizes.size(); j++)
		{
			const u32 size = ueToEndianess(srcSegment.m_chunkSizes[j], m_config.m_endianess);
			WRITE_STRUCT(size);
		}
	}

	// Write segments patches & data

	for (u32 i = 0; i < m_segments.size(); i++)
	{
		const Segment& srcSegment = m_segments[i];

		// Write data

		//ueLogD("Segment %u starts at offset %u", i, f.GetOffset());

		if (srcSegment.IsDataCompressed())
			WRITE_VEC(srcSegment.m_compressedData)
		else
			WRITE_VEC(srcSegment.m_data);

		// Write patches

		for (u32 j = 0; j < srcSegment.m_patches.size(); j++)
		{
			const PtrPatch& srcPatch = srcSegment.m_patches[j];

			ioPackageFile_Patch dstPatch;
			dstPatch.m_srcOffset = ueToEndianess(srcPatch.m_srcOffset, m_config.m_endianess);

			WRITE_STRUCT(dstPatch);
		}

		for (u32 j = 0; j < srcSegment.m_strPatches.size(); j++)
		{
			const StrPatch& srcPatch = srcSegment.m_strPatches[j];

			ioPackageFile_Patch dstPatch;
			dstPatch.m_srcOffset = ueToEndianess(srcPatch.m_srcOffset, m_config.m_endianess);

			WRITE_STRUCT(dstPatch);
		}

		for (u32 j = 0; j < srcSegment.m_resPatches.size(); j++)
		{
			const ResourcePatch& patch = srcSegment.m_resPatches[j];

			const u32 srcOffset = ueToEndianess(patch.m_srcOffset, m_config.m_endianess);
			const u32 typeId = ueToEndianess(patch.m_typeId, m_config.m_endianess);

			WRITE(&srcOffset, sizeof(u32));
			WRITE(&typeId, sizeof(u32));
			WRITE(patch.m_name.c_str(), patch.m_name.length() + 1);
		}
	}

#undef WRITE
#undef WRITE_STRUCT
#undef WRITE_VEC

	return true;
}

// Segment writer

ioSegmentWriter::ioSegmentWriter() :
	m_pw(NULL),
	m_segmentIndex(0),
	m_isEntryStarted(false)
{}

ioSegmentWriter::ioSegmentWriter(ioSegmentWriter& other) :
	m_pw(other.m_pw),
	m_segmentIndex(other.m_segmentIndex),
	m_isEntryStarted(other.m_isEntryStarted),
	m_isCreatePtrFromWritePtr(UE_FALSE)
{}

ioSegmentWriter::~ioSegmentWriter()
{
	if (!m_pw)
		return;

	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	if (!segment->m_isDone)
		EndSegment();
}

u32 ioPackageWriter::BeginSegment(ioSegmentWriter& segmentWriter, const ioSegmentParams& params)
{
	Segment& newSegment = vector_push(m_segments);
	newSegment.m_alignment = params.m_alignment;
	if (!newSegment.m_alignment)
		newSegment.m_alignment = m_config.m_defaultPtrAlignment;
	newSegment.m_userFlags = params.m_userFlags;
	newSegment.m_symbol = params.m_symbol;
	newSegment.m_compressionLib = params.m_useDefaultCompressionLib ? m_config.m_compressionLib : params.m_compressionLib;
	newSegment.m_chunkSize = params.m_chunkSize;
	newSegment.m_isDone = false;

	segmentWriter.m_pw = this;
	segmentWriter.m_segmentIndex = (u32) m_segments.size() - 1;

	return segmentWriter.m_segmentIndex;
}

void ioSegmentWriter::EndSegment()
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];

	UE_ASSERT(!segment->m_isDone);
	UE_ASSERT(!m_isEntryStarted);

#if defined(UE_ENABLE_ASSERTION)
	for (u32 i = 0; i < segment->m_ptrs.size(); i++)
	{
		const ioPackageWriter::PtrInfo& ptrInfo = segment->m_ptrs[i];

		if (ptrInfo.m_isWritten)
			continue;

		const char* fileName = NULL;
		u32 fileLine = 0;
		ueCallstackHelper_GetFrameDetails(ptrInfo.m_debugFramePtr, fileName, fileLine);
		UE_ASSERT_MSGP(0, "Pointer created but not written; callstack indicates that pointer was created at:\n%s(%u)", fileName, fileLine);
	}
#endif

	// Write dest offset to src offset (to save memory)
	for (u32 i = 0; i < segment->m_patches.size(); i++)
	{
		const ioPackageWriter::PtrPatch& patch = segment->m_patches[i];
		const ioPackageWriter::PtrInfo& ptrInfo = segment->m_ptrs[patch.m_ptr.m_index];

		std::vector<u8>& data = segment->m_data;
		*(u32*) &data[patch.m_srcOffset] = ueToEndianess(ptrInfo.m_dstOffset, GetConfig().m_endianess);
	}

	segment->m_isDone = true;

	m_pw = NULL;
	m_segmentIndex = -1;
}

void ioSegmentWriter::BeginEntry(const ioEntryParams& params)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!m_isEntryStarted);

	ioPackageWriter::Entry& newEntry = vector_push(segment->m_entries);
	newEntry.m_alignment = params.m_alignment;
	if (!newEntry.m_alignment)
		newEntry.m_alignment = segment->m_alignment;
	newEntry.m_userFlags = params.m_userFlags;
	newEntry.m_symbol = params.m_symbol;
	WriteAlign(newEntry.m_alignment); // Start entry at aligned offset within segment
	newEntry.m_offset = (u32) segment->m_data.size();

	m_isEntryStarted = true;
}

void ioSegmentWriter::EndEntry()
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(m_isEntryStarted);
	ioPackageWriter::Entry* entry = &segment->m_entries.back();

	entry->m_size = (u32) segment->m_data.size() - entry->m_offset;
	m_isEntryStarted = false;
}

// Pointers

ioPtr ioSegmentWriter::CreatePtr()
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	ioPtr ptr;
	ptr.m_index = (u32) segment->m_ptrs.size();

	ioPackageWriter::PtrInfo ptrInfo;
	ptrInfo.m_dstOffset = 0;
	ptrInfo.m_refCount = 0;
	ptrInfo.m_isWritten = UE_FALSE;
#if defined(UE_ENABLE_ASSERTION)
	u32 isCallstackRetrieved = 1;
	if (!ueCallstackHelper_GetCallstack(&ptrInfo.m_debugFramePtr, isCallstackRetrieved, m_isCreatePtrFromWritePtr ? 3 : 2) || !isCallstackRetrieved)
		ptrInfo.m_debugFramePtr = NULL;
#endif
	segment->m_ptrs.push_back(ptrInfo);

	return ptr;
}

ioPtr ioSegmentWriter::WritePtr()
{
	m_isCreatePtrFromWritePtr = UE_TRUE;
	ioPtr ptr = CreatePtr();
	m_isCreatePtrFromWritePtr = UE_FALSE;
	WritePtr(ptr);
	return ptr;
}

ioPtr ioSegmentWriter::WriteAndBeginPtr(u32 alignment)
{
	m_isCreatePtrFromWritePtr = UE_TRUE;
	ioPtr ptr = CreatePtr();
	m_isCreatePtrFromWritePtr = UE_FALSE;
	WritePtr(ptr);

	BeginPtr(ptr, alignment);

	return ptr;
}

void ioSegmentWriter::WritePtr(ioPtr ptr)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	ioPackageWriter::PtrInfo& ptrInfo = segment->m_ptrs[ptr.m_index];
	ptrInfo.m_refCount++;

	ioPackageWriter::PtrPatch patch;
	patch.m_ptr = ptr;
	WriteAlignPtr(); // Write at aligned offset
	patch.m_srcOffset = (u32) GetData()->size();
	segment->m_patches.push_back(patch);

	// Write null-ptr for now; will be overwritten with dest offset when segment is finalized

	WriteNullPtr();
}

void ioSegmentWriter::WriteNullPtr()
{
	WriteAlignPtr();

	static u8 nullPtrData[8] = {0};
	WriteData(nullPtrData, GetConfig().m_ptrSize);
}

void ioSegmentWriter::BeginPtr(ioPtr ptr, u32 alignment)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	// Align data pointed

	if (!alignment)
		alignment = GetConfig().m_defaultPtrAlignment;
	WriteAlign(alignment);

	// Update pointer entry

	ioPackageWriter::PtrInfo& ptrInfo = segment->m_ptrs[ptr.m_index];

#if defined(UE_ENABLE_ASSERTION)
	if (ptrInfo.m_isWritten)
	{
		const char* fileName = NULL;
		u32 fileLine = 0;
		ueCallstackHelper_GetFrameDetails(ptrInfo.m_debugFramePtr, fileName, fileLine);
		UE_ASSERT_MSGP(0, "Pointer already written; callstack indicates that pointer was created at:\n%s(%u)", fileName, fileLine);
	}
#endif

	ptrInfo.m_isWritten = UE_TRUE;
	ptrInfo.m_dstOffset = (u32) segment->m_data.size();
}

// Resources

void ioSegmentWriter::WriteResourceHandle(u32 typeId, const char* name)
{
	WriteAlignPtr(); // Align first so we store aligned address

	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	ioPackageWriter::ResourcePatch& patch = vector_push(segment->m_resPatches);
	patch.m_typeId = typeId;
	patch.m_name = name;
	ueStrToLower(&patch.m_name[0]);
	patch.m_srcOffset = (u32) GetData()->size();

	WriteNullPtr();	// ueResourceHandle::m_resource
}

// Data

void ioSegmentWriter::WriteData(const void* data, ueSize size)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	std::vector<u8>* dataPtr = GetData();

	const ueSize oldSize = dataPtr->size();
	dataPtr->resize(dataPtr->size() + size);
	ueMemCpy(&(*dataPtr)[oldSize], data, size);
}

void ioSegmentWriter::WriteBool(bool value)
{
	static const u8 falseValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	static const u8 trueValue[8] = {U8_MAX, U8_MAX, U8_MAX, U8_MAX, U8_MAX, U8_MAX, U8_MAX, U8_MAX};

	WriteAlign(GetConfig().m_boolSize);
	WriteData(value ? trueValue : falseValue, GetConfig().m_boolSize);
}

void ioSegmentWriter::WritePooledString(const char* s)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	ioPackageWriter::StrPatch patch;
	WriteAlignPtr(); // Write string ptr at aligned offset
	patch.m_srcOffset = (u32) GetData()->size();
	patch.m_strIndex = m_pw->GetStringIndex(s);
	segment->m_strPatches.push_back(patch);

	WriteNullPtr(); // To be overwritten when offsets are resolved
}

void ioSegmentWriter::WriteSkip(ueSize size)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	std::vector<u8>* data = GetData();
	data->resize(data->size() + size);
}

void ioSegmentWriter::WriteAlign(ueSize alignment)
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	UE_ASSERT(!segment->m_isDone);

	std::vector<u8>* data = GetData();
	if (data->size() % alignment)
		data->resize(data->size() + alignment - (data->size() % alignment));
}

// Misc.

ueBool ioSegmentWriter::IsAligned(ueSize alignment) const
{
	return GetData()->size() % alignment == 0;
}

std::vector<u8>* ioSegmentWriter::GetData() const
{
	ioPackageWriter::Segment* segment = &m_pw->m_segments[m_segmentIndex];
	return &segment->m_data;
}

const ioPackageConfig& ioSegmentWriter::GetConfig() const
{
	return m_pw->m_config;
}

u32 ioPackageWriter::GetStringIndex(const char* s)
{
	std::map<std::string, u32>::iterator i = m_stringPool.find(s);
	if (i != m_stringPool.end())
		return i->second;

	const u32 newId = (u32) m_stringPool.size();
	m_stringPool[s] = newId;
	return newId;
}
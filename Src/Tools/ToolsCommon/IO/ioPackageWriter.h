#pragma once

#include "Base/ueToolsBase.h"
#include "Utils/utCompression.h"
#include "IO/ioFile.h"

class ioPackageWriter;

//! Package creation parameters / configuration
struct ioPackageConfig
{
	u32 m_endianess;			//!< UE_ENDIANESS_LITTLE or UE_ENDIANESS_BIG

	u32 m_ptrSize;				//!< Pointer size (4 on 32-bit platform; 8 on 64-bit platform)
	u32 m_defaultPtrAlignment;	//!< Alignment of the data pointed by pointer (ioPtr); overriddable via ioSegmentWriter::BeginPtr()
	u32 m_boolSize;				//!< ueBool size
	bool m_alignVecsTo16Bytes;	//!< Indicates whether to align ueVec2/3/4/ueQuat/smPlane/ueMat44 to 16-bytes; used when calling ioSegmentWriter::WriteAlignVec()
	u32 m_defaultAlignment;		//!< Default alignment as per compiler's settings; used when calling ioSegmentWriter::WriteAlignDefault()

	bool m_silentMode;			//!< Enables silent logging mode

	u32 m_compressionLib;		//!< Default compression library

	ioPackageConfig() :
		m_endianess(UE_ENDIANESS_LITTLE),
		m_ptrSize(4),
		m_defaultPtrAlignment(4),
		m_boolSize(1),
		m_alignVecsTo16Bytes(false),
		m_defaultAlignment(4),
		m_silentMode(true),
		m_compressionLib(UT_NONE_COMPRESSION_SYMBOL)
	{}

	void SetForPlatform(uePlatform platform);
};

//! Package's segment creation parameters
struct ioSegmentParams
{
	u32 m_symbol;			//!< Custom user symbol
	u32 m_userFlags;		//!< Custom user flags
	u32 m_alignment;		//!< 0 indicates default package pointer alignment (i.e. ioPackageConfig::m_defaultPtrAlignment)

	ueBool m_useDefaultCompressionLib; //!< Indicates whether to use default compression library (taken from ioPackageConfig)
	u32 m_compressionLib;	//!< Compression library to be used
	u32 m_chunkSize;		//!< Compression chunk size

	ioSegmentParams() :
		m_symbol(0),
		m_userFlags(0),
		m_alignment(0),
		m_useDefaultCompressionLib(UE_TRUE),
		m_compressionLib(UT_NONE_COMPRESSION_SYMBOL),
		m_chunkSize(0)
	{}
};

//! Segment's entry creation parameters
struct ioEntryParams
{
	u32 m_symbol;			//!< Custom user symbol
	u32 m_userFlags;		//!< Custom user flags
	u32 m_alignment;		//!< 0 indicates parent segment's alignment (i.e. ioSegmentParams::m_alignment)

	ioEntryParams() :
		m_symbol(0),
		m_userFlags(0),
		m_alignment(0)
	{}
};

//! Represents run-time pointer
struct ioPtr
{
	u32 m_index;
};

//! Segment data writer helper
class ioSegmentWriter
{
public:
	ioSegmentWriter();
	ioSegmentWriter(ioSegmentWriter& other);
	~ioSegmentWriter(); // Ends segment if EndSegment() wasn't called

	void EndSegment();

	void BeginEntry(const ioEntryParams& params);
	void EndEntry();

	// Pointers

	ioPtr CreatePtr();

	ioPtr WritePtr();
	void WritePtr(ioPtr ptr);
	void WriteNullPtr();

	ioPtr WriteAndBeginPtr(u32 alignment = 0);

	//! Begins writing data pointed by specified pointer; alignment specifies alignment of the data start (0 indicates parent segment / entry alignment)
	void BeginPtr(ioPtr p, u32 alignment = 0);

	// Resources

	void WriteResourceHandle(u32 typeId, const char* name);

	// Writing data

	void WriteData(const void* data, ueSize size);

	template <class NUMBER_TYPE>
	void WriteNumber(NUMBER_TYPE value)
	{
		WriteAlign(sizeof(NUMBER_TYPE));
		const NUMBER_TYPE value2 = ueToEndianess(value, GetConfig().m_endianess);
		WriteData(&value2, sizeof(NUMBER_TYPE));
	}
	void WriteBool(bool value);

	void WritePooledString(const char* s);

	void WriteSkip(ueSize size);

	void WriteAlign(ueSize alignment);
	void WriteAlignPtr() { WriteAlign(GetConfig().m_ptrSize); }
	void WriteAlignDefault() { WriteAlign(GetConfig().m_defaultAlignment); }
	void WriteAlignVec() { if (GetConfig().m_alignVecsTo16Bytes) WriteAlign(16); }
	ueBool IsAligned(ueSize alignment) const;

	const ioPackageConfig& GetConfig() const;

private:
	std::vector<u8>* GetData() const;

	ioPackageWriter* m_pw;
	u32 m_segmentIndex;
	bool m_isEntryStarted;

	bool m_isCreatePtrFromWritePtr;

	friend class ioPackageWriter;
};

//! Package writer helper
class ioPackageWriter
{
public:

	ioPackageWriter();

	void Reset(const ioPackageConfig& config);
	void ResetForPlatform(uePlatform platform);
	UE_INLINE const ioPackageConfig& GetConfig() const { return m_config; }

	//! Saves package to file
	ueBool SaveToFile(const char* path);

	//! Begins writing segment
	u32 BeginSegment(ioSegmentWriter& segmentWriter, const ioSegmentParams& params);

private:

	u32 GetStringIndex(const char* s);

	struct PtrInfo
	{
		u32 m_dstOffset;

		u32 m_refCount;
		ueBool m_isWritten;

		void* m_debugFramePtr;
	};

	struct PtrPatch
	{
		ioPtr m_ptr;
		u32 m_srcOffset;
	};

	struct StrPatch
	{
		u32 m_srcOffset;
		u32 m_strIndex;
	};

	struct ResourcePatch
	{
		u32 m_srcOffset;

		u32 m_typeId;
		std::string m_name;
	};

	struct Entry
	{
		u32 m_symbol;
		u32 m_userFlags;
		u32 m_alignment;

		u32 m_offset;
		u32 m_size;
	};

	struct Segment
	{
		u32 m_symbol;
		u32 m_userFlags;
		u32 m_alignment;

		u32 m_compressionLib;
		u32 m_chunkSize;
		std::vector<u32> m_chunkOffsets;
		std::vector<u32> m_chunkSizes;

		std::vector<Entry> m_entries;
		std::vector<PtrInfo> m_ptrs;
		std::vector<PtrPatch> m_patches;
		std::vector<StrPatch> m_strPatches;
		std::vector<ResourcePatch> m_resPatches;
		std::vector<u8> m_data;

		std::vector<u8> m_compressedData;

		bool m_isDone;

		void Compress();
		bool IsDataCompressed() const;
		u32 CalcDecompressionBufferSize() const;
		u32 GetFileSize() const;
		u32 CalcResPatchesSize() const;
	};

	// Configuration

	ioPackageConfig m_config;

	// Data

	std::vector<Segment> m_segments;
	std::map<std::string, u32> m_stringPool;

	friend class ioSegmentWriter;
};

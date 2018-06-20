#pragma once

#include "Base/ueToolsBase.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioFileSys_AFS_Types.h"
#include "Utils/utCompression.h"

class ueAFSPacker
{
public:
	struct PackParams
	{
		uePlatform m_platform;
		const char* m_inputPath;
		const char* m_outputPath;
		bool m_silentMode;

		PackParams() :
			m_platform(uePlatform_Unknown),
			m_inputPath(NULL),
			m_outputPath(NULL),
			m_silentMode(UE_TRUE)
		{}
	};

	struct PackStats
	{
		ueSize m_totalSize;
		ueSize m_totalCompressedSize;
		u32 m_numFiles;
		u32 m_numDirs;
		f32 m_compressionRatio;
	};

private:
	struct FileInfo
	{
		s32 m_parentIndex;
		u32 m_level;

		std::string m_name;
		std::vector<u32> m_dirs;
		std::vector<u32> m_files;

		bool m_isDir;
	};

	std::string m_inputPath;
	std::string m_outputPath;
	bool m_silentMode;

	u32 m_compressedChunkSize;

	std::vector<FileInfo> m_infos;
	u32 m_rootIndex;

	PackStats m_stats;

	std::string m_errorDescription;
	ioPackageWriter* m_pw;
	ioSegmentWriter m_sw;

public:
	ueAFSPacker();
	~ueAFSPacker();
	bool PackArchive(const PackParams& params, PackStats* stats = NULL);
	const char* GetErrorDescription() const { return m_errorDescription.c_str(); }

private:
	bool BuildHierarchy(const std::string& parentFullPath, u32 parentIndex, u32 level);
	void GetFullPath(const FileInfo* info, std::string& fullPath) const;

	bool WriteFileData(const FileInfo* info, ioSegmentWriter& fileSw, ueSize& size);
	bool WriteFileInfos(const std::vector<u32>& dirs, const std::vector<u32>& files, ioPtr* dirsPtr, ioPtr* filesPtr);

	void OutputInfo(const FileInfo* info) const;
};
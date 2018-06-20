#include "IO/ioPackageWriter.h"

#include "ueAFSPacker.h"

ueAFSPacker::ueAFSPacker() :
	m_pw(NULL)
{
}

ueAFSPacker::~ueAFSPacker()
{
}

bool ueAFSPacker::PackArchive(const PackParams& params, PackStats* stats)
{
	m_inputPath = params.m_inputPath;
	m_outputPath = params.m_outputPath;
	m_silentMode = params.m_silentMode;
	m_compressedChunkSize = 16384;

	m_errorDescription = "";
	m_infos.clear();

	FileInfo root;
	root.m_parentIndex = -1;
	root.m_isDir = UE_TRUE;
	root.m_name = "";
	root.m_level = 0;
	m_infos.push_back(root);
	m_rootIndex = 0;

	m_stats.m_totalSize = 0;
	m_stats.m_totalCompressedSize = 0;
	m_stats.m_numFiles = 0;
	m_stats.m_numDirs = 0;

	if (!BuildHierarchy(m_inputPath, m_rootIndex, 0))
		return UE_FALSE;
	if (m_infos[m_rootIndex].m_dirs.size() == 0 && m_infos[m_rootIndex].m_files.size() == 0)
	{
		if (!m_silentMode)
			fprintf(stderr, "ERROR: Empty root directory '%s' or directory doesn't exist.\n", m_inputPath.c_str());
		return UE_FALSE;
	}

	if (!m_silentMode)
		fprintf(stdout, "Started packing '%s' directory into '%s'...\n", params.m_inputPath, params.m_outputPath);

	ioPackageConfig writerConfig;
	writerConfig.SetForPlatform(params.m_platform);
	writerConfig.m_silentMode = params.m_silentMode;

	m_pw = new ioPackageWriter();
	m_pw->Reset(writerConfig);

	ioSegmentParams segParams;
	segParams.m_symbol = 'afsh';
	segParams.m_chunkSize = m_compressedChunkSize;
	m_pw->BeginSegment(m_sw, segParams);
	{
		std::vector<u32> rootDir;
		rootDir.push_back(m_rootIndex);
		if (!WriteFileInfos(rootDir, std::vector<u32>(), NULL, NULL)) // Serialize recursively
			goto Failure;
	}
	m_sw.EndSegment();

	if (!m_pw->SaveToFile(params.m_outputPath))
		goto Failure;
	delete m_pw;

	ioFile_GetSize(params.m_outputPath, m_stats.m_totalCompressedSize);

	m_stats.m_compressionRatio = m_stats.m_totalSize > 0 ? ((f32) m_stats.m_totalCompressedSize / (f32) m_stats.m_totalSize) : 1.0f;
	if (!m_silentMode)
		printf("DONE\n"
			"  files: %d\n"
			"  directories: %d\n"
			"  total size: %lld bytes\n"
			"  total compressed size: %lld bytes\n"
			"  compression ratio: %.2f %%\n",
			m_stats.m_numFiles,
			m_stats.m_numDirs,
			(u64) m_stats.m_totalSize,
			(u64) m_stats.m_totalCompressedSize,
			m_stats.m_compressionRatio * 100.0f);
	if (stats)
		*stats = m_stats;
	return UE_TRUE;

Failure:
	delete m_pw;
	return UE_FALSE;
}

bool ueAFSPacker::BuildHierarchy(const std::string& parentFullPath, u32 parentIndex, u32 level)
{
	const std::string searchPath = parentFullPath + m_infos[parentIndex].m_name + "/*";

	WIN32_FIND_DATAA tempFile;
	HANDLE fileHandle = FindFirstFileA(searchPath.c_str(), &tempFile); 

	m_infos[parentIndex].m_dirs.clear();
	m_infos[parentIndex].m_files.clear();
	while (fileHandle != INVALID_HANDLE_VALUE) 
	{
		// Exclude special, hidden & system files & directories

		if (strcmp(tempFile.cFileName, ".") && strcmp(tempFile.cFileName, "..") &&
			!(tempFile.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
		{
			// Directory

			if (tempFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FileInfo info;
				info.m_parentIndex = parentIndex;
				info.m_name = tempFile.cFileName;
				info.m_isDir = UE_TRUE;
				info.m_level = level;

				m_infos.push_back(info);

				m_infos[parentIndex].m_dirs.push_back((u32) m_infos.size() - 1);
				m_stats.m_numDirs++;

				// Recurse

				BuildHierarchy(parentFullPath + m_infos[parentIndex].m_name + "/", (u32) m_infos.size() - 1, level + 1);
			}

			// File

			else if (tempFile.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
			{
				FileInfo info;
				info.m_parentIndex = parentIndex;
				info.m_isDir = UE_FALSE;
				info.m_name = tempFile.cFileName;
				info.m_level = level;
				
				m_infos.push_back(info);
				m_stats.m_numFiles++;

				m_infos[parentIndex].m_files.push_back((u32) m_infos.size() - 1);
			}
		}

		// Find next file

		if (!FindNextFileA(fileHandle, &tempFile))
		{
			FindClose(fileHandle);
			break;
		}
	}

	return UE_TRUE;
}

void ueAFSPacker::GetFullPath(const FileInfo* info, std::string& fullPath) const
{
	if (info->m_parentIndex != -1)
	{
		GetFullPath(&m_infos[info->m_parentIndex], fullPath);
		fullPath += '/';
		fullPath += info->m_name;
	}
	else
		fullPath = m_inputPath;
}

bool ueAFSPacker::WriteFileInfos(const std::vector<u32>& dirs, const std::vector<u32>& files, ioPtr* dirsPtr, ioPtr* filesPtr)
{
	// Write dir infos

	std::vector<ioPtr> subDirsPtrs;
	std::vector<ioPtr> subFilesPtrs;
	if (dirsPtr)
		m_sw.BeginPtr(*dirsPtr);
	for (u32 i = 0; i < dirs.size(); i++)
	{
		FileInfo* dir = &m_infos[ dirs[i] ];

		m_sw.WritePooledString(dir->m_name.c_str());
		m_sw.WriteNumber<u32>((u32) dir->m_dirs.size());
		subDirsPtrs.push_back( m_sw.WritePtr() );
		m_sw.WriteNumber<u32>((u32) dir->m_files.size());
		subFilesPtrs.push_back( m_sw.WritePtr() );
	}

	// Write file infos and file data

	if (filesPtr)
		m_sw.BeginPtr(*filesPtr);
	for (u32 i = 0; i < files.size(); i++)
	{
		FileInfo* file = &m_infos[ files[i] ];

		if (!m_silentMode)
			OutputInfo(file);

		ioSegmentParams fileSegmentParams;
		fileSegmentParams.m_chunkSize = m_compressedChunkSize;
		ioSegmentWriter fileSw;
		const u32 fileSegmentIndex = m_pw->BeginSegment(fileSw, fileSegmentParams);

		ueSize fileSize;
		if (!WriteFileData(file, fileSw, fileSize))
			return false;

		m_sw.WritePooledString(file->m_name.c_str());
		m_sw.WriteNumber<u32>((u32) fileSize);
		m_sw.WriteNumber<u32>(fileSegmentIndex);
	}

	// Recurse into sub dirs

	for (u32 i = 0; i < dirs.size(); i++)
	{
		FileInfo* dir = &m_infos[ dirs[i] ];

		if (!m_silentMode)
			OutputInfo(dir);

		if (!WriteFileInfos(dir->m_dirs, dir->m_files, &subDirsPtrs[i], &subFilesPtrs[i]))
			return false;
	}

	return true;
}

bool ueAFSPacker::WriteFileData(const FileInfo* info, ioSegmentWriter& fileSw, ueSize& size)
{
	// Load file to memory

	std::string fullPath;
	GetFullPath(info, fullPath);

	ioFilePtr f;
	if (!f.Open(fullPath.c_str(), ioFileOpenFlags_Read))
	{
		if (!m_silentMode)
			fprintf(stderr, "ERROR: Failed to open '%s' file.\n", fullPath.c_str());
		return false;
	}

	const ueSize fileSize = f.GetSize();
	if (fileSize == -1)
	{
		if (!m_silentMode)
			fprintf(stderr, "ERROR: Failed to get file '%s' size.\n", fullPath.c_str());
		return false;
	}

	if (fileSize == 0)
	{
		size = 0;
		return true; // It's okay for the file to be of size 0, but there's nothing to write if so
	}

	if (fileSize > ((u32) 1 << 31))
	{
		if (!m_silentMode)
			fprintf(stderr, "ERROR: File '%s' exceeds max. allowed file size.", info->m_name.c_str());
		return false;
	}
	size = (u32) fileSize;

	void* fileData = malloc(fileSize);
	if (!fileData)
	{
		free(fileData);
		if (!m_silentMode)
			fprintf(stderr, "ERROR: Failed to allocate temporary memory for '%s' file.\n", fullPath.c_str());
		return false;
	}

	if (f.Read(fileData, fileSize, 1) != 1)
	{
		free(fileData);
		if (!m_silentMode)
			fprintf(stderr, "ERROR: Failed to read in file '%s' file.\n", fullPath.c_str());
		return false;
	}

	f.Close();

	// Write file data

	fileSw.WriteData(fileData, size);

	// Release file data

	free(fileData);

	// Update stats

	m_stats.m_totalCompressedSize += fileSize;
	m_stats.m_totalSize += fileSize;

	return true;
}

void ueAFSPacker::OutputInfo(const FileInfo* info) const
{
	if (info == &m_infos[m_rootIndex])
		return;

	const u32 indent = info->m_level * 2;

	char buffer[64];
	UE_ASSERT(indent < UE_ARRAY_SIZE(buffer));
	ueMemSet(buffer, ' ', indent);
	buffer[indent] = 0;
	printf(buffer);

	fprintf(stdout, "%s '%s' ...\n", info->m_isDir ? "DIR " : "FILE", info->m_name.c_str());
}
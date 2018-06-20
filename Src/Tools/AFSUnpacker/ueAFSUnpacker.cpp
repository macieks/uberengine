#include "ueAFSUnpacker.h"
#include "IO/ioFile.h"

#include "direct.h"

extern const ioDirInfo_AFS* ioFileSys_AFS_GetRootDir(ioFileSys* fs);

ioArcFileSysUnpacker::ioArcFileSysUnpacker()
{}

bool ioArcFileSysUnpacker::UnpackArchive(const UnpackParams& params)
{
	m_silentMode = params.m_silentMode;
	m_outputPath = params.m_outputPath;

	if (!m_silentMode)
		fprintf(stdout, "Started unpacking from '%s' to '%s'...\n", params.m_inputPath, params.m_outputPath);

	// Initialize file system

	ioArchiveFileSysParams afsParams;
	afsParams.m_allocator = ueAllocator::GetGlobal();
	afsParams.m_path = params.m_inputPath;
	m_afs = ioFileSys_CreateAFS(&afsParams);
	if (!m_afs)
	{
		fprintf(stderr, "Failed to open AFS archive (path = '%s')\n", afsParams.m_path);
		return false;
	}

	// Create output directory

	if (!io_create_dir_chain(params.m_outputPath, false))
	{
		fprintf(stderr, "Failed to create output directory (path = '%s')\n", params.m_outputPath);
		return false;
	}

	// Get root directory info

	const ioDirInfo_AFS* rootDirInfo = ioFileSys_AFS_GetRootDir(m_afs);

	// Create dirs and files recursively

	for (u32 i = 0; i < rootDirInfo->m_numSubDirs; i++)
		if (!ProcessDir("", &rootDirInfo->m_subDirs[i], 0))
			goto Failure;
	for (u32 i = 0; i < rootDirInfo->m_numFiles; i++)
		if (!ProcessFile("", &rootDirInfo->m_files[i], 0))
			goto Failure;

	ioFileSys_Destroy(m_afs);

	fprintf(stdout, "DONE\n");
	return UE_TRUE;

Failure:

	ioFileSys_Destroy(m_afs);
	return UE_FALSE;
}

void ioArcFileSysUnpacker::OutputIndent(u32 level)
{
	const u32 indent = level * 2;

	char buffer[64];
	UE_ASSERT(indent < UE_ARRAY_SIZE(buffer));
	ueMemSet(buffer, ' ', indent);
	buffer[indent] = 0;
	fprintf(stdout, buffer);
}

bool ioArcFileSysUnpacker::ProcessDir(const std::string& parentPath, const ioDirInfo_AFS* dirInfo, u32 level)
{
	if (!m_silentMode)
		OutputIndent(level);

	// Create directory

	const std::string dirInName = parentPath + dirInfo->m_name;
	const std::string dirOutName = m_outputPath + "/" + parentPath + dirInfo->m_name;
	if (!m_silentMode)
		fprintf(stdout, "DIR  '%s'\n", dirInfo->m_name);
	mkdir(dirOutName.c_str());

	for (u32 i = 0; i < dirInfo->m_numFiles; i++)
		if (!ProcessFile(parentPath + dirInfo->m_name + "/", &dirInfo->m_files[i], level + 1))
			return UE_FALSE;

	for (u32 i = 0; i < dirInfo->m_numSubDirs; i++)
		if (!ProcessDir(parentPath + dirInfo->m_name + "/", &dirInfo->m_subDirs[i], level + 1))
			return UE_FALSE;

	return UE_TRUE;
}

bool ioArcFileSysUnpacker::ProcessFile(const std::string& parentPath, const ioFileInfo_AFS* fileInfo, u32 level)
{
	if (!m_silentMode)
		OutputIndent(level);

	// Read in file data from AFS

	const std::string inFileName = parentPath + fileInfo->m_name;
	if (!m_silentMode)
		fprintf(stdout, "FILE '%s' (%d bytes) ...'\n", fileInfo->m_name, fileInfo->m_size);

	void* tempMemory = NULL;
	if (fileInfo->m_size > 0)
	{
		tempMemory = malloc(fileInfo->m_size);
		ioFilePtr inFile;
		if (!inFile.Open(m_afs, inFileName.c_str(), ioFileOpenFlags_Read))
		{
			if (!m_silentMode)
				fprintf(stdout, "ERROR: Failed to open '%s' file in AFS archive.\n", inFileName.c_str());
			return UE_FALSE;
		}

		if (inFile.Read(tempMemory, fileInfo->m_size, 1) != 1)
		{
			if (!m_silentMode)
				fprintf(stdout, "ERROR: Failed to read '%s' file from AFS archive.\n", inFileName.c_str());
			return UE_FALSE;
		}
		inFile.Close();
	}

	// Write out file content to HDD

	const std::string outFileName = m_outputPath + "/" + parentPath + fileInfo->m_name;
	ioFilePtr outFile;
	if (!outFile.Open(outFileName.c_str(), ioFileOpenFlags_Write | ioFileOpenFlags_Create))
	{
		if (!m_silentMode)
			fprintf(stdout, "ERROR: Failed to open file '%s' for writing.\n", outFileName.c_str());
		return UE_FALSE;
	}

	if (fileInfo->m_size > 0 && outFile.Write(tempMemory, fileInfo->m_size, 1) != 1)
	{
		if (!m_silentMode)
			fprintf(stdout, "ERROR: Failed to write to '%s' file.\n", outFileName.c_str());
		return UE_FALSE;
	}

	outFile.Close();

	return UE_TRUE;
}
#include "IO/ioFile.h"
#include "Utils/utProcess.h"
#include "Base/ueJobSys.h"

u32 s_numFilePaths;
uePath s_filePaths[3];

ueBool FileSysCallback(const ioEnumInfo* info, void* userData)
{
	ueLogI("%s: %s", info->m_isDirectory ? "DIR " : "FILE", info->m_path);
	if (!info->m_isDirectory && s_numFilePaths < UE_ARRAY_SIZE(s_filePaths))
		ueStrCpyS(s_filePaths[s_numFilePaths++], info->m_path);
	return UE_TRUE;
}

void TestFileSys(const char* fileSysName, ioFileSys* fs, ueAllocator* allocator, ueBool async)
{
	ueLogI("Testing '%s' file system %s...", fileSysName, async ? "asynchronously" : "synchronously");

	s_numFilePaths = 0;
	ioFileSys_Enumerate(fs, "test_files/", "*.*", FileSysCallback, ioEnumFlags_Files | ioEnumFlags_Dirs | ioEnumFlags_Recursive);

	char buffer[128];

	// Read in content of all the files

	for (u32 i = 0; i < s_numFilePaths; i++)
	{
		const u32 openFlags = ioFileOpenFlags_Read | (async ? ioFileOpenFlags_Async : 0);

		ioFilePtr f;
		UE_ASSERT_FUNC(f.Open(fs, s_filePaths[i], openFlags));

		const ueSize size = f.GetSize();
		UE_ASSERT(size < UE_ARRAY_SIZE(buffer));

		f32 readCallTime, readTime;

		// Async read

		if (async)
		{
			ueTimer timer;

			// Start read

			ueAsync* async = NULL;
			f.Read(buffer, size, 1, &async);
			UE_ASSERT(async);

			readCallTime = timer.GetSecs();

			// Wait for read to complete

			const ueAsyncState state = ueAsync_Wait(async);
			UE_ASSERT(state == ueAsyncState_Succeeded);

			// Get result

			const ueSize* numElementsRead = (const ueSize*) ueAsync_GetData(async);
			UE_ASSERT(*numElementsRead == 1);

			// End read

			ueAsync_Destroy(async);

			readTime = timer.GetSecs();
		}

		// Sync read

		else
		{
			ueTimer timer;
			const ueSize numElementsRead = f.Read(buffer, size, 1);
			readCallTime = readTime = timer.GetSecs();
			UE_ASSERT(numElementsRead == 1);
		}

		// Clean up

		buffer[size] = 0;
		f.Close();

		ueLogI("Read '%s' file - call time: %.6f ms, read time: %.6f ms, content: '%s'", s_filePaths[i], readCallTime * 1000.0f, readTime * 1000.0f, buffer);
	}
}

void TestNative(ueAllocator* allocator, ueJobSys* jobSys)
{
	ioFileSys* fs = ioFileSys_GetDefault();

	TestFileSys("Native", fs, allocator, UE_TRUE);

	ioFileSys_SetJobSystem(fs, jobSys);
	TestFileSys("Native (JobSys)", fs, allocator, UE_TRUE);
	ioFileSys_SetJobSystem(fs, NULL);

	TestFileSys("Native", fs, allocator, UE_FALSE);
}

void TestAFS(ueAllocator* allocator, ueJobSys* jobSys)
{
	// Assign job system to native file system
	// The reason we do this is because async AFS operations create jobs on parent file
	// system's job queue - in our case parent file system is native file system

	ioFileSys* nativeFS = ioFileSys_GetDefault();
	ioFileSys_SetJobSystem(nativeFS, jobSys);

	// Create AFS file

	utProcessParams params;
	params.m_executableFileName = "../../Bin/Win32_Debug/AFSPacker.exe";
	params.m_arguments = "win32 \"../../Apps/UnitTests/test_files/\" \"../../Apps/UnitTests/test_files.afs\"";
	params.m_wait = UE_TRUE;

	s32 afsBuildResult;
	utProcess_Spawn(&params, NULL, &afsBuildResult);
	UE_ASSERT(afsBuildResult == 0);

	// Create and mount file system

	ioArchiveFileSysParams fsParams;
	fsParams.m_allocator = allocator;
	fsParams.m_path = "../../Apps/UnitTests/test_files.afs";
	ioFileSys* fs = ioFileSys_CreateAFS(&fsParams);
	UE_ASSERT(fs);
	ioFileSys_Mount(fs, "test_files/", 0);

	// Read AFS archive content

	TestFileSys("AFS", fs, allocator, UE_TRUE);
	TestFileSys("AFS", fs, allocator, UE_FALSE);

	// Unmount and destroy file system

	ioFileSys_Unmount(fs);
	ioFileSys_Destroy(fs);

	// Reset back native file system's job system

	ioFileSys_SetJobSystem(nativeFS, NULL);
}

void TestZIP(ueAllocator* allocator, ueJobSys* jobSys)
{
	// Create and mount file system

	ioArchiveFileSysParams fsParams;
	fsParams.m_allocator = allocator;
	fsParams.m_path = "../../Apps/UnitTests/test_files.zip";
	ioFileSys* fs = ioFileSys_CreateZip(&fsParams);
	UE_ASSERT(fs);
	ioFileSys_Mount(fs, "test_files/", 0);
	ioFileSys_SetJobSystem(fs, jobSys);

	// Read ZIP archive content

	TestFileSys("ZIP", fs, allocator, UE_TRUE);
	TestFileSys("ZIP", fs, allocator, UE_FALSE);

	// Unmount and destroy file system

	ioFileSys_Unmount(fs);
	ioFileSys_Destroy(fs);
}

void UnitTest_FileSys(ueAllocator* allocator)
{
	// Create job system

	ueJobSysThreadDesc threadDesc;
	threadDesc.m_name = "fs-job-sys";
	ueJobSysParams jobSysParams;
	jobSysParams.m_stackAllocator = allocator;
	jobSysParams.m_numThreads = 1;
	jobSysParams.m_threadDescs = &threadDesc;
	ueJobSys* jobSys = ueJobSys_Create(&jobSysParams);
	UE_ASSERT(jobSys);
	ueJobSys_Start(jobSys);

	// Run synchronous and asynchronous tests for 3 different file systems

	TestNative(allocator, jobSys);
	TestAFS(allocator, jobSys);
	TestZIP(allocator, jobSys);

	// Destroy job system

	ueJobSys_Destroy(jobSys);
}

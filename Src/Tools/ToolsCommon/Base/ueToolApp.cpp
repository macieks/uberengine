#include "Base/ueToolsBase.h"
#include "Utils/utCompression.h"

extern void ueRegisterCompressionLib_ZLIB();

struct ueToolAppData
{
	ioFileSys* m_fs;

	ueToolAppData() :
		m_fs(NULL)
	{}
};

static ueToolAppData s_data;

void ueToolApp_Startup()
{
	UE_ASSERT(!s_data.m_fs);

#if defined(UE_DEBUG)
	ueCallstackHelper_Startup();
#endif

	ueLogger_Startup();

	// Compression

	utCompression_Startup(ueAllocator::GetGlobal());
	ueRegisterCompressionLib_ZLIB();

	// File system

	ioFileSys_Startup(ueAllocator::GetGlobal());

	ioNativeFileSysParams fsParams;
	fsParams.m_allocator = ueAllocator::GetGlobal();
	fsParams.m_maxFilesOpen = 256;
	fsParams.m_maxJobs = 0;
	s_data.m_fs = ioFileSys_CreateNative(&fsParams);
	UE_ASSERT(s_data.m_fs);

	ioFileSys_Mount(s_data.m_fs, "", 0);
	ioFileSys_SetDefault(s_data.m_fs);
}

void ueToolApp_Shutdown()
{
	UE_ASSERT(s_data.m_fs);

	ioFileSys_Unmount(s_data.m_fs);
	ioFileSys_Destroy(s_data.m_fs);
	s_data.m_fs = NULL;
	ioFileSys_Shutdown();

	utCompression_Shutdown();

	ueLogger_Shutdown();

#if defined(UE_DEBUG)
	ueCallstackHelper_Shutdown();
#endif

	UE_ASSERT(!ueAllocator::GetGlobal()->HasAnyAllocation());
}

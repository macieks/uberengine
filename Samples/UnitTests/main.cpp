#include "Base/ueBase.h"
#include "Base/ueRand.h"
#include "Utils/utCompression.h"
#include "Base/ueThreading.h"
#include "Base/Allocators/ueMallocAllocator.h"
#include "IO/ioFile.h"

#define UE_RUN_UNIT_TEST(name) \
	extern void UnitTest_##name(ueAllocator* allocator); \
	ueLogI("Running '%s' unit test...", #name); \
	UnitTest_##name(allocator);

void RunUnitTests(ueAllocator* allocator)
{
	UE_RUN_UNIT_TEST(Compression);
	UE_RUN_UNIT_TEST(Script);
	UE_RUN_UNIT_TEST(Env);
	UE_RUN_UNIT_TEST(Encryption);
	UE_RUN_UNIT_TEST(List);
	UE_RUN_UNIT_TEST(HashMap);
	UE_RUN_UNIT_TEST(RedBlackTreeSet);
	UE_RUN_UNIT_TEST(Memory);
	UE_RUN_UNIT_TEST(JobSys);
	UE_RUN_UNIT_TEST(FileSys);
}

#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_LINUX)
    extern void ueRegisterCompressionLib_ZLIB();
#elif defined(UE_X360)
    extern void ueRegisterCompressionLib_XCOMPRESS();
#endif

int main(int argc, char** args)
{
	// Init

	ueMallocAllocator allocator;
	ueClock_Startup();
	ueLogger_Startup();
	ueRandMgr_Startup(&allocator, 2);

	ioFileSys_Startup(&allocator);
	ioNativeFileSysParams fsParams;
	fsParams.m_allocator = &allocator;
	fsParams.m_root = "../../Apps/UnitTests/";
	fsParams.m_maxJobs = 16;
	ioFileSys* fs = ioFileSys_CreateNative(&fsParams);
	UE_ASSERT(fs);
	ioFileSys_Mount(fs, "", 0);
	ioFileSys_SetDefault(fs);

    utCompression_Startup(&allocator, 10);
#if defined(UE_WIN32) || defined(UE_LINUX) || defined(UE_LINUX)
    ueRegisterCompressionLib_ZLIB();
#elif defined(UE_X360)
    ueRegisterCompressionLib_XCOMPRESS();
#endif

	// Run unit tests

	RunUnitTests(&allocator);

	// Shut down

    utCompression_Shutdown();

	ioFileSys_Unmount(fs);
	ioFileSys_Destroy(fs);
	ioFileSys_Shutdown();

	ueRandMgr_Shutdown();
	ueLogger_Shutdown();
	ueClock_Shutdown();

	UE_ASSERT(!allocator.HasAnyAllocation());

	return 0;
}

#include "Base/ueBase.h"

#include <intrin.h>

s32 ReadCPUSpeedFromRegistry(s32 cpuIndex)
{
	HKEY regKey;

	// Get the key name

	char regName[256];
	ueStrFormatS(regName, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d\\", cpuIndex);

	// Open the key

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
		return 0;

	// Read the value

	DWORD cpuSpeed;
	DWORD length = sizeof(DWORD);
	if (RegQueryValueEx(regKey, "~MHz", NULL, NULL, (LPBYTE)&cpuSpeed, &length) != ERROR_SUCCESS)
	{
		RegCloseKey(regKey);
		return 0;
	}

	// Cleanup and return

	RegCloseKey(regKey);
    return cpuSpeed;
}

void ueLogSystemInfo()
{
	DWORD bufferSize;
	char buffer[1024];

	ueLogI("App info (win32):");

	// Engine version & build date

	u32 majorVer, minorVer;
	ueGetEngineVersion(majorVer, minorVer);

	ueLogI("\tEngine version: %u.%u", majorVer, minorVer);
	ueLogI("\tBuild: %s %s", __DATE__, __TIME__);

	// Process name

	GetModuleFileName(NULL, buffer, UE_ARRAY_SIZE(buffer));
	ueLogI("\tProcess name: %s", buffer);

	// Current directory

	GetCurrentDirectory(UE_ARRAY_SIZE(buffer), buffer);
	ueLogI("\tCurrent directory: %s", buffer);

	// Command line

	ueLogI("\tCommand line: %s", GetCommandLineA());

	ueLogI("Machine info:");

	// Computer name

	bufferSize = UE_ARRAY_SIZE(buffer);
	GetComputerNameA(buffer, &bufferSize);
	ueLogI("\tComputer name: %s", buffer);

	// User name

	bufferSize = UE_ARRAY_SIZE(buffer);
	GetUserNameA(buffer, &bufferSize);
	ueLogI("\tUser name: %s", buffer);

	// Number of processors and speed of each one

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	for (u32 i = 0; i < systemInfo.dwNumberOfProcessors; i++)
		ueLogI("\tCPU %d: %d MHz (%d hardware threads)", i, ReadCPUSpeedFromRegistry(i), 1);

	// Operating system

	OSVERSIONINFOEX osInfo;
	ZeroMemory(&osInfo, sizeof(osInfo));
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);

	if (!GetVersionEx((OSVERSIONINFO*) &osInfo))
	{
		ZeroMemory(&osInfo, sizeof(osInfo));
		osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO*) &osInfo);
	}

	if (osInfo.dwPlatformId == 1) // Win 9x
	{
		if ((osInfo.dwMajorVersion == 4) && (osInfo.dwMinorVersion == 0))
			ueStrCpyS(buffer, "Windows 95 ");
		else if ((osInfo.dwMajorVersion == 4) && (osInfo.dwMinorVersion == 10))
			ueStrCpyS(buffer, "Windows 98 ");
		else if ((osInfo.dwMajorVersion == 4) && (osInfo.dwMinorVersion == 90))
			ueStrCpyS(buffer, "Windows ME ");
		else
			ueStrCpyS(buffer, "Unknown Windows OS ");
	}
	else if (osInfo.dwPlatformId == 2) // Win NT
	{
		if ((osInfo.dwMajorVersion == 4) && (osInfo.dwMinorVersion == 0))
			ueStrCpyS(buffer, "Windows NT 4.0 ");
		else if ((osInfo.dwMajorVersion == 5) && (osInfo.dwMinorVersion == 0))
			ueStrCpyS(buffer, "Windows 2000 ");
		else if ((osInfo.dwMajorVersion == 5) && (osInfo.dwMinorVersion == 1))
			ueStrCpyS(buffer, "Windows XP ");
		else if ((osInfo.dwMajorVersion == 5) && (osInfo.dwMinorVersion == 2))
		{
			if (GetSystemMetrics(89))	// SM_SERVERR2 == 89
				ueStrCpyS(buffer, "Windows Server 2003 R2 ");
			else if ((osInfo.wProductType == VER_NT_WORKSTATION) && (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
				ueStrCpyS(buffer, "Windows XP Professional x64 Edition ");
			else
				ueStrCpyS(buffer, "Windows Server 2003 ");
		}
		else if ((osInfo.dwMajorVersion == 6) && (osInfo.dwMinorVersion == 0))
			ueStrCpyS(buffer, "Windows Vista ");
		else
			ueStrCpyS(buffer, "Unknown WinNT OS ");
	}
	else // Unknown
		ueStrCpyS(buffer, "Unknown Operating System ");

	const u32 osNameLength = ueStrLen(buffer);
	ueStrCpy(buffer + osNameLength, UE_ARRAY_SIZE(buffer) - osNameLength, osInfo.szCSDVersion);

	ueLogI("\tOperating system: %s", buffer);

	// Processor

	s32 cpuBuffer[4];
	char cpuBrand[13];
	__cpuid(cpuBuffer, 0);
	*(int*)&cpuBrand[0] = cpuBuffer[1];
	*(int*)&cpuBrand[4] = cpuBuffer[3];
	*(int*)&cpuBrand[8] = cpuBuffer[2];
	cpuBrand[12] = 0;

	__cpuid(cpuBuffer, 0x80000000);
	const u32 highestFeatureEx = buffer[0];

	// Get processor brand name

	if (highestFeatureEx >= 0x80000004)
	{
		char cpuName[49];
		cpuName[0] = 0;
		__cpuid((s32*)&cpuName[0], 0x80000002);
		__cpuid((s32*)&cpuName[16], 0x80000003);
		__cpuid((s32*)&cpuName[32], 0x80000004);
		cpuName[48] = 0;
		for (s32 i = ueStrLen(cpuName) - 1; i >= 0; --i)
		{
			if (cpuName[i] == ' ')
				cpuName[i] = '\0';
			else
				break;
		}

		ueStrFormatS(buffer, "%s (%s)", cpuName, cpuBrand);
	}
	else
		ueStrCpyS(buffer, cpuBrand);

	ueLogI("\tProcessor: %s", buffer);

	// L1 & L2 cache sizes

	s32 L1CacheSize = -1;
	if (!ueStrCmp(cpuBrand, "AuthenticAMD") && highestFeatureEx >= 0x80000005)
	{
		__cpuid(cpuBuffer, 0x80000005);
		L1CacheSize = (u32) cpuBuffer[2] >> 24;
	}

	s32 L2CacheSize = -1;
	if (highestFeatureEx >= 0x80000006)
	{
		__cpuid(cpuBuffer, 0x80000006);
		L2CacheSize = (u32) cpuBuffer[2] >> 16;
	}

	if (L1CacheSize != -1)
		ueLogI("\tL1 cache size: %d b", L1CacheSize);
	if (L2CacheSize != -1)
		ueLogI("\tL2 cache size: %d b", L2CacheSize);

	// Physical shared memory

	MEMORYSTATUS memoryStatus;
	ZeroMemory(&memoryStatus, sizeof(memoryStatus));
	memoryStatus.dwLength = sizeof(memoryStatus);
	GlobalMemoryStatus(&memoryStatus);

	ueLogI("\tMemory: Phys = %d / %d MB Virt = %d / %d MB",
		memoryStatus.dwAvailPhys >> 20, memoryStatus.dwTotalPhys >> 20,
		memoryStatus.dwAvailVirtual >> 20, memoryStatus.dwTotalVirtual >> 20);
}
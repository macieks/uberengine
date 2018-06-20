#include "Utils/utProcess.h"
#include "IO/ioFile.h"

struct utProcessData
{
	u32 m_numArgs;
	const char** m_args;

	utProcessData() :
		m_numArgs(0),
		m_args(NULL)
	{}
};

static utProcessData s_data;

void utProcess_Startup(u32 numArgs, const char** args)
{
	s_data.m_numArgs = numArgs;
	s_data.m_args = args;
}

u32 utProcess_GetNumArgs()
{
	return s_data.m_numArgs;
}

const char* utProcess_GetArg(u32 index)
{
	UE_ASSERT(index < s_data.m_numArgs);
	return s_data.m_args[index];
}

const char* utProcess_GetArgByName(const char* name)
{
	for (u32 i = 0; i < s_data.m_numArgs; i++)
		if (s_data.m_args[i][0] == '-' && !ueStrCmp(s_data.m_args[i] + 1, name))
		{
			if (i + 1 < s_data.m_numArgs && s_data.m_args[i + 1][0] != '-')
				return s_data.m_args[i + 1];
			return "";
		}
	return NULL;
}

#if defined(UE_WIN32)

#include <windows.h>

ueBool utProcess_Spawn(utProcessParams* params, utProcess** processPtr, s32* errorCode)
{
#if 0

	s32 exitCode = (s32) _spawnlp(wait ? _P_WAIT : _P_NOWAIT, executableFileName, executableFileName, arguments, NULL);
	return exitCode;

#else

	*errorCode = 0;

	// Spawn process

	PROCESS_INFORMATION processInformation;
	STARTUPINFO startupInfo;
	memset(&processInformation, 0, sizeof(processInformation));
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	char commandLine[1024];
	ueStrFormatS(commandLine, "%s %s", params->m_executableFileName, params->m_arguments);

	if (!CreateProcessA(
		params->m_executableFileName,
		commandLine,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS | (params->m_separateWindow ? CREATE_NEW_CONSOLE : 0),
		NULL,
		NULL,
		&startupInfo,
		&processInformation))
		return UE_FALSE;

	// Close thread handle

	CloseHandle(processInformation.hThread);

	utProcess* process = (utProcess*) processInformation.hProcess;
	if (!params->m_wait)
	{
		if (processPtr)
			*processPtr = process;
		return UE_TRUE;
	}

	// Wait for process to finish and close it

	utProcess_Close(process, errorCode);
	if (processPtr)
		*processPtr = NULL;
	return UE_TRUE;

#endif
}

void utProcess_Close(utProcess* process, s32* exitCode)
{
	HANDLE handle = (HANDLE) process;

	WaitForSingleObject(handle, INFINITE);

	if (exitCode)
	{
		DWORD dwExitCode;
		if (!GetExitCodeProcess(handle, &dwExitCode))
		{
			*exitCode = 0;
			return;
		}
		*exitCode = dwExitCode;
	}

	CloseHandle(handle);
}

ueBool utProcess_IsDebuggerAttached()
{
	return IsDebuggerPresent() ? UE_TRUE : UE_FALSE;
}

#else

ueBool utProcess_IsDebuggerAttached() { return UE_FALSE; }

#endif

void utProcess_CommandLineToArgs(const char* srcBuffer, char* dstBuffer, u32 dstBufferCapacity, char** argv, u32 argvCapacity, u32* argc)
{
	// Convert command line into args

	*argc = 0;

	s32 start = 0;
	s32 pos = 0;
	for (const char* c = srcBuffer;; c++)
	{
		if (!*c || *c == ' ')
		{
			dstBuffer[pos++] = 0;
			argv[(*argc)++] = &dstBuffer[start];
			start = pos;

			if (!*c)
				break;
			continue;
		}
		dstBuffer[pos++] = *c;
	}
}

#if defined(UE_WIN32)

#include <Shlobj.h>

ueBool utProcess_GetAppDataDir(const char* companyName, const char* appName, char* dstBuffer, u32 dstBufferCapacity)
{
	if (FAILED(SHGetFolderPathA(NULL, 
								CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
                             NULL, 
                             SHGFP_TYPE_CURRENT, 
                             dstBuffer))) 
		return UE_FALSE;

	ueStrCat(dstBuffer, dstBufferCapacity, "\\");
	ueStrCat(dstBuffer, dstBufferCapacity, companyName);
	if (!ioDir_Create(ioFileSys_GetDefault(), dstBuffer, UE_FALSE))
		return UE_FALSE;

	ueStrCat(dstBuffer, dstBufferCapacity, "\\");
	ueStrCat(dstBuffer, dstBufferCapacity, appName);
	if (!ioDir_Create(ioFileSys_GetDefault(), dstBuffer, UE_FALSE))
		return UE_FALSE;

	ueStrCat(dstBuffer, dstBufferCapacity, "\\");
	return UE_TRUE;
}

#elif defined(UE_MARMALADE)

ueBool utProcess_GetAppDataDir(const char* companyName, const char* appName, char* dstBuffer, u32 dstBufferCapacity)
{
	const int32 os = s3eDeviceGetInt(S3E_DEVICE_OS);

	switch (os)
	{
		case S3E_OS_ID_IPHONE:
			ueStrCpy(dstBuffer, dstBufferCapacity, "cache://");
			break;
		default:
			ueStrCpy(dstBuffer, dstBufferCapacity, "ram://");
			break;
	}
	return UE_TRUE;
}

#else

ueBool utProcess_GetAppDataDir(const char* companyName, const char* appName, char* dstBuffer, u32 dstBufferCapacity)
{
	ueStrCpy(dstBuffer, dstBufferCapacity, "");
	return UE_TRUE;
}

#endif

#if defined(UE_WIN32)
	#include <Shellapi.h>
#elif defined(UE_MARMALADE)
	#include "s3eOSExec.h"
#endif

void utProcess_OpenURL(const char* url)
{
#if defined(UE_WIN32)
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif defined(UE_MARMALADE)
	s3eOSExecExecute(url, S3E_FALSE);
#endif
}
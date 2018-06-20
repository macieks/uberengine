#include "Base/ueBase.h"

// Fatal error

void ueDefaultFatalErrorHandlerFunc(const char* file, u32 line, u32 errorCode, const char* msg)
{
	ueExit(666);
}

static ueFatalErrorHandlerFunc s_fatalErrorHandler = ueDefaultFatalErrorHandlerFunc;

void ueSetFatalErrorHandler(ueFatalErrorHandlerFunc func)
{
	s_fatalErrorHandler = func;
}

void ueFatalErrorHandler(const char* file, u32 line, u32 errorCode, const char* msg, ...)
{
	char msgBuffer[1 << 10];
	va_list argList;
	va_start(argList, msg);
	ueStrFormatVArgs(msgBuffer, UE_ARRAY_SIZE(msgBuffer), msg, argList);
	va_end(argList);

	ueLog("== FATAL ERROR ==\n"
		"Error code: %u\n"
		"File: %s:%d\n"
		"Message: %s\n"
		"Callstack:\n",
		errorCode, file, line, msgBuffer);

	ueCallstackHelper_DumpCallstack(3);

	if (s_fatalErrorHandler)
		s_fatalErrorHandler(file, line, errorCode, msgBuffer);

#if defined(UE_DEBUG)
	UE_DEBUG_BREAK();
#endif
}

// Assertion

void UE_ASSERTionHandler(const char* file, u32 line, const char* condition, const char* msg, ...)
{
	ueLog(
		"== ASSERTION FAILED ==\n"
		"Condition: %s\n"
		"File: %s:%d\n"
		"Message: ",
		condition, file, line);

	va_list argList;
	va_start(argList, msg);
	ueLogV(msg, argList);
	va_end(argList);

	ueLog("\nCallstack:\n");
	ueCallstackHelper_DumpCallstack();
}

void ueExit(s32 exitCode)
{
	// Close all log files

	ueLogger_Shutdown();

	// Exit process

#if defined(UE_WIN32)
	ExitProcess(exitCode);
#else
	exit(exitCode);
#endif
}
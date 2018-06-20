#include "Base/ueBase.h"
#include "Base/ueThreading.h"
#include "IO/ioFile.h"

struct ueLogListener
{
	ueLoggerCallback m_callback;
	char m_context[256];
	void* m_userData;
};

struct ueLoggerData
{
	u32 m_numListeners;
	ueLogListener m_listeners[UE_MAX_LOG_LISTENERS];

	ueMutex* m_mutex;
	ueBool m_isThreadSafe;

	ueLoggerData() :
		m_numListeners(0),
#ifdef UE_DEFAULT_THREAD_SAFE
		m_isThreadSafe(UE_TRUE)
#else
		m_isThreadSafe(UE_FALSE)
#endif
	{}
};

static ueLoggerData s_data;

// Platform standard output

void ueOutput(const char* buffer, ueBool isError)
{
#if defined(UE_WIN32)
	OutputDebugStringA(buffer);
	fprintf(isError ? stderr : stdout, "%s", buffer);
#elif defined(UE_MARMALADE)
//  s3eDebugTracePrintf(buffer);
	IwTrace(UE, (buffer));
#else
	fprintf(isError ? stderr : stdout, "%s", buffer);
#endif
}

void ueOutput_StdOut(const ueLogContext& context, const char* buffer, void* userData)
{
	char finalBuffer[1 << 10];
	const ueBool isContextfull = context.m_fileName != NULL;
	const ueBool isError = context.m_loggingLevel == ueLoggingLevel_Error;

	finalBuffer[0] = 0;
#ifndef UE_MARMALADE // Marmalade already adds date/time
	if (isContextfull)
		ueStrFormatS(finalBuffer, "%s [%s:%s:%d] ", ueDateTime::GetCurrent().ToString("hh:mm:ss"), ueLogger_ToString(context.m_loggingLevel), context.m_functionName, context.m_line);
#endif

	ueStrCatFit(finalBuffer, UE_ARRAY_SIZE(finalBuffer) - 1, buffer);

	if (isContextfull)
		ueStrCatFit(finalBuffer, UE_ARRAY_SIZE(finalBuffer), "\n");

	ueOutput(finalBuffer, isError);
}

// ueLogger

void ueLogger_Startup()
{
	UE_ASSERT(s_data.m_numListeners == 0);
	s_data.m_mutex = ueMutex_Create();
	ueLogger_RegisterOutputListener(ueOutput_StdOut, NULL);
}

void ueLogger_Shutdown()
{
	ueLogger_UnregisterOutputListener(ueOutput_StdOut, NULL);
	ueMutex_Destroy(s_data.m_mutex);
	UE_ASSERT(s_data.m_numListeners == 0);
}

void ueLogger_SetThreadSafe(ueBool threadSafe)
{
	s_data.m_isThreadSafe = threadSafe;
}

void ueLogger_RegisterOutputListener(ueLoggerCallback callback, void* userData, const char* context)
{
	ueMutexLock guard(s_data.m_mutex);
	UE_ASSERT(s_data.m_numListeners < UE_MAX_LOG_LISTENERS);
	ueLogListener* listener = &s_data.m_listeners[s_data.m_numListeners++];
	listener->m_callback = callback;
	listener->m_userData = userData;
	ueStrCpyS(listener->m_context, context);
}

void ueLogger_UnregisterOutputListener(ueLoggerCallback callback, void* userData)
{
	ueMutexLock guard(s_data.m_mutex);
	for (u32 i = 0; i < s_data.m_numListeners; i++)
		if (s_data.m_listeners[i].m_callback == callback &&
			s_data.m_listeners[i].m_userData == userData)
		{
			s_data.m_listeners[i] = s_data.m_listeners[ --s_data.m_numListeners ];
			return;
		}
}

void ueLogger_LogV(const ueLogContext& context, const char* format, va_list argList)
{
	// Compose text

	char buffer[1 << 14];
	if (!ueStrFormatVArgs(buffer, UE_ARRAY_SIZE(buffer), format, argList))
	{
		// Report problem
		ueStrCpy(buffer, UE_ARRAY_SIZE(buffer),
			"Error: failed to output string because it's too long or invalid.\n"
			"Following is the beginning of the string:\n");
		const u32 errorMsgLength = ueStrLen(buffer);

		const u32 formatLength = ueStrLen(format);
		const u32 formatLengthToOutput = ueMin(formatLength, UE_ARRAY_SIZE(buffer) - formatLength - 1);
		ueMemCpy(buffer + errorMsgLength, format, formatLengthToOutput);
		buffer[errorMsgLength + formatLengthToOutput] = '\0';
	}

	// Notify all listeners (filter by context)
	{
		if (s_data.m_isThreadSafe)
			ueMutex_Lock(s_data.m_mutex);

		for (u32 i = 0; i < s_data.m_numListeners; ++i)
		{
			ueLogListener* listener = &s_data.m_listeners[i];
			if (listener->m_context[0] && ueStrCmp(listener->m_context, context.m_ctxName))
				continue;
			listener->m_callback(context, buffer, listener->m_userData);
		}

		if (s_data.m_isThreadSafe)
			ueMutex_Unlock(s_data.m_mutex);
	}
}

const char* ueLogger_ToString(ueLoggingLevel level)
{
	switch (level)
	{
		case ueLoggingLevel_Debug: return "DBG";
		case ueLoggingLevel_Info: return "INF";
		case ueLoggingLevel_Warning: return "WRN";
		case ueLoggingLevel_Error: return "ERR";
	}
	return "";
}

// ueLogFile

ueLogFile::ueLogFile() :
	m_file(NULL)
{}

ueLogFile::~ueLogFile()
{
	Deinit();
}

void ueLogFile::Init(const char* fileName, const char* context)
{
	UE_ASSERT(!m_file);
	m_file = ioFile_Open(ioFileSys_GetDefault(), fileName, ioFileOpenFlags_Create | ioFileOpenFlags_Write | ioFileOpenFlags_NoBuffer);
	if (!m_file)
	{
		ueLogW("Failed to create log file '%s' (can't open file for writing)", fileName);
		return;
	}

	ueLogger_RegisterOutputListener(Callback, this, context);

	ueLogI("Opened log file at %s", ueDateTime::GetCurrent().ToString());
}

void ueLogFile::Deinit()
{
	if (!m_file)
		return;

	ueLogI("Closed log file at %s", ueDateTime::GetCurrent().ToString());

	ueLogger_UnregisterOutputListener(Callback, this);

	const char eof = 0;

	ioFile_Write(m_file, &eof, 1);
	ioFile_Close(m_file);
	m_file = NULL;
}

void ueLogFile::Callback(const ueLogContext& context, const char* buffer, void* userData)
{
	ueLogFile* f = (ueLogFile*) userData;

	char contextString[1 << 8];
	const ueBool isContextfull = context.m_fileName != NULL;
	if (isContextfull && ueStrFormatS(contextString, "%s [%s:%s:%d] ", ueDateTime::GetCurrent().ToString("hh:mm:ss"), ueLogger_ToString(context.m_loggingLevel), context.m_functionName, context.m_line))
		ioFile_Write(f->m_file, contextString, ueStrLen(contextString));

	ioFile_Write(f->m_file, buffer, ueStrLen(buffer));

	if (isContextfull)
		ioFile_Write(f->m_file, "\n", 1);
}

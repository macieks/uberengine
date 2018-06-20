#ifndef UE_LOGGER_H
#define UE_LOGGER_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueConfig.h"

struct ioFile;

//! Platform independent string output
void ueOutput(const char* buffer, ueBool isError);

//! Available logging importance levels
enum ueLoggingLevel
{
	ueLoggingLevel_Debug = 0,
	ueLoggingLevel_Info,
	ueLoggingLevel_Warning,
	ueLoggingLevel_Error
};

//! Context of the log output
struct ueLogContext
{
	const char* m_ctxName;			//!< Optional context name
	const char* m_fileName;			//!< File name from which the logging function was invoked
	u32 m_line;						//!< Line number of the file from which the logging function was invoked
	const char* m_functionName;		//!< Containing function name
	ueLoggingLevel m_loggingLevel;	//!< Importance level of this log

	ueLogContext(const char* ctxName = NULL, const char* fileName = NULL, const u32 line = 0, const char* functionName = NULL, ueLoggingLevel level = ueLoggingLevel_Debug) :
		m_ctxName(ctxName),
		m_fileName(fileName),
		m_line(line),
		m_functionName(functionName),
		m_loggingLevel(level)
	{}
};

//! Logger callback function
typedef void (*ueLoggerCallback)(const ueLogContext& context, const char* buffer, void* userData);

// Engine logger
// --------------------------

//! Starts up engine logger
void ueLogger_Startup();
//! Shuts down engine logger
void ueLogger_Shutdown();

//! Sets thread safety for the logger; default bahviour is determined based on UE_DEFAULT_THREAD_SAFE macro
void ueLogger_SetThreadSafe(ueBool threadSafe);

//! Registers logger output listener; optional context allows for capturing only given context
void ueLogger_RegisterOutputListener(ueLoggerCallback callback, void* userData, const char* context = NULL);
//! Unregisters logger output listener
void ueLogger_UnregisterOutputListener(ueLoggerCallback callback, void* userData);

//! Logs message
void ueLogger_LogV(const ueLogContext& context, const char* format, va_list argList);
//! Gets logging level string name
const char* ueLogger_ToString(ueLoggingLevel level);

//! Log helper class to allow for automatic log context setup when using logging macros (e.g. ueLogI)
class ueLogHelper
{
public:
	ueLogHelper(const char* ctxName = NULL, const char* fileName = NULL, const u32 line = 0, const char* functionName = NULL, ueLoggingLevel level = ueLoggingLevel_Debug) :
		m_context(ctxName, fileName, line, functionName, level)
	{}

	UE_INLINE void LogIf(ueBool condition, const char* format, ...)
	{
		if (!condition) return;

		va_list argList;
		va_start(argList, format);
		ueLogger_LogV(m_context, format, argList);
		va_end(argList);
	}

	UE_INLINE void Log(const char* format, ...)
	{
		va_list argList;
		va_start(argList, format);
		ueLogger_LogV(m_context, format, argList);
		va_end(argList);
	}

	UE_INLINE void LogCtx(const char* ctxName, const char* format, ...)
	{
		m_context.m_ctxName = ctxName;

		va_list argList;
		va_start(argList, format);
		ueLogger_LogV(m_context, format, argList);
		va_end(argList);
	}

	UE_INLINE void LogV(const char* format, va_list argList)
	{
		ueLogger_LogV(m_context, format, argList);
	}

private:
	ueLogContext m_context;
};

#if defined(UE_ENABLE_LOGGING)

	//! Logs context-less message
	#define ueLog  ueLogHelper(NULL, NULL, NULL, NULL, ueLoggingLevel_Info).Log
	//! Logs info message
	#define ueLogI ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Info).Log
	//! Logs warning message
	#define ueLogW ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Warning).Log
	//! Logs error message
	#define ueLogE ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Error).Log
	//! Logs debug message
	#define ueLogD ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Debug).Log

	#define ueLogV  ueLogHelper(NULL, NULL, NULL, NULL, ueLoggingLevel_Info).LogV
	#define ueLogVI ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Info).LogV
	#define ueLogVW ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Warning).LogV
	#define ueLogVE ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Error).LogV
	#define ueLogVD ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Debug).LogV

	#define ueLogCtxI ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Info).LogCtx
	#define ueLogCtxW ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Warning).LogCtx
	#define ueLogCtxE ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Error).LogCtx
	#define ueLogCtxD ueLogHelper(NULL, __FILE__, __LINE__, __FUNCTION__, ueLoggingLevel_Debug).LogCtx

#else

	#define ueLog  UE_NOOP
	#define ueLogI UE_NOOP
	#define ueLogE UE_NOOP
	#define ueLogW UE_NOOP
	#define ueLogD UE_NOOP

	#define ueLogV  UE_NOOP
	#define ueLogVI UE_NOOP
	#define ueLogVW UE_NOOP
	#define ueLogVE UE_NOOP
	#define ueLogVD UE_NOOP

	#define ueLogCtxI UE_NOOP
	#define ueLogCtxE UE_NOOP
	#define ueLogCtxW UE_NOOP
	#define ueLogCtxD UE_NOOP

#endif

//! Helper class used to output logs to file
class ueLogFile : public ueLinkedList<ueLogFile>::Node
{
public:
	ueLogFile();
	~ueLogFile();

	//! Initializes log file output
	void Init(const char* fileName, const char* context = NULL);
	//! Deinitializes log file output
	void Deinit();

private:
	static void Callback(const ueLogContext& context, const char* buffer, void* userData);
	ioFile* m_file;
};

// @}

#endif // UE_LOGGER_H

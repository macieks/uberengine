#ifndef UE_DEBUG_H
#define UE_DEBUG_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Empty operation
#define UE_NOOP() do {} while(0)
//! Non-empty operation
#define UE_OP(op) do { op } while(0)
//! Macro used to suppress MSVC linker warning LNK4221: no public symbols found; archive member will be inaccessible
#define UE_NO_EMPTY_FILE namespace { char NoEmptyFileDummy##__LINE__; }

#if defined(UE_WIN32) || defined(UE_X360)
	#define UE_DEBUG_BREAK() DebugBreak()
#elif defined(UE_LINUX)
	#define UE_DEBUG_BREAK() asm("int $3")
#elif defined(UE_MARMALADE)
	#define UE_DEBUG_BREAK()
#else
	"unsupported platform"
#endif

//! Fatal error handler function signature
typedef void (*ueFatalErrorHandlerFunc)(const char* file, u32 line, u32 errorCode, const char* msg);

//! Sets fatal error handler
void ueSetFatalErrorHandler(ueFatalErrorHandlerFunc func);
//! Function to handle fatal error
void ueFatalErrorHandler(const char* file, u32 line, u32 errorCode, const char* msg, ...);

//! Fatal error
#define ueFatalError(msg) ueFatalErrorHandler(__FILE__, __LINE__, 0, msg);
//! Fatal error with trailing parameters
#define ueFatalErrorP(msg, ...) ueFatalErrorHandler(__FILE__, __LINE__, 0, msg, __VA_ARGS__);
//! Fatal error with error code
#define ueFatalErrorC(errorCode, msg) ueFatalErrorHandler(__FILE__, __LINE__, errorCode, msg);
//! Fatal error with error code and trailing parameters
#define ueFatalErrorCP(errorCode, msg, ...) ueFatalErrorHandler(__FILE__, __LINE__, errorCode, msg, __VA_ARGS__);

#if defined(UE_ENABLE_ASSERTION)

//! Assertion handler
void UE_ASSERTionHandler(const char* file, u32 line, const char* condition, const char* msg, ...);

//! Assertion
#define UE_ASSERT(condition)                 UE_OP(if (!(condition)) { UE_ASSERTionHandler(__FILE__, __LINE__, #condition,                "<assert>"                ); UE_DEBUG_BREAK(); })
//! Assertion with message
#define UE_ASSERT_MSG(condition, msg, ...)    UE_OP(if (!(condition)) { UE_ASSERTionHandler(__FILE__, __LINE__, #condition,                msg                       ); UE_DEBUG_BREAK(); })
//! Assertion with message and trailing parameters
#define UE_ASSERT_MSGP(condition, msg, ...)   UE_OP(if (!(condition)) { UE_ASSERTionHandler(__FILE__, __LINE__, #condition,                msg,          __VA_ARGS__ ); UE_DEBUG_BREAK(); })
//! Assertion of a function call with message and trailing parameters
#define UE_ASSERT_FUNC(operation)             UE_OP(if (!(operation)) { UE_ASSERTionHandler(__FILE__, __LINE__, #operation,                "<verify>"                ); UE_DEBUG_BREAK(); })
//! Assertion of a function call with message
#define UE_ASSERT_FUNC_MSG(operation, msg, ...) UE_OP(if (!(operation)) { UE_ASSERTionHandler(__FILE__, __LINE__, #operation,                msg                       ); UE_DEBUG_BREAK(); })
//! Assertion of a function call with message and trailing parameters
#define UE_ASSERT_FUNC_MSGP(operation, msg, ...) UE_OP(if (!(operation)) { UE_ASSERTionHandler(__FILE__, __LINE__, #operation,                msg,          __VA_ARGS__ ); UE_DEBUG_BREAK(); })
//! Assertion for a code that hasn't been yet implemented
#define UE_NOT_IMPLEMENTED()               UE_OP(                  { UE_ASSERTionHandler(__FILE__, __LINE__, "<not-yet-implemented>",   "<assert>"                ); UE_DEBUG_BREAK(); })

#else // defined(UE_ENABLE_ASSERTION)

#define UE_ASSERT(condition)                 UE_NOOP()
#define UE_ASSERT_MSG(condition, msg, ...)    UE_NOOP()
#define UE_ASSERT_MSGP(condition, msg, ...)    UE_NOOP()
#define UE_ASSERT_FUNC(operation)                 operation
#define UE_ASSERT_FUNC_MSG(operation, msg, ...)    operation
#define UE_ASSERT_FUNC_MSGP(operation, msg, ...)    operation
#define UE_NOT_IMPLEMENTED()               UE_NOOP()

#endif // defined(UE_ENABLE_ASSERTION)

//! Ends the calling process and all its threads
void ueExit(s32 exitCode);

// @}

#endif // UE_DEBUG_H

#include "Base/ueBase.h"

#include <Dbghelp.h>
#pragma comment(lib, "dbghelp")

//#define UE_CALLSTACK_USE_WALK_STACK_64
#define UE_CALLSTACK_USE_RTL

#if defined(UE_CALLSTACK_USE_RTL)

	struct
	{
	  unsigned __int64 imageOffset;
	  unsigned int imageSize;
	} CallstackHelpResultInfo;
	
	typedef USHORT  (__stdcall *tRtlCaptureStackBackTrace)(IN ULONG FramesToSkip, IN ULONG FramesToCapture, OUT PVOID *BackTrace, OUT PULONG BackTraceHash);

	static HINSTANCE s_kernel32Dll = NULL;
	static tRtlCaptureStackBackTrace s_rtlCaptureStackBackTrace = NULL;

#endif

static ueBool s_isInitialized = UE_FALSE;
static HANDLE s_process = NULL;

void ueCallstackHelper_Startup()
{
	UE_ASSERT(!s_isInitialized);

	// Get process handle

	s_process = GetCurrentProcess();
	UE_ASSERT(s_process);

	// Initialize symbols

	DWORD symOptions = SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS;
	SymSetOptions(symOptions);

	const ueBool symbolsLoadingResult = SymInitialize(s_process, 0, TRUE) == TRUE;
	UE_ASSERT(symbolsLoadingResult);

#if defined(UE_CALLSTACK_USE_RTL)

	// Load RTL function for getting callstack

	s_kernel32Dll = LoadLibraryA("kernel32.dll");
	UE_ASSERT(s_kernel32Dll);
	s_rtlCaptureStackBackTrace = (tRtlCaptureStackBackTrace) GetProcAddress(s_kernel32Dll, "RtlCaptureStackBackTrace");
	UE_ASSERT(s_rtlCaptureStackBackTrace);

#endif

	s_isInitialized = UE_TRUE;
}

void ueCallstackHelper_Shutdown()
{
	UE_ASSERT(s_isInitialized);

	SymCleanup(s_process);
	s_process = NULL;

#if defined(UE_CALLSTACK_USE_RTL)

	s_rtlCaptureStackBackTrace = NULL;

	FreeModule(s_kernel32Dll);
	s_kernel32Dll = NULL;

#endif

	s_isInitialized = UE_FALSE;
}

ueBool ueCallstackHelper_GetCallstack(void** frames, u32& framesSize, u32 framesToSkip)
{
	if (!s_isInitialized)
	{
		framesSize = 0;
		return UE_FALSE;
	}

#if defined(UE_CALLSTACK_USE_WALK_STACK_64)

	HANDLE thread = GetCurrentThread();

	CONTEXT context;
	ueMemZeroS(context);
	context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&context);

	STACKFRAME64 stackFrame;
	ueMemZeroS(stackFrame);
	DWORD imageType;
#ifdef _M_IX86
	imageType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
#elif _M_X64
	imageType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	imageType = IMAGE_FILE_MACHINE_IA64;
	stackFrame.AddrPC.Offset = context.StIIP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.IntSp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Offset = context.RsBSP;
	stackFrame.AddrBStore.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.IntSp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#else
	#error "Platform not supported!"
#endif

	u32 level = 0;
	while ((s32) level - (s32) framesToSkip < (s32) framesSize &&
		StackWalk64(
			imageType,
			s_process,
			thread,
			&stackFrame,
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL))
	{
		if (level >= framesToSkip)
			frames[level - framesToSkip] = (void*) stackFrame.AddrPC.Offset;
		level++;
	}

	framesSize = (u32) ueMax((s32) 0, (s32) level - (s32) framesToSkip);

#elif defined(UE_CALLSTACK_USE_RTL)

	framesSize = s_rtlCaptureStackBackTrace(framesToSkip, framesSize, frames, NULL);

#else

	#error "No implementation"

#endif

	return UE_TRUE;
}

ueBool ueCallstackHelper_GetFrameDetails(void* frame, const char*& fileName, u32& lineNumber, char* symbolName, u32 symbolNameBufferSize)
{
	UE_ASSERT(s_isInitialized);

	// Get file name and line number

	DWORD displacement;
	IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
	if (!SymGetLineFromAddr64(s_process, (DWORD64) frame, &displacement, &lineInfo))
		return UE_FALSE;

	fileName = lineInfo.FileName;
	lineNumber = lineInfo.LineNumber;

	// Get symbol name

	if (symbolName)
	{
		u8 buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		SYMBOL_INFO* symbolPtr = (SYMBOL_INFO*) buffer;
		symbolPtr->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbolPtr->MaxNameLen = MAX_SYM_NAME;

		DWORD64 displacement64;
		if (!SymFromAddr(s_process, (DWORD64) frame, &displacement64, symbolPtr))
			return UE_FALSE;

		ueStrCpy(symbolName, symbolNameBufferSize, symbolPtr->Name);
	}

	return UE_TRUE;
}
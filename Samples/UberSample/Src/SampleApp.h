#pragma once

#include "Base/ueBase.h"
#include "Base/Allocators/ueMallocAllocator.h"
#include "Base/Allocators/ueDLMallocAllocator.h"
#include "Base/Allocators/ueStackAllocator.h"

#include "Sample.h"

class ueSample;
struct ueProfilerCapture;
struct glRenderGroup;
struct glCtx;
struct ueWindow;
struct ueMemoryDebugger;

//! The main application capable of switching between available samples
class ueSampleApp
{
public:
	ueSampleApp();
	void Run(u32 argc, const char** argv);

	glCtx* BeginDrawing();
	void EndDrawing();
	void DrawAppOverlay();

	UE_INLINE const ueAppTimer& GetGameTimer() const { return m_timer; }

	UE_INLINE ueAllocator* GetFreqAllocator() { return &m_freqAllocator; }
	UE_INLINE ueAllocator* GetStackAllocator() { return &m_stackAllocator; }
	UE_INLINE ueMemoryDebugger* GetMemoryDebugger() { return m_memoryDebugger; }

	UE_INLINE void Quit() { m_quit = UE_TRUE; }

#if defined(UE_WIN32)
	UE_INLINE ueWindow* GetWindow() { return m_window; }
#endif // defined(UE_WIN32)

private:
	void RestartSample();
	void NextSample();
	void PrevSample();
	void SetSample(ueSampleCreateFunc sample);

	static void gxDebugMenuCallback_RestartSample(const char* actionName, void* userData);
	static void gxDebugMenuCallback_NextSample(const char* actionName, void* userData);
	static void gxDebugMenuCallback_PrevSample(const char* actionName, void* userData);
	static void gxDebugMenuCallback_Quit(const char* actionName, void* userData);

	static void inActiveConsumerChangedCallback(const char* consumerName);
	char m_inputConsumerName[64];
	ueTime m_inputConsumerChangeTime;

	static void FatalErrorHandlerFunc(const char* file, u32 line, u32 errorCode, const char* msg);

	ueMallocAllocator m_sysAllocator;
	ueDLMallocAllocator m_freqAllocator;
	ueStackAllocator m_stackAllocator;
	ueMemoryDebugger* m_memoryDebugger;

	ueBool m_quit;
	ueAppTimer m_timer;

	ueProfilerCapture* m_profCapture;

	ueSampleCreateFunc m_sampleCreateFunc;
	ueSample* m_sample;

#if defined(UE_WIN32)
	ueWindow* m_window;
#endif // defined(UE_WIN32)
};

extern ueSampleApp g_app;
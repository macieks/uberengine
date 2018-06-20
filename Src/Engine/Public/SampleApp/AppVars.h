#pragma once

struct ueVar;

//! Common variables
struct ueAppVars
{
	static ueVar* m_resolution;
	static ueVar* m_fullscreen;
	static ueVar* m_frequency;
	static ueVar* m_msaaLevel;
	static ueVar* m_audioVolume;

	static void Startup();
	static void Shutdown();

	static void AddToDebugMenu(const char* configFilePath);

	static void GetResolution(u32& width, u32& height);
	static void Apply(const char* actionName, void* userData);
};

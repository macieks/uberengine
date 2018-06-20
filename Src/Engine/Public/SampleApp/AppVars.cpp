#include "Utils/ueWindow.h"
#include "Audio/auLib.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxDebugMenu.h"
#include "AppVars.h"
#include "App.h"

ueVar* ueAppVars::m_resolution;
ueVar* ueAppVars::m_fullscreen;
ueVar* ueAppVars::m_frequency;
ueVar* ueAppVars::m_msaaLevel;
ueVar* ueAppVars::m_audioVolume;

void ueAppVars_FrequencyStepCallback(ueVar* var, ueBool increase);
void ueAppVars_MSAALevelStepCallback(ueVar* var, ueBool increase);
void ueAppVars_ResolutionStepCallback(ueVar* var, ueBool increase);

void ueAppVars::Startup()
{
	m_resolution = ueVar_CreateString("settings.resolution", 16, "800x600");
	ueVar_SetStepCallback(m_resolution, ueAppVars_ResolutionStepCallback);
	m_fullscreen = ueVar_CreateBool("settings.fullscreen", UE_FALSE);
	m_frequency = ueVar_CreateS32("settings.frequency", 60, 60, 120);
	ueVar_SetStepCallback(m_frequency, ueAppVars_FrequencyStepCallback);
	m_msaaLevel = ueVar_CreateS32("settings.msaa", 1, 1, 32);
	ueVar_SetStepCallback(m_msaaLevel, ueAppVars_MSAALevelStepCallback);
	m_audioVolume = ueVar_CreateS32("settings.audio_volume", 10, 0, 10);
}

void ueAppVars::Shutdown()
{
	ueVar_Destroy(m_audioVolume);
	ueVar_Destroy(m_resolution);
	ueVar_Destroy(m_fullscreen);
	ueVar_Destroy(m_frequency);
	ueVar_Destroy(m_msaaLevel);
}

void ueAppVars::AddToDebugMenu(const char* configFilePath)
{
	char command[512];

	gxDebugMenuItem* settingsMenu = gxDebugMenu_CreateMenu(NULL, "Settings");

	ueStrFormatS(command, "save_vars %s", configFilePath);
	gxDebugMenu_CreateCommand(settingsMenu, "Save settings", command);
	ueStrFormatS(command, "do_file %s", configFilePath);
	gxDebugMenu_CreateCommand(settingsMenu, "Load settings", command);
	gxDebugMenu_CreateCommand(settingsMenu, "List all vars", "list_vars");
	gxDebugMenu_CreateUserAction(settingsMenu, "Apply settings", Apply);
	gxDebugMenu_CreateVar(settingsMenu, m_resolution);
	gxDebugMenu_CreateVar(settingsMenu, m_fullscreen);
	gxDebugMenu_CreateVar(settingsMenu, m_frequency);
	gxDebugMenu_CreateVar(settingsMenu, m_msaaLevel);
	gxDebugMenu_CreateVar(settingsMenu, m_audioVolume);
}

void ueAppVars::GetResolution(u32& width, u32& height)
{
	const char* resString = ueVar_GetString(m_resolution);
	if (ueStrScanf(resString, "%ux%u", &width, &height) != 2)
	{
		// Fallback to defaults

		ueVar_SetString(m_resolution, "800x600");
		width = 800;
		height = 600;
	}
}

void ueAppVars::Apply(const char* actionName, void* userData)
{
	u32 width, height;
	GetResolution(width, height);

#if defined(GL_OPENGL) && !defined(GL_OPENGL_ES)

	ueWindow_ChangeMode(g_app.GetWindow(), ueVar_GetBool(m_fullscreen), width, height, ueVar_GetS32(m_frequency));

	// TODO: Switch MSAA level as well

#elif defined(GL_D3D9) //|| defined(GL_D3D11)

	// Change device mode

	glDeviceStartupParams params = *glDevice_GetStartupParams();
	params.m_fullscreen = ueVar_GetBool(m_fullscreen);
	params.m_width = width;
	params.m_height = height;
	params.m_MSAALevel = ueVar_GetS32(m_msaaLevel);
	params.m_frequency = ueVar_GetS32(m_frequency);
	if (params.m_fullscreen)
		glDevice_GetClosestSupportedMode(&params);
	glDevice_Reset(&params);

	// Change window mode

	ueWindow_ChangeMode(g_app.GetWindow(), ueVar_GetBool(m_fullscreen), params.m_width, params.m_height, 0);

#endif

	auLib_SetVolume("Default", (f32) ueVar_GetS32(m_audioVolume) / 10.0f);
}

void ueAppVars_ResolutionStepCallback(ueVar* var, ueBool increase)
{
	const char* availableRes[] =
	{
		"640x480",
		"640x360", // Half HD
		"800x600",
		"1024x768",
		"1280x720" // HD
	};

	const char* currentValue = ueVar_GetString(var);

	u32 currentIndex = U32_MAX;
	for (u32 i = 0; i < UE_ARRAY_SIZE(availableRes); i++)
		if (!ueStrCmp(availableRes[i], currentValue))
		{
			currentIndex = i;
			break;
		}

	if (currentIndex == U32_MAX)
		ueVar_SetString(var, availableRes[0]);
	else if (increase && currentIndex + 1 < UE_ARRAY_SIZE(availableRes))
		ueVar_SetString(var, availableRes[currentIndex + 1]);
	else if (!increase && 0 < currentIndex)
		ueVar_SetString(var, availableRes[currentIndex - 1]);
}

void ueAppVars_MSAALevelStepCallback(ueVar* var, ueBool increase)
{
	const glCaps* caps = glDevice_GetCaps();

	const s32 currentLevel = ueVar_GetS32(var);

	u32 currentLevelIndex = U32_MAX;
	for (u32 i = 0; i < caps->m_numMSAALevels; i++)
		if (caps->m_MSAALevels[i] == (u32) currentLevel)
		{
			currentLevelIndex = i;
			break;
		}

	if (currentLevelIndex == U32_MAX)
		ueVar_SetS32(var, caps->m_MSAALevels[0]);
	else if (increase && currentLevelIndex + 1 < caps->m_numMSAALevels)
		ueVar_SetS32(var, caps->m_MSAALevels[currentLevelIndex + 1]);
	else if (!increase && 0 < currentLevelIndex)
		ueVar_SetS32(var, caps->m_MSAALevels[currentLevelIndex - 1]);
}

void ueAppVars_FrequencyStepCallback(ueVar* var, ueBool increase)
{
	const s32 availableValues[] = {60, 75, 90, 100, 120};

	const s32 currentValue = ueVar_GetS32(var);

	u32 currentValueIndex = U32_MAX;
	for (u32 i = 0; i < UE_ARRAY_SIZE(availableValues); i++)
		if (availableValues[i] == currentValue)
		{
			currentValueIndex = i;
			break;
		}
	if (currentValueIndex == U32_MAX)
		ueVar_SetS32(var, availableValues[0]);
	else if (increase && currentValueIndex + 1 < UE_ARRAY_SIZE(availableValues))
		ueVar_SetS32(var, availableValues[currentValueIndex + 1]);
	else if (!increase && 0 < currentValueIndex)
		ueVar_SetS32(var, availableValues[currentValueIndex - 1]);
}
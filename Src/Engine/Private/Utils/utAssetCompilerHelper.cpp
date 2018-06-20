#include "Utils/utAssetCompilerHelper.h"
#include "Utils/utProcess.h"

static char s_targets[512];
static utProcess* s_process = NULL;

ueBool utAssetCompilerHelper_Run(utAssetCompilerHelperConfig* config, s32* errorCode)
{
	UE_ASSERT(!s_process);

	s_targets[0] = 0;

#if defined(UE_WIN32)
	ueStrCatS(s_targets, "win32-");
#elif defined(UE_WIN32)
	ueStrCatS(s_targets, "x360-");
#elif defined(UE_LINUX)
	ueStrCatS(s_targets, "linux-");
#elif defined(UE_MAC)
	ueStrCatS(s_targets, "mac-");
#else
	ueStrCatS(s_targets, "marmalade");
#endif

#if defined(PH_PHYSX)
	ueStrCatS(s_targets, "physx-");
#elif defined(PH_BULLET)
	ueStrCatS(s_targets, "bullet-");
#endif

#if defined(GL_OPENGL_ES)
	ueStrCatS(s_targets, "gles");
#elif defined(GL_OPENGL)
	ueStrCatS(s_targets, "gl");
#elif defined(GL_D3D9)
	ueStrCatS(s_targets, "d3d9_sm");
	ueStrCatS(s_targets, config->m_shaderModel);
#elif defined(GL_D3D11)
	ueStrCatS(s_targets, "d3d11_sm40");
#elif defined(GL_MARMALADE)
	// Do nothing
#else
	#error "Unsupported graphics library"
#endif

	char args[1024];
	ueStrFormatS(args, "-noWait -waitOnError %s%s-targetsFile %s -targets %s -source %s",
		!config->m_build ? "-noBuild " : "",
		config->m_enableAssetWatcher ? "-enableAssetWatcher " : "",
		config->m_targetsFile,
		s_targets,
		config->m_assetsFile);

	utProcessParams params;
	params.m_executableFileName = config->m_assetCompilerPath;
	params.m_arguments = args;
	params.m_wait = !config->m_enableAssetWatcher;
	params.m_separateWindow = UE_TRUE;

	ueLogI("Compiling assets...");
	ueLogI("Running '%s' with args '%s'...", config->m_assetCompilerPath, args);

	if (!utProcess_Spawn(&params, &s_process, errorCode))
	{
		ueLogE("Failed to build assets (asset compiler process failed to start; does '%s' exist?).", params.m_executableFileName);
		return UE_FALSE;
	}

	if (*errorCode)
	{
		ueLogE("Failed to build assets (asset compiler returned %d).", *errorCode);
		return UE_FALSE;
	}

	ueLogI("Assets built successfully.");
	return UE_TRUE;
}

void utAssetCompilerHelper_Stop()
{
	if (s_process)
	{
		s32 exitCode;
		utProcess_Close(s_process, &exitCode);
		s_process = NULL;
	}
}
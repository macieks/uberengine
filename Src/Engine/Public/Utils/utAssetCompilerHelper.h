#ifndef UT_ASSET_COMPILER_HELPER_H
#define UT_ASSET_COMPILER_HELPER_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

/*
	Helper utility used to (re)build assets at run-time including optional on-demand resource hotswapping.
*/

//! Asset compiler helper config
struct utAssetCompilerHelperConfig
{
	const char* m_assetCompilerPath;	//!< Asset compiler executable path
	ueBool m_build;						//!< Indicates whether to enable regular building of assets; enabled by default
	ueBool m_enableAssetWatcher;		//!< Indicates whether to enable asset watcher (rebuilds assets when change on disc is detected); disabled by default
	const char* m_targetsFile;			//!< Path to targets Xml file
	const char* m_assetsFile;			//!< Path to assets Xml file
	const char* m_shaderModel;			//!< Shader model version; only used on DX for now

	utAssetCompilerHelperConfig() :
		m_assetCompilerPath("../../../../Bin/Win32_Release/AssetCompiler.exe"),
		m_build(UE_TRUE),
		m_enableAssetWatcher(UE_FALSE),
		m_targetsFile("../../Data/Src/game_targets.xml"),
		m_assetsFile("../../Data/Src/game_assets.xml"),
		m_shaderModel("30")
	{}
};

//! Starts up asset compilation; returns UE_TRUE on success, UE_FALSE otherwise and errorCode receives asset compiler process exit code
ueBool utAssetCompilerHelper_Run(utAssetCompilerHelperConfig* config, s32* errorCode);
//! Stops asset compiler (if started by calling utAssetCompilerHelper_Run)
void utAssetCompilerHelper_Stop();

// @}

#endif // UT_ASSET_COMPILER_HELPER_H
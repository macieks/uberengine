#include "Base/ueToolsBase.h"
#include "IO/ioPackageWriter.h"

#include "ueToolEffect.h"

extern bool CompileEffect(ueToolEffect& effect, const ueToolEffect::LoadSettings& loadSettings);

bool CompileEffect(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Set up load parameters

	ueToolEffect::LoadSettings settings;
	if (!ueToolEffect::ParseSettings(settings, params))
		return false;

	// Load effect

	ueToolEffect effect;
	if (!CompileEffect(effect, settings))
		return false;

	// Serialize texture

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!effect.Serialize(&pw))
		return false;

	ioPath outputPath;
	ueStrFormatS(outputPath, "%s/%s/%s.asset", argSet.m_rootExpDir, argSet.m_config, argSet.m_asset);
	if (!io_create_dir_chain(outputPath) || !pw.SaveToFile(outputPath))
	{
		ueLogE("Failed to save package to '%s'", outputPath);
		return false;
	}

	// Save dependency info

	ueBuildDependency dep;
	dep.AddCurrentExecutable();
	dep.Add(settings.m_sourceFileName.c_str());
	dep.Add(outputPath);
	dep.AddAssetParamFiles(params);
	dep.Save(argSet.m_rootDepDir, argSet.m_asset, argSet.m_config);

	return true;
}

int main(int argc, char** args)
{
#if 1
	ueAssetCompilerAppArgSet argSet;
	if (!argSet.Init(argc, args))
		return 1;

	ueToolApp_Startup();

	bool success = true;
	if (argSet.m_help)
		ueToolEffect::DumpSettings();
	else
		success = CompileEffect(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
#else
	ueAssetCompilerAppArgSet argSet;
	if (!argSet.Init(argc, args))
		return 1;

	ueToolApp_Startup();
	return 0;
#endif
}
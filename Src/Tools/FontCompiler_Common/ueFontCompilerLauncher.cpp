#include "ContentPipeline/ueContentPipeline.h"
#include "IO/ioPackageWriter.h"
#include "FontCompiler_Common/ueToolFont.h"

extern bool GenerateFont(ueToolFont& font, ueToolFont::BuildSettings& settings, const ueAssetParams& params);

bool CompileFont(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Set up load parameters

	ueToolFont::BuildSettings settings;
	if (!ueToolFont::ParseBuildSettings(settings, params))
		return false;

	// Load resource

	ueToolFont res;
	res.m_name = argSet.m_asset;
	res.m_sourcePath = settings.m_sourcePath;
	if (!GenerateFont(res, settings, params))
		return false;

	// Serialize resource

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!res.Serialize(&pw))
		return false;

	char outputPath[512];
	ueStrFormatS(outputPath, "%s/%s/%s.asset", argSet.m_rootExpDir, argSet.m_config, argSet.m_asset);
	if (!io_create_dir_chain(outputPath) || !pw.SaveToFile(outputPath))
	{
		ueLogE("Failed to save package to '%s'", outputPath);
		return false;
	}

	// Save dependency info

	ueBuildDependency dep;
	dep.AddCurrentExecutable();
	dep.Add(settings.m_sourcePath.c_str());
	dep.Add(outputPath);
	dep.AddAssetParamFiles(params);
	dep.Save(argSet.m_rootDepDir, argSet.m_asset, argSet.m_config);

	return true;
}

int main(int argc, char** args)
{
	ueAssetCompilerAppArgSet argSet;
	if (!argSet.Init(argc, args))
		return 1;

	ueToolApp_Startup();

	bool success = true;
	if (argSet.m_help)
		ueToolFont::DumpSettings();
	else
		success = CompileFont(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
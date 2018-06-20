#include "ContentPipeline/ueContentPipeline.h"
#include "ShaderCompiler_Common/ueToolShader.h"

extern bool CompileShader(ueToolShader& shader, const ueToolShader::CompileSettings& settings, const ueAssetParams& params);

bool CompileShader(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Parse compile settings

	ueToolShader::CompileSettings settings;
	if (!ueToolShader::ParseCompileSettings(settings, params))
		return false;

	// Compile asset

	ueToolShader res;
	res.m_name = argSet.m_asset;
	res.m_sourcePath = settings.m_sourcePath;
	if (!CompileShader(res, settings, params))
		return false;

	// Save the shader (platform specific)

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!res.Serialize(&pw))
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
	dep.Add(res.m_sourcePath.c_str());
	dep.Add(outputPath);
	dep.AddAssetParamFiles(params);
	if (!dep.Save(argSet.m_rootDepDir, argSet.m_asset, argSet.m_config))
		ueLogW("Failed to save dependancy info.");

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
		ueToolShader::DumpSettings();
	else
		success = CompileShader(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
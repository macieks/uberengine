#include "Base/ueToolsBase.h"
#include "IO/ioPackageWriter.h"

#include "TextureCompiler_Common/ueToolTexture.h"

extern bool CompileTexture(ueToolTexture& t, const ueToolTexture::LoadSettings& loadSettings);

bool CompileTexture(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Set up load parameters

	ueToolTexture::LoadSettings settings;
	if (!ueToolTexture::ParseSettings(settings, params))
		return false;

	// Create output directory

	ueToolTexture tex;
	ueStrFormatS(tex.m_outputPath, "%s/%s/%s.asset", argSet.m_rootExpDir, argSet.m_config, argSet.m_asset);
	if (!io_create_dir_chain(tex.m_outputPath))
	{
		ueLogE("Failed to create output directory for path = '%s'", tex.m_outputPath);
		return false;
	}

	// Compile texture

	if (!CompileTexture(tex, settings))
		return false;

	// Serialize texture

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!tex.Serialize(&pw))
		return false;

	if (!pw.SaveToFile(tex.m_outputPath))
	{
		ueLogE("Failed to save package to '%s'", tex.m_outputPath);
		return false;
	}

	// Save dependency info

	ueBuildDependency dep;
	dep.AddCurrentExecutable();
	for (u32 i = 0; i < settings.m_sourceFileNames.size(); i++)
		dep.Add(settings.m_sourceFileNames[i].c_str());
	if (settings.m_isAtlas && settings.m_atlasDescriptionFilePath.length())
		dep.Add(settings.m_atlasDescriptionFilePath.c_str());
	dep.Add(tex.m_outputPath);
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
		ueToolTexture::DumpSettings();
	else
		success = CompileTexture(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
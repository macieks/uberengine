#include "ContentPipeline/ueContentPipeline.h"
#include "IO/ioPackageWriter.h"
#include "AnimationCompiler_Common/ueToolAnimation.h"

extern bool ImportFromFile(ueToolAnimation& anim, const ueToolAnimation::ImportSettings& settings, const ueAssetParams& assetParams);

bool CompileAnimation(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Set up load parameters

	ueToolAnimation::ImportSettings settings;
	if (!ueToolAnimation::ParseImportSettings(settings, params))
		return false;

	// Import the model

	ueToolAnimation anim;
	anim.m_name = argSet.m_asset;
	anim.m_sourcePath = settings.m_sourcePath;
	if (!ImportFromFile(anim, settings, params))
		return false;

	// Preprocess animation

	anim.RemoveRedundantBones();
	anim.RemoveTrivialKeys();
	anim.ScaleTimeLines(settings.m_timeScale);
	anim.CalculateAnimationLengthFromKeyFrames();
	anim.Resample(settings.m_minFrequency);

	// Serialize out

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!anim.Serialize(&pw))
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
		ueToolAnimation::DumpSettings();
	else
		success = CompileAnimation(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
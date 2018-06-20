#include "ContentPipeline/ueContentPipeline.h"
#include "IO/ioPackageWriter.h"
#include "ModelCompiler_Common/uePhysicsCompiler.h"
#include "ModelCompiler_Common/ueToolModel.h"
#include "MaterialCompiler/ueMaterialMapping.h"

extern bool ImportFromFile(ueToolModel& model, const ueToolModel::ImportSettings& settings, const ueAssetParams& params);

bool CompileModel(const ueAssetCompilerAppArgSet& argSet)
{
	// Load asset parameters

	ueAssetParams params;
	if (!params.Load(argSet.m_asset, argSet.m_config, argSet.m_verbose))
		return false;

	// Parse import settings

	ueToolModel::ImportSettings settings;
	if (!ueToolModel::ParseImportSettings(settings, params))
		return false;

	// Load physics compiler

	if (settings.m_physicsCompilerName.length() > 0)
	{
		settings.m_physicsCompilerFuncs = uePhysicsCompiler_Load(settings.m_physicsCompilerName.c_str());
		if (!settings.m_physicsCompilerFuncs)
		{
			ueLogE("Failed to load physics compiler DLL / SO '%s'. Make sure physics compiler file exists.", settings.m_physicsCompilerName.c_str());
			return false;
		}
	}

	// Import the model

	ioPath sourceDir;
	ioPath_ExtractDirectoryName(settings.m_sourcePath.c_str(), sourceDir, UE_ARRAY_SIZE(sourceDir));

	ueToolModel model;
	model.m_name = argSet.m_asset;
	model.m_sourcePath = settings.m_sourcePath;
	model.m_sourceDir = sourceDir;
	if (!ImportFromFile(model, settings, params))
	{
		if (settings.m_physicsCompilerFuncs)
			uePhysicsCompiler_Unload();
		return false;
	}

	// Apply existing or create new material mapping

	ueMaterialMapping matMapping;
	if (matMapping.Load(argSet.m_rootSrcDir, argSet.m_asset))
	{
		matMapping.Update(&model);
		matMapping.Apply(&model);
	}
	else
	{
		matMapping.Create(&model);
		if (!matMapping.Save(argSet.m_rootSrcDir))
		{
			if (settings.m_physicsCompilerFuncs)
				uePhysicsCompiler_Unload();
			return false;
		}
	}

	// Preprocess

	model.CalculateMissingInvBindPoseTransforms();

	model.ConvertPatchesToTrimeshes();

	if (settings.m_removeDuplicateVertices)
		model.RemoveDuplicateVertices();

	if (settings.m_generateNormals)
		model.GenerateTangentSpace();

	if (settings.m_optimizeSettings.m_makeStatic)
		model.MakeStatic(settings.m_optimizeSettings.m_preserveUnusedBones);

	if (settings.m_transformation != ueMat44::Identity)
		model.Transform(settings.m_transformation);

	model.Optimize(settings.m_optimizeSettings);

	u32 uvStreamsToAdd = 0;
	for (u32 i = 0; i < UE_ARRAY_SIZE(settings.m_generateUVs); i++)
		if (settings.m_generateUVs[i])
			uvStreamsToAdd |= ueToolModel::Vertex::Flags_HasTex0 << i;
	if (uvStreamsToAdd != 0)
		model.AddUVStreams(uvStreamsToAdd);

	model.CalculateBox();

	// Serialize

	ioPackageWriter pw;
	pw.ResetForPlatform(params.GetPlatform());

	if (!model.Serialize(&pw, settings))
	{
		if (settings.m_physicsCompilerFuncs)
			uePhysicsCompiler_Unload();
		return false;
	}

	if (settings.m_physicsCompilerFuncs)
		uePhysicsCompiler_Unload();

	// Save dependency info

	char outputPath[512];
	ueStrFormatS(outputPath, "%s/%s/%s.asset", argSet.m_rootExpDir, argSet.m_config, argSet.m_asset);
	if (!io_create_dir_chain(outputPath) || !pw.SaveToFile(outputPath))
	{
		ueLogE("Failed to save package to '%s'", outputPath);
		return false;
	}

	ueBuildDependency dep;
	dep.AddCurrentExecutable();
	if (settings.m_physicsCompilerName.length() > 0)
		dep.AddExecutable(settings.m_physicsCompilerName.c_str());
	dep.Add(settings.m_sourcePath.c_str());
	dep.Add(outputPath);
	dep.AddAssetParamFiles(params);
	matMapping.AddDependecies(argSet.m_rootSrcDir, &dep);
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
		ueToolModel::DumpSettings();
	else
		success = CompileModel(argSet);

	ueToolApp_Shutdown();

	return success ? 0 : 3;
}
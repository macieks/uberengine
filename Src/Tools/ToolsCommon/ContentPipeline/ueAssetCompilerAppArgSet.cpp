#include "ContentPipeline/ueContentPipeline.h"

void ueAssetCompilerAppArgSet::DumpUsage()
{
	fprintf(stderr, "Invalid arguments, usage:\n"
		"-asset <asset-name> -config <config-name> -rootsrc <root-src-dir> -rootexp <root-exp-dir> -rootdep <root-dep-dir> [-verbose]\n"
		"-help");
}

bool ueAssetCompilerAppArgSet::Init(u32 count, char** args)
{
	UE_ASSERT(count > 0);

	if (!m_argSet.Init(count - 1, args + 1))
	{
		DumpUsage();
		return false;
	}
	
	m_help = m_argSet.HasArg("help");
	if (m_help)
		return true;

	if (!m_argSet.GetArg("asset", m_asset) ||
		!m_argSet.GetArg("config", m_config) ||
		!m_argSet.GetArg("root-src", m_rootSrcDir) ||
		!m_argSet.GetArg("root-exp", m_rootExpDir) ||
		!m_argSet.GetArg("root-dep", m_rootDepDir))
	{
		DumpUsage();
		return false;
	}

	m_verbose = m_argSet.HasArg("verbose");
	return true;
}
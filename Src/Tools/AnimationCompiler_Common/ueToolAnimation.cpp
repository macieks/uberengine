#include "AnimationCompiler_Common/ueToolAnimation.h"

u32 ueToolAnimation::VERSION = 1;

ueToolAnimation::ueToolAnimation()
{
	Reset();
}

void ueToolAnimation::Reset()
{
	m_version = VERSION;
	m_length = 0;
	m_nodes.clear();
}

// Saving

bool ueToolAnimation::Save(const char* path)
{
/*
	OutFileStream fs;
	if (!fs.Open(path)) return false;
	m_version = CurrentVersion;
	fs << m_version << m_length << m_nodes;
	return fs.IsError();
*/
	return false;
}

// Loading

bool ueToolAnimation::Load(const char* path)
{
/*
	InFileStream fs;
	if (!fs.Open(path)) return false;
	fs << m_version;
	if (m_version != CurrentVersion) return false;
	fs << m_length << m_nodes;
	return fs.IsError();
*/
	return false;
}

void ueToolAnimation::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", 0, "Source animation file name relative to root source directory ('root_src'), e.g. 'my_anims/walk.3ds'");
	DumpSettingString("sourceAnimationName", Setting_Optional, "Source animation name within source animation file; necessary if source animation file contains more than 1 animation");
	DumpSettingU32("startFrame", Setting_Optional, "Starting animation frame index");
	DumpSettingU32("numFrames", Setting_Optional, "Number of animation frames");
	DumpSettingF32("minFrequency", Setting_Optional, "Minimal animation sampling frequency");
	DumpSettingF32("timeScale", Setting_Optional, "Time scale");
}

bool ueToolAnimation::ParseImportSettings(ImportSettings& settings, ueAssetParams& params)
{
	settings.m_sourcePath = params.GetParam("source");
	params.GetParam("sourceAnimationName", settings.m_sourceAnimationName);
	params.GetU32Param("startFrame", settings.m_startFrame);
	params.GetU32Param("numFrames", settings.m_numFrames);
	params.GetF32Param("minFrequency", settings.m_minFrequency);
	params.GetF32Param("timeScale", settings.m_timeScale);

	return true;
}

ueToolAnimation::Node* ueToolAnimation::GetAddNode(const char* name)
{
	for (u32 i = 0; i < m_nodes.size(); i++)
		if (m_nodes[i].m_name == name)
			return &m_nodes[i];

	Node* node = &vector_push(m_nodes);
	node->m_name = name;
	return node;
}
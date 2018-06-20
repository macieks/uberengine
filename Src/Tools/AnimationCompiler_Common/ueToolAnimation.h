#pragma once

#include "ContentPipeline/ueContentPipeline.h"
#include "Math/ueShapes.h"

class ioPackageWriter;

struct ueToolAnimation
{
	static u32 VERSION;

	struct ImportSettings
	{
		std::string m_sourcePath;
		std::string m_sourceAnimationName;

		u32 m_startFrame;
		u32 m_numFrames;
		f32 m_minFrequency;
		f32 m_timeScale;

		ImportSettings() :
			m_startFrame(0),
			m_numFrames(~0),
			m_minFrequency(0.0f),
			m_timeScale(1.0f)
		{}
	};

	struct Key
	{
		f32 m_time;

		Key() {}
		Key(f32 time) :
			m_time(time)
		{}
	};

	struct Vec3Key : public Key
	{
		ueVec3 m_value;

		Vec3Key() {}
		Vec3Key(f32 time, ueVec3 value) :
			Key(time),
			m_value(value)
		{}
	};

	struct QuatKey : public Key
	{
		ueQuat m_value;

		QuatKey() {}
		QuatKey(f32 time, ueQuat value) :
			Key(time),
			m_value(value)
		{}
	};

	struct Node
	{
		std::string m_name;
		
		std::vector<Vec3Key> m_translationKeys;
		std::vector<Vec3Key> m_scaleKeys;
		std::vector<QuatKey> m_rotationKeys;
	};

	u32 m_version;
	std::string m_name;
	std::string m_sourcePath;

	std::string m_sourceAnimationName;
	f32 m_length;
	std::vector<Node> m_nodes;

	ueToolAnimation();
	void Reset();
	bool Save(const char* path);
	bool Load(const char* path);

	bool Serialize(ioPackageWriter* pw);

	static void DumpSettings();
	static bool ParseImportSettings(ImportSettings& settings, ueAssetParams& params);

	// Helpers

	void RemoveRedundantBones();
	void RemoveTrivialKeys();
	void ScaleTimeLines(f32 scale);
	void CalculateAnimationLengthFromKeyFrames();
	void Resample(f32 minSampleFrequency);

	Node* GetAddNode(const char* name);
};
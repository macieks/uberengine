#pragma once

#include "ContentPipeline/ueContentPipeline.h"

class ioPackageWriter;
class ioSegmentWriter;
struct gxEffectTypeData;

class ueToolEffect
{
public:
	struct LoadSettings
	{
		std::string m_sourceFileName;
	};

	gxEffectTypeData* m_data;
	std::vector< std::vector<std::string> > m_textureNames; // An array of gxEmitterType::Texture_MAX per emitter

	ueToolEffect() : m_data(NULL) {}
	void LoadFromXml();
	bool Serialize(ioPackageWriter* pw);

	// Utils

	static void DumpSettings();
	static bool ParseSettings(LoadSettings& s, const ueAssetParams& params);
};
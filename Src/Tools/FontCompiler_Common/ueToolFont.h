#pragma once

#include "ContentPipeline/ueContentPipeline.h"
#include "TextureCompiler_Common/ueToolTexture.h"
#include "Misc/ueTextureRectPacker.h"
#include "GX/gxFont_Shared.h"

class ioPackageWriter;

struct ueToolFont
{
public:
	struct BuildSettings
	{
		std::string m_sourcePath;

		u32 m_sizeInPixels;
		u32 m_options; // See gxFontFlags
		u32 m_numTextureLevels;
		std::vector<u32> m_codes;
		glBufferFormat m_textureFormat;

		bool m_packIntoSeparateChannels;
		bool m_makePow2Texture;

		BuildSettings() :
			m_sizeInPixels(32),
			m_options(0),
			m_numTextureLevels(0),
			m_packIntoSeparateChannels(true),
			m_textureFormat(glBufferFormat_B8G8R8A8),
			m_makePow2Texture(false)
		{}
	};

	std::string m_name;
	std::string m_sourcePath;

	u32 m_sizeInPixels;
	u32 m_options; // See gxFontFlags
	std::map<u32, gxFontGlyph> m_glyphs;
	ueToolTexture m_texture;

	glBufferFormat m_textureFormat;
	bool m_packIntoSeparateChannels;

	ueToolFont();
	~ueToolFont();
	void Destroy();

	bool Serialize(ioPackageWriter* pw);

	void CopyFrom(const ueTextureRectPacker::Rect& rect, const u8* buffer, u32 pitch);

	static void DumpSettings();
	static bool ParseBuildSettings(BuildSettings& settings, ueAssetParams& params);
};